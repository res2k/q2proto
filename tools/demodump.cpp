/*
Copyright (C) 2026 Frank Richter

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#include "q2protoio.hpp"

#include "expected.hpp"
#include <fmt/format.h>
#include <fmt/ranges.h>
#include <fmt/std.h>
#include <map>
#include <memory>
#include <ranges>
#include "scope.hpp"
#include <span>

#include "q2proto/q2proto_client.h"
#include "q2proto/q2proto_error.h"

#include "../src/q2proto_internal_protocol.h" // to access values currently not exposed in headers

/// This is essentially format_to_n(), but null-terminates the output.
template<typename... T>
static inline fmt::format_to_n_result<char*> format_to_s(std::span<char> out, fmt::format_string<T...> fmt, T&&... args)
{
    auto fmt_result = fmt::format_to_n(out.data(), out.size(), fmt, std::forward<T>(args)...);
    *fmt_result.out = 0;
    return fmt_result;
}

template<>
struct fmt::formatter<q2proto_svc_message_type_t> : formatter<const char*>
{
    auto format(q2proto_svc_message_type_t type, format_context& ctx) const -> format_context::iterator
    {
        char buf[64];
        const char* str = nullptr;
        switch(type)
        {
        #define S(X) case Q2P_SVC_ ## X: str = #X; break;
            S(INVALID)
            S(MUZZLEFLASH)
            S(MUZZLEFLASH2)
            S(TEMP_ENTITY)
            S(NOP)
            S(DISCONNECT)
            S(RECONNECT)
            S(SOUND)
            S(PRINT)
            S(STUFFTEXT)
            S(SERVERDATA)
            S(CONFIGSTRING)
            S(SPAWNBASELINE)
            S(CENTERPRINT)
            S(DOWNLOAD)
            S(FRAME)
            S(INVENTORY)
            S(LAYOUT)
            S(FRAME_ENTITY_DELTA)
            S(SETTING)
            S(DAMAGE)
            S(FOG)
            S(POI)
            S(HELP_PATH)
            S(ACHIEVEMENT)
            S(LOCPRINT)
        #undef S
        }
        if (!str) {
            format_to_s(buf, "<{}>", static_cast<int>(type));
            str = buf;
        }
        return formatter<const char*>::format(str, ctx);
    }
};

#define OUTPUT_FIELD(Context, Struct, Field, ...)                       \
    (Context).field(#Field, (Struct).Field __VA_OPT__(, ) __VA_ARGS__)

template <typename T> static std::string print_hex(const T &val) { return fmt::format("{:#x}", val); }

template <typename T> class PrintTraits
{
public:
    static std::string format(const T &val) { return fmt::to_string(val); }
};

template<>
class PrintTraits<q2proto_string_t>
{
public:
    static std::string format(const q2proto_string_t &str)
    {
        return fmt::format("{:?}", std::string_view(str.str, str.len));
    }
};

template<>
class PrintTraits<q2proto_var_coords_t>
{
public:
    static void format_comp(std::string& str, const q2proto_var_coords_t &coords, int c, bool is_diff = false)
    {
        char comp_type = '?';
        if (q2proto_var_coords_is_comp_float(&coords, c))
            comp_type = 'f';
        else if (q2proto_var_coords_is_comp_int(&coords, c))
            comp_type = 'i';
        fmt::format_to(std::back_inserter(str), "{}{:c}: {}{}{}", str.empty() ? "" : " ", 'x' + c, is_diff ? "Δ" : "",
                       q2proto_var_coords_get_float_comp(&coords, c), comp_type);
    }
    static std::string format(const q2proto_var_coords_t &coords)
    {
        std::string str;
        for (int c = 0; c < 3; c++)
            format_comp(str, coords, c);
        return str;
    }
};

template<>
class PrintTraits<q2proto_var_coord_t>
{
public:
    static std::string format(const q2proto_var_coord_t &coord)
    {
        std::string str;
        char comp_type = '?';
        if (q2proto_var_coord_is_float(&coord))
            comp_type = 'f';
        else if (q2proto_var_coord_is_int(&coord))
            comp_type = 'i';
        fmt::format_to(std::back_inserter(str), "{}{}{}", str.empty() ? "" : " ", q2proto_var_coord_get_float(&coord),
                       comp_type);
        return str;
    }
};

template<>
class PrintTraits<q2proto_var_angles_t>
{
public:
    static void format_comp(std::string& str, const q2proto_var_angles_t &angles, int c)
    {
        char comp_type = '?';
        if (q2proto_var_angles_is_comp_float(&angles, c))
            comp_type = 'f';
        else if (q2proto_var_angles_is_comp_short(&angles, c))
            comp_type = 's';
        else if (q2proto_var_angles_is_comp_char(&angles, c))
            comp_type = 'c';
        fmt::format_to(std::back_inserter(str), "{}{:c}: {}{}", str.empty() ? "" : " ", 'x' + c,
                        q2proto_var_angles_get_float_comp(&angles, c), comp_type);
    }
    static std::string format(const q2proto_var_angles_t &angles)
    {
        std::string str;
        for (int c = 0; c < 3; c++)
            format_comp(str, angles, c);
        return str;
    }
};

template<>
class PrintTraits<q2proto_var_small_offsets_t>
{
public:
    static void format_comp(std::string& str, const q2proto_var_small_offsets_t &coords, int c)
    {
        char comp_type = '?';
        if (q2proto_var_small_offsets_is_comp_float(&coords, c))
            comp_type = 'f';
        else if (q2proto_var_small_offsets_is_comp_char(&coords, c))
            comp_type = 'c';
        else if (q2proto_var_small_offsets_is_comp_q2repro_viewoffset(&coords, c))
            comp_type = 'v';
        else if (q2proto_var_small_offsets_is_comp_q2repro_gunoffset(&coords, c))
            comp_type = 'g';
        fmt::format_to(std::back_inserter(str), "{}{:c}: {}{}", str.empty() ? "" : " ", 'x' + c,
                        q2proto_var_small_offsets_get_float_comp(&coords, c), comp_type);
    }
    static std::string format(const q2proto_var_small_offsets_t &coords)
    {
        std::string str;
        for (int c = 0; c < 3; c++)
            format_comp(str, coords, c);
        return str;
    }
};

template<>
class PrintTraits<q2proto_var_small_angles_t>
{
public:
    static void format_comp(std::string& str, const q2proto_var_small_angles_t &angles, int c)
    {
        char comp_type = '?';
        if (q2proto_var_small_angles_is_comp_float(&angles, c))
            comp_type = 'f';
        else if (q2proto_var_small_angles_is_comp_char(&angles, c))
            comp_type = 'c';
        else if (q2proto_var_small_angles_is_comp_q2repro_kick_angles(&angles, c))
            comp_type = 'k';
        else if (q2proto_var_small_angles_is_comp_q2repro_gunangles(&angles, c))
            comp_type = 'g';
        fmt::format_to(std::back_inserter(str), "{}{:c}: {}{}", str.empty() ? "" : " ", 'x' + c,
                        q2proto_var_small_angles_get_float_comp(&angles, c), comp_type);
    }
    static std::string format(const q2proto_var_small_angles_t &angles)
    {
        std::string str;
        for (int c = 0; c < 3; c++)
            format_comp(str, angles, c);
        return str;
    }
};

template<>
class PrintTraits<q2proto_var_fraction_t>
{
public:
    static std::string format(const q2proto_var_fraction_t &frac)
    {
        std::string str;
        char comp_type = '?';
        if (q2proto_var_fraction_is_float(&frac))
            comp_type = 'f';
        else if (q2proto_var_fraction_is_word(&frac))
            comp_type = 'w';
        else if (q2proto_var_fraction_is_byte(&frac))
            comp_type = 'b';
        fmt::format_to(std::back_inserter(str), "{}{}{}", str.empty() ? "" : " ", q2proto_var_fraction_get_float(&frac),
                       comp_type);
        return str;
    }
};

template<>
class PrintTraits<q2proto_angles_delta_t>
{
public:
    static std::string format(const q2proto_angles_delta_t &angles)
    {
        std::string str;
        for (int c = 0; c < 3; c++) {
            if ((angles.delta_bits & (1 << c)) == 0) continue;
            PrintTraits<q2proto_var_angles_t>::format_comp(str, angles.values, c);
        }

        return str;
    }
};

template<>
class PrintTraits<q2proto_small_offsets_delta_t>
{
public:
    static std::string format(const q2proto_small_offsets_delta_t &coords)
    {
        std::string str;
        for (int c = 0; c < 3; c++) {
            if ((coords.delta_bits & (1 << c)) == 0) continue;
            PrintTraits<q2proto_var_small_offsets_t>::format_comp(str, coords.values, c);
        }

        return str;
    }
};

template<>
class PrintTraits<q2proto_small_angles_delta_t>
{
public:
    static std::string format(const q2proto_small_angles_delta_t &angles)
    {
        std::string str;
        for (int c = 0; c < 3; c++) {
            if ((angles.delta_bits & (1 << c)) == 0) continue;
            PrintTraits<q2proto_var_small_angles_t>::format_comp(str, angles.values, c);
        }

        return str;
    }
};

template<>
class PrintTraits<q2proto_color_delta_t>
{
public:
    static std::string format(const q2proto_color_delta_t &color)
    {
        static constexpr char comp_names[4] = {'r', 'g', 'b', 'a'};
        std::string str;
        for (int c = 0; c < 4; c++) {
            if ((color.delta_bits & (1 << c)) == 0) continue;
            char comp_type = '?';
            if (q2proto_var_color_is_comp_float(&color.values, c))
                comp_type = 'f';
            else if (q2proto_var_color_is_comp_byte(&color.values, c))
                comp_type = 'b';
            fmt::format_to(std::back_inserter(str), "{}{:c}: {}{}", str.empty() ? "" : " ", comp_names[c],
                           q2proto_var_color_get_float_comp(&color.values, c), comp_type);
        }

        return str;
    }
};

template<>
class PrintTraits<q2proto_maybe_diff_coords_t>
{
public:
    static std::string format(const q2proto_maybe_diff_coords_t &coord)
    {
        std::string str;
        for (int c = 0; c < 3; c++) {
            if ((coord.read.value.delta_bits & (1 << c)) == 0) continue;
            PrintTraits<q2proto_var_coords_t>::format_comp(str, coord.read.value.values, c,
                                                           (coord.read.diff_bits & (1 << c)) != 0);
        }

        return str;
    }
};

template <typename T>
class DeltaPrintHelper
{
    template<typename U, typename Enable = void> struct storage_type_impl
    {
        using type = U;
    };
    template<typename U> struct storage_type_impl<U, std::enable_if_t<std::is_enum_v<U>>>
    {
        using type = std::underlying_type_t<U>;
    };
    using storage_type = typename storage_type_impl<T>::type;

    storage_type remaining;
    std::string str;

public:
    DeltaPrintHelper(storage_type value) : remaining(value) {}

    void check_flag(const char *name, T flag_value)
    {
        auto underlying_flag = static_cast<storage_type>(flag_value);
        if ((remaining & underlying_flag) != 0) {
            if (!str.empty())
                str.append(" | ");
            str.append(name);
            remaining &= ~underlying_flag;
        }
    }
    template<typename FormatFunc>
    void check_alternatives(storage_type mask, const FormatFunc& formatter)
    {
        auto formatted = formatter(static_cast<T>(remaining & mask));
        bool is_known = false;
        if constexpr (std::is_same_v<decltype(formatted), const char*>)
            is_known = formatted && *formatted;
        else
            is_known = !formatted.empty();
        if (is_known) {
            if (!str.empty())
                str.append(" | ");
            str.append(formatted);
            remaining &= ~mask;
        }
    }

    std::string finish()
    {
        if (str.empty())
            str = fmt::format("{:#x}", remaining);
        else if (remaining != 0)
            str.append(fmt::format("| {:#x}", remaining));
        return str;
    }
};

#define CHECK_FLAG(Helper, Flag) (Helper).check_flag(#Flag, Flag)

static std::string print_entity_delta_bits(const uint32_t &delta_bits)
{
    auto helper = DeltaPrintHelper<q2proto_entity_state_delta_flags>(delta_bits);
    CHECK_FLAG(helper, Q2P_ESD_MODELINDEX);
    CHECK_FLAG(helper, Q2P_ESD_MODELINDEX2);
    CHECK_FLAG(helper, Q2P_ESD_MODELINDEX3);
    CHECK_FLAG(helper, Q2P_ESD_MODELINDEX4);
    CHECK_FLAG(helper, Q2P_ESD_FRAME);
    CHECK_FLAG(helper, Q2P_ESD_SKINNUM);
    CHECK_FLAG(helper, Q2P_ESD_EFFECTS);
    CHECK_FLAG(helper, Q2P_ESD_EFFECTS_MORE);
    CHECK_FLAG(helper, Q2P_ESD_RENDERFX);
    CHECK_FLAG(helper, Q2P_ESD_OLD_ORIGIN);
    CHECK_FLAG(helper, Q2P_ESD_SOUND);
    CHECK_FLAG(helper, Q2P_ESD_LOOP_VOLUME);
    CHECK_FLAG(helper, Q2P_ESD_LOOP_ATTENUATION);
    CHECK_FLAG(helper, Q2P_ESD_EVENT);
    CHECK_FLAG(helper, Q2P_ESD_SOLID);
    CHECK_FLAG(helper, Q2P_ESD_ALPHA);
    CHECK_FLAG(helper, Q2P_ESD_SCALE);
    return helper.finish();
}

template<>
class PrintTraits<q2proto_entity_state_delta_t>
{
public:
    template<typename Ctx>
    static void members(Ctx& context, const q2proto_entity_state_delta_t& x)
    {
        OUTPUT_FIELD(context, x, delta_bits, print_entity_delta_bits);

    #define ENTITY_FIELD(Flag, Field, ...)                                  \
        do {                                                                \
            if ((x.delta_bits & Q2P_ESD_##Flag) != 0)                       \
                OUTPUT_FIELD(context, x, Field __VA_OPT__(, ) __VA_ARGS__); \
        } while (false)

        ENTITY_FIELD(MODELINDEX, modelindex);
        ENTITY_FIELD(MODELINDEX2, modelindex2);
        ENTITY_FIELD(MODELINDEX3, modelindex3);
        ENTITY_FIELD(MODELINDEX4, modelindex4);
        ENTITY_FIELD(FRAME, frame);
        ENTITY_FIELD(SKINNUM, skinnum);
        ENTITY_FIELD(EFFECTS, effects, print_hex);
        ENTITY_FIELD(EFFECTS_MORE, effects_more, print_hex);
        ENTITY_FIELD(RENDERFX, renderfx, print_hex);
        if (x.origin.read.value.delta_bits != 0)
            OUTPUT_FIELD(context, x, origin);
        if (x.angle.delta_bits != 0)
            OUTPUT_FIELD(context, x, angle);
        ENTITY_FIELD(OLD_ORIGIN, old_origin);
        ENTITY_FIELD(SOUND, sound);
        ENTITY_FIELD(LOOP_VOLUME, loop_volume);
        ENTITY_FIELD(LOOP_ATTENUATION, loop_attenuation);
        ENTITY_FIELD(EVENT, event);
        ENTITY_FIELD(SOLID, solid, print_hex);
        ENTITY_FIELD(ALPHA, alpha);
        ENTITY_FIELD(SCALE, scale);

    #undef ENTITY_FIELD
    }
};

template<>
class PrintTraits<q2proto_svc_muzzleflash_t>
{
public:
    template<typename Ctx>
    static void members(Ctx& context, const q2proto_svc_muzzleflash_t& x)
    {
        OUTPUT_FIELD(context, x, entity);
        OUTPUT_FIELD(context, x, weapon);
        OUTPUT_FIELD(context, x, silenced);
    };
};

static const char *temp_ent_type_str(uint8_t te_type)
{
    switch (te_type)
    {
    #define TE(x) case TE_##x: return #x;
        TE(GUNSHOT)
        TE(BLOOD)
        TE(BLASTER)
        TE(RAILTRAIL)
        TE(SHOTGUN)
        TE(EXPLOSION1)
        TE(EXPLOSION2)
        TE(ROCKET_EXPLOSION)
        TE(GRENADE_EXPLOSION)
        TE(SPARKS)
        TE(SPLASH)
        TE(BUBBLETRAIL)
        TE(SCREEN_SPARKS)
        TE(SHIELD_SPARKS)
        TE(BULLET_SPARKS)
        TE(LASER_SPARKS)
        TE(PARASITE_ATTACK)
        TE(ROCKET_EXPLOSION_WATER)
        TE(GRENADE_EXPLOSION_WATER)
        TE(MEDIC_CABLE_ATTACK)
        TE(BFG_EXPLOSION)
        TE(BFG_BIGEXPLOSION)
        TE(BOSSTPORT)
        TE(BFG_LASER)
        TE(GRAPPLE_CABLE)
        TE(WELDING_SPARKS)
        TE(GREENBLOOD)
        TE(BLUEHYPERBLASTER)
        TE(PLASMA_EXPLOSION)
        TE(TUNNEL_SPARKS)
        TE(BLASTER2)
        TE(RAILTRAIL2)
        TE(FLAME)
        TE(LIGHTNING)
        TE(DEBUGTRAIL)
        TE(PLAIN_EXPLOSION)
        TE(FLASHLIGHT)
        TE(FORCEWALL)
        TE(HEATBEAM)
        TE(MONSTER_HEATBEAM)
        TE(STEAM)
        TE(BUBBLETRAIL2)
        TE(MOREBLOOD)
        TE(HEATBEAM_SPARKS)
        TE(HEATBEAM_STEAM)
        TE(CHAINFIST_SMOKE)
        TE(ELECTRIC_SPARKS)
        TE(TRACKER_EXPLOSION)
        TE(TELEPORT_EFFECT)
        TE(DBALL_GOAL)
        TE(WIDOWBEAMOUT)
        TE(NUKEBLAST)
        TE(WIDOWSPLASH)
        TE(EXPLOSION1_BIG)
        TE(EXPLOSION1_NP)
        TE(FLECHETTE)
        TE(BLUEHYPERBLASTER_2)
        TE(BFG_ZAP)
        TE(BERSERK_SLAM)
        TE(GRAPPLE_CABLE_2)
        TE(POWER_SPLASH)
        TE(LIGHTNING_BEAM)
        TE(EXPLOSION1_NL)
        TE(EXPLOSION2_NL)
        TE(Q2PRO_DAMAGE_DEALT)
    #undef TE
    }

    return "???";
}

template<>
class PrintTraits<q2proto_svc_temp_entity_t>
{
public:
    template<typename Ctx>
    static void members(Ctx& context, const q2proto_svc_temp_entity_t& x)
    {
        context.field("type", fmt::format("{} ({})", temp_ent_type_str(x.type), x.type));
        OUTPUT_FIELD(context, x, position1);
        OUTPUT_FIELD(context, x, position2);
        OUTPUT_FIELD(context, x, offset);
        OUTPUT_FIELD(context, x, direction);
        OUTPUT_FIELD(context, x, count);
        OUTPUT_FIELD(context, x, color);
        OUTPUT_FIELD(context, x, entity1);
        OUTPUT_FIELD(context, x, entity2);
        OUTPUT_FIELD(context, x, time);
    };
};

template<>
class PrintTraits<q2proto_svc_layout_t>
{
public:
    template<typename Ctx>
    static void members(Ctx& context, const q2proto_svc_layout_t& x)
    {
        OUTPUT_FIELD(context, x, layout_str);
    };
};

template<>
class PrintTraits<q2proto_svc_inventory_t>
{
public:
    template<typename Ctx>
    static void members(Ctx& context, const q2proto_svc_inventory_t& x)
    {
        OUTPUT_FIELD(context, x, inventory);
    };
};

static std::string print_sound_flags(const uint8_t &flags)
{
    auto helper = DeltaPrintHelper<uint8_t>(flags);
    CHECK_FLAG(helper, SND_VOLUME);
    CHECK_FLAG(helper, SND_ATTENUATION);
    CHECK_FLAG(helper, SND_POS);
    CHECK_FLAG(helper, SND_ENT);
    CHECK_FLAG(helper, SND_OFFSET);
    CHECK_FLAG(helper, SND_Q2PRO_INDEX16);
    CHECK_FLAG(helper, SND_KEX_LARGE_ENT);
    return helper.finish();
}

template<>
class PrintTraits<q2proto_svc_sound_t>
{
public:
    template<typename Ctx>
    static void members(Ctx& context, const q2proto_svc_sound_t& x)
    {
        // FIXME: employ q2proto_sound_decode_message() instead?
        OUTPUT_FIELD(context, x, flags, print_sound_flags);
        OUTPUT_FIELD(context, x, index);
        OUTPUT_FIELD(context, x, volume);
        OUTPUT_FIELD(context, x, attenuation);
        OUTPUT_FIELD(context, x, timeofs);
        OUTPUT_FIELD(context, x, entity);
        OUTPUT_FIELD(context, x, channel);
        OUTPUT_FIELD(context, x, pos);
    };
};

static std::string print_print_level(const uint8_t &level)
{
    // game print flags
    enum print_type_t
    {
        PRINT_LOW = 0,	  // pickup messages
        PRINT_MEDIUM = 1, // death messages
        PRINT_HIGH = 2,	  // critical messages
        PRINT_CHAT = 3,	  // chat messages
        PRINT_TYPEWRITER = 4, // centerprint but typed out one char at a time
        PRINT_CENTER = 5, // centerprint without a separate function (loc variants only)
        PRINT_TTS = 6, // PRINT_HIGH but will speak for players with narration on

        PRINT_BROADCAST = (1 << 3), // Bitflag, add to message to broadcast print to all clients.
        PRINT_NO_NOTIFY = (1 << 4) // Bitflag, don't put on notify
    };
    constexpr uint8_t print_flag_bits = PRINT_BROADCAST | PRINT_NO_NOTIFY;
    auto print_type_str = [](print_type_t type) -> const char * {
        switch (type)
        {
        #define T(Type) case Type: return #Type;
            T(PRINT_LOW)
            T(PRINT_MEDIUM)
            T(PRINT_HIGH)
            T(PRINT_CHAT)
            T(PRINT_TYPEWRITER)
            T(PRINT_CENTER)
            T(PRINT_TTS)
        #undef T
        }
        return nullptr;
    };

    auto helper = DeltaPrintHelper<print_type_t>(level);
    helper.check_alternatives(~print_flag_bits, print_type_str);
    CHECK_FLAG(helper, PRINT_BROADCAST);
    CHECK_FLAG(helper, PRINT_NO_NOTIFY);
    return helper.finish();
}

template<>
class PrintTraits<q2proto_svc_print_t>
{
public:
    template<typename Ctx>
    static void members(Ctx& context, const q2proto_svc_print_t& x)
    {
        OUTPUT_FIELD(context, x, level, print_print_level);
        OUTPUT_FIELD(context, x, string);
    };
};

template<>
class PrintTraits<q2proto_svc_stufftext_t>
{
public:
    template<typename Ctx>
    static void members(Ctx& context, const q2proto_svc_stufftext_t& x)
    {
        OUTPUT_FIELD(context, x, string);
    };
};

template<>
class PrintTraits<decltype(q2proto_svc_serverdata_t::r1q2)>
{
public:
    template<typename Ctx, typename U>
    static void members(Ctx& context, const U& x)
    {
        OUTPUT_FIELD(context, x, enhanced);
    };
};

template<>
class PrintTraits<decltype(q2proto_svc_serverdata_t::q2pro)>
{
public:
    template<typename Ctx, typename U>
    static void members(Ctx& context, const U& x)
    {
        OUTPUT_FIELD(context, x, server_state);
        OUTPUT_FIELD(context, x, qw_mode);
        OUTPUT_FIELD(context, x, waterjump_hack);
        OUTPUT_FIELD(context, x, extensions);
        OUTPUT_FIELD(context, x, extensions_v2);
    };
};

template<>
class PrintTraits<decltype(q2proto_svc_serverdata_t::q2repro)>
{
public:
    template<typename Ctx, typename U>
    static void members(Ctx& context, const U& x)
    {
        OUTPUT_FIELD(context, x, game3_compat);
    };
};

template<>
class PrintTraits<q2proto_svc_serverdata_t>
{
public:
    template<typename Ctx>
    static void members(Ctx& context, const q2proto_svc_serverdata_t& x)
    {
        OUTPUT_FIELD(context, x, protocol);
        OUTPUT_FIELD(context, x, servercount);
        OUTPUT_FIELD(context, x, attractloop);
        OUTPUT_FIELD(context, x, gamedir);
        OUTPUT_FIELD(context, x, clientnum);
        OUTPUT_FIELD(context, x, levelname);
        OUTPUT_FIELD(context, x, protocol_version);
        OUTPUT_FIELD(context, x, strafejump_hack);
        OUTPUT_FIELD(context, x, server_fps);
        OUTPUT_FIELD(context, x, r1q2);
        OUTPUT_FIELD(context, x, q2pro);
        OUTPUT_FIELD(context, x, q2repro);
    }
};

template<>
class PrintTraits<q2proto_svc_configstring_t>
{
public:
    template<typename Ctx>
    static void members(Ctx& context, const q2proto_svc_configstring_t& x)
    {
        OUTPUT_FIELD(context, x, index);
        OUTPUT_FIELD(context, x, value);
    }
};

template<>
class PrintTraits<q2proto_svc_spawnbaseline_t>
{
public:
    template<typename Ctx>
    static void members(Ctx& context, const q2proto_svc_spawnbaseline_t& x)
    {
        OUTPUT_FIELD(context, x, entnum);
        OUTPUT_FIELD(context, x, delta_state);
    }
};

template<>
class PrintTraits<q2proto_svc_centerprint_t>
{
public:
    template<typename Ctx>
    static void members(Ctx& context, const q2proto_svc_centerprint_t& x)
    {
        OUTPUT_FIELD(context, x, message);
    }
};

template<>
class PrintTraits<q2proto_svc_download_t>
{
public:
    template<typename Ctx>
    static void members(Ctx& context, const q2proto_svc_download_t& x)
    {
        OUTPUT_FIELD(context, x, size);
        OUTPUT_FIELD(context, x, percent);
        context.field("data", "<omitted>");
        OUTPUT_FIELD(context, x, compressed);
        OUTPUT_FIELD(context, x, uncompressed_size);
    }
};

static std::string print_fog_flags(const uint32_t &flags)
{
    auto helper = DeltaPrintHelper<uint32_t>(flags);
    CHECK_FLAG(helper, Q2P_FOG_DENSITY_SKYFACTOR);
    CHECK_FLAG(helper, Q2P_FOG_TIME);
    CHECK_FLAG(helper, Q2P_HEIGHTFOG_FALLOFF);
    CHECK_FLAG(helper, Q2P_HEIGHTFOG_DENSITY);
    CHECK_FLAG(helper, Q2P_HEIGHTFOG_START_DIST);
    CHECK_FLAG(helper, Q2P_HEIGHTFOG_END_DIST);
    return helper.finish();
}

template<>
class PrintTraits<q2proto_svc_fog_t>
{
public:
    template<typename Ctx>
    static void members(Ctx& context, const q2proto_svc_fog_t& x)
    {
        OUTPUT_FIELD(context, x, flags, print_fog_flags);

    #define FOG_FIELD(Flag, Field, ...)                                     \
        do {                                                                \
            if ((x.flags & Q2P_##Flag) != 0)                                \
                OUTPUT_FIELD(context, x, Field __VA_OPT__(, ) __VA_ARGS__); \
        } while (false)

        FOG_FIELD(FOG_DENSITY_SKYFACTOR, global.density);
        FOG_FIELD(FOG_DENSITY_SKYFACTOR, global.skyfactor);
        if (x.global.color.delta_bits != 0)
            OUTPUT_FIELD(context, x, global.color);
        FOG_FIELD(FOG_TIME, global.time);
        FOG_FIELD(HEIGHTFOG_FALLOFF, height.falloff);
        FOG_FIELD(HEIGHTFOG_DENSITY, height.density);
        if (x.height.start_color.delta_bits != 0)
            OUTPUT_FIELD(context, x, height.start_color);
        FOG_FIELD(HEIGHTFOG_START_DIST, height.start_dist);
        if (x.height.end_color.delta_bits != 0)
            OUTPUT_FIELD(context, x, height.end_color);
        FOG_FIELD(HEIGHTFOG_END_DIST, height.end_dist);

    #undef FOG_FIELD
    }
};

static std::string print_player_delta_bits(const uint32_t &delta_bits)
{
    auto helper = DeltaPrintHelper<q2proto_playerstate_delta_flags>(delta_bits);
    CHECK_FLAG(helper, Q2P_PSD_PM_TYPE);
    CHECK_FLAG(helper, Q2P_PSD_PM_TIME);
    CHECK_FLAG(helper, Q2P_PSD_PM_FLAGS);
    CHECK_FLAG(helper, Q2P_PSD_PM_GRAVITY);
    CHECK_FLAG(helper, Q2P_PSD_PM_DELTA_ANGLES);
    CHECK_FLAG(helper, Q2P_PSD_PM_VIEWHEIGHT);
    CHECK_FLAG(helper, Q2P_PSD_VIEWOFFSET);
    CHECK_FLAG(helper, Q2P_PSD_KICKANGLES);
    CHECK_FLAG(helper, Q2P_PSD_GUNINDEX);
    CHECK_FLAG(helper, Q2P_PSD_GUNSKIN);
    CHECK_FLAG(helper, Q2P_PSD_GUNFRAME);
    CHECK_FLAG(helper, Q2P_PSD_FOV);
    CHECK_FLAG(helper, Q2P_PSD_RDFLAGS);
    CHECK_FLAG(helper, Q2P_PSD_GUNRATE);
    CHECK_FLAG(helper, Q2P_PSD_CLIENTNUM);
    return helper.finish();
}

template<>
class PrintTraits<q2proto_svc_playerstate_t>
{
public:
    template<typename Ctx>
    static void members(Ctx& context, const q2proto_svc_playerstate_t& x)
    {
        OUTPUT_FIELD(context, x, delta_bits, print_player_delta_bits);

    #define PLAYER_FIELD(Flag, Field, ...)                                  \
        do {                                                                \
            if ((x.delta_bits & Q2P_PSD_##Flag) != 0)                       \
                OUTPUT_FIELD(context, x, Field __VA_OPT__(, ) __VA_ARGS__); \
        } while (false)

        PLAYER_FIELD(PM_TYPE, pm_type);
        if (x.pm_origin.read.value.delta_bits != 0)
            OUTPUT_FIELD(context, x, pm_origin);
        if (x.pm_velocity.read.value.delta_bits != 0)
            OUTPUT_FIELD(context, x, pm_velocity);
        PLAYER_FIELD(PM_TIME, pm_time);
        PLAYER_FIELD(PM_FLAGS, pm_flags, print_hex);
        PLAYER_FIELD(PM_GRAVITY, pm_gravity);
        PLAYER_FIELD(PM_DELTA_ANGLES, pm_delta_angles);
        PLAYER_FIELD(PM_VIEWHEIGHT, pm_viewheight);
        PLAYER_FIELD(VIEWOFFSET, viewoffset);
        if (x.viewangles.delta_bits != 0)
            OUTPUT_FIELD(context, x, viewangles);
        PLAYER_FIELD(KICKANGLES, kick_angles);
        PLAYER_FIELD(GUNINDEX, gunindex);
        PLAYER_FIELD(GUNSKIN, gunskin);
        PLAYER_FIELD(GUNFRAME, gunframe);
        if (x.gunoffset.delta_bits != 0)
            OUTPUT_FIELD(context, x, gunoffset);
        if (x.gunangles.delta_bits != 0)
            OUTPUT_FIELD(context, x, gunangles);
        if (x.blend.delta_bits != 0)
            OUTPUT_FIELD(context, x, blend);
        if (x.damage_blend.delta_bits != 0)
            OUTPUT_FIELD(context, x, damage_blend);
        PLAYER_FIELD(FOV, fov);
        PLAYER_FIELD(RDFLAGS, rdflags, print_hex);

        OUTPUT_FIELD(context, x, statbits, print_hex);
        if (x.statbits != 0) {
            // Build a nice-to-read map stats map
            std::map<size_t, int> stats;
            for (const auto [index, value] : std::views::enumerate(x.stats)) {
                if ((x.statbits & (1ull << index)) != 0)
                    stats.emplace(index, value);
            }
            context.field("stats", stats);
        }

        PLAYER_FIELD(GUNRATE, gunrate);
        PLAYER_FIELD(CLIENTNUM, clientnum);
        OUTPUT_FIELD(context, x, fog);

    #undef PLAYER_FIELD
    }
};

template<>
class PrintTraits<q2proto_svc_frame_t>
{
public:
    template<typename Ctx>
    static void members(Ctx& context, const q2proto_svc_frame_t& x)
    {
        OUTPUT_FIELD(context, x, serverframe);
        OUTPUT_FIELD(context, x, deltaframe);
        OUTPUT_FIELD(context, x, suppress_count);
        OUTPUT_FIELD(context, x, q2pro_frame_flags, print_hex);
        auto areabits = std::span(reinterpret_cast<const uint8_t *>(x.areabits), x.areabits_len);
        context.field("areabits", fmt::format("{::#x}", areabits));
        OUTPUT_FIELD(context, x, playerstate);
    }
};

template<>
class PrintTraits<q2proto_svc_frame_entity_delta_t>
{
public:
    template<typename Ctx>
    static void members(Ctx& context, const q2proto_svc_frame_entity_delta_t& x)
    {
        OUTPUT_FIELD(context, x, newnum);
        if (x.newnum != 0) {
            OUTPUT_FIELD(context, x, remove);
            OUTPUT_FIELD(context, x, entity_delta);
        }
    }
};

template<>
class PrintTraits<q2proto_svc_setting_t>
{
public:
    template<typename Ctx>
    static void members(Ctx& context, const q2proto_svc_setting_t& x)
    {
        OUTPUT_FIELD(context, x, index);
        OUTPUT_FIELD(context, x, value);
    }
};

template<>
class PrintTraits<std::remove_reference_t<decltype(q2proto_svc_damage_t::damage[0])>>
{
public:
    template<typename Ctx, typename T>
    static void members(Ctx& context, const T& x)
    {
        OUTPUT_FIELD(context, x, damage);
        OUTPUT_FIELD(context, x, health);
        OUTPUT_FIELD(context, x, armor);
        OUTPUT_FIELD(context, x, shield);
        OUTPUT_FIELD(context, x, direction);
    }
};

template<>
class PrintTraits<q2proto_svc_damage_t>
{
public:
    template<typename Ctx>
    static void members(Ctx& context, const q2proto_svc_damage_t& x)
    {
        OUTPUT_FIELD(context, x, count);
        for (unsigned i = 0; i < x.count; i++) {
            context.field(fmt::format("damage[{}]", i), x.damage[i]);
        }
    }
};

template<>
class PrintTraits<q2proto_svc_poi_t>
{
public:
    template<typename Ctx>
    static void members(Ctx& context, const q2proto_svc_poi_t& x)
    {
        OUTPUT_FIELD(context, x, key);
        OUTPUT_FIELD(context, x, time);
        OUTPUT_FIELD(context, x, pos);
        OUTPUT_FIELD(context, x, image);
        OUTPUT_FIELD(context, x, color);
        OUTPUT_FIELD(context, x, flags, print_hex);
    }
};

template<>
class PrintTraits<q2proto_svc_help_path_t>
{
public:
    template<typename Ctx>
    static void members(Ctx& context, const q2proto_svc_help_path_t& x)
    {
        OUTPUT_FIELD(context, x, start);
        OUTPUT_FIELD(context, x, pos);
        OUTPUT_FIELD(context, x, dir);
    }
};

template<>
class PrintTraits<q2proto_svc_achievement_t>
{
public:
    template<typename Ctx>
    static void members(Ctx& context, const q2proto_svc_achievement_t& x)
    {
        OUTPUT_FIELD(context, x, id);
    }
};

template<>
class PrintTraits<q2proto_svc_locprint_t>
{
public:
    template<typename Ctx>
    static void members(Ctx& context, const q2proto_svc_locprint_t& x)
    {
        OUTPUT_FIELD(context, x, flags, print_print_level);
        OUTPUT_FIELD(context, x, base);
        OUTPUT_FIELD(context, x, num_args);
        for (unsigned i = 0; i < x.num_args; i++) {
            context.field(fmt::format("args[{}]", i), x.args[i]);
        }
    }
};

#undef OUTPUT_FIELD

namespace print_struct_detail {
struct DummyContext {
    template <typename T> void field(std::string_view, const T &);
};

template <typename T>
concept trait_has_members = requires(const T& t, DummyContext dc) { PrintTraits<T>::members(dc, t); };

struct NameLengthContext
{
    size_t max_len = 0;
    size_t prefix_len;

    NameLengthContext(size_t prefix_len = 0) : prefix_len(prefix_len) {}

    template <typename T, typename = std::enable_if_t<!trait_has_members<T>>> void field(std::string_view name, const T &x, std::string(const T&) = PrintTraits<T>::format)
    {
        size_t name_len = name.size();
        max_len = std::max(max_len, prefix_len + name_len);
    }

    template <typename T, typename = std::enable_if_t<trait_has_members<T>>> void field(std::string_view name, const T &x)
    {
        size_t name_len = this->prefix_len + name.size();
        size_t prefix_len = name_len + 1;
        auto inner_ctx = NameLengthContext(prefix_len);
        PrintTraits<T>::members(inner_ctx, x);
        max_len = std::max(max_len, inner_ctx.max_len);
    }
};

struct PrintStructContext
{
    size_t max_name_len;
    std::string_view prefix;

    PrintStructContext(size_t max_name_len, std::string_view prefix = {}) : max_name_len(max_name_len), prefix(prefix) {}

    template <typename T, typename = std::enable_if_t<!trait_has_members<T>>> void field(std::string_view name, const T &x, std::string format(const T&) = PrintTraits<T>::format)
    {
        char label[64];
        format_to_s(label, "{}{}:", prefix, name);
        auto value = format(x);
        fmt::println("{:<12}{:<{}} {}", "", label, max_name_len + 1, value);
    }

    template <typename T, typename = std::enable_if_t<trait_has_members<T>>> void field(std::string_view name, const T &x)
    {
        char new_prefix[64];
        format_to_s(new_prefix, "{}{}.", prefix, name);
        auto print_ctx = PrintStructContext(max_name_len, new_prefix);
        PrintTraits<T>::members(print_ctx, x);
    }
};
} // namespace print_struct_detail

template <typename T>
static void PrintStruct(const T &val)
{
    print_struct_detail::NameLengthContext name_len;
    PrintTraits<T>::members(name_len, val);
    auto print_ctx = print_struct_detail::PrintStructContext(name_len.max_len);
    PrintTraits<T>::members(print_ctx, val);
}

static void print_message(const q2proto_svc_message_t& msg)
{
    fmt::println("{:<8}{}:", "", msg.type);
    switch(msg.type)
    {
    case Q2P_SVC_INVALID:
    case Q2P_SVC_NOP:
    case Q2P_SVC_DISCONNECT:
    case Q2P_SVC_RECONNECT:
        // nothing to print
        break;
    case Q2P_SVC_MUZZLEFLASH:
    case Q2P_SVC_MUZZLEFLASH2:
        PrintStruct(msg.muzzleflash);
        break;
    case Q2P_SVC_TEMP_ENTITY:
        PrintStruct(msg.temp_entity);
        break;
    case Q2P_SVC_SOUND:
        PrintStruct(msg.sound);
        break;
    case Q2P_SVC_PRINT:
        PrintStruct(msg.print);
        break;
    case Q2P_SVC_STUFFTEXT:
        PrintStruct(msg.stufftext);
        break;
    case Q2P_SVC_SERVERDATA:
        PrintStruct(msg.serverdata);
        break;
    case Q2P_SVC_CONFIGSTRING:
        PrintStruct(msg.configstring);
        break;
    case Q2P_SVC_SPAWNBASELINE:
        PrintStruct(msg.spawnbaseline);
        break;
    case Q2P_SVC_CENTERPRINT:
        PrintStruct(msg.centerprint);
        break;
    case Q2P_SVC_DOWNLOAD:
        PrintStruct(msg.download);
        break;
    case Q2P_SVC_FRAME:
        PrintStruct(msg.frame);
        break;
    case Q2P_SVC_INVENTORY:
        PrintStruct(msg.inventory);
        break;
    case Q2P_SVC_LAYOUT:
        PrintStruct(msg.layout);
        break;
    case Q2P_SVC_FRAME_ENTITY_DELTA:
        PrintStruct(msg.frame_entity_delta);
        break;
    case Q2P_SVC_SETTING:
        PrintStruct(msg.setting);
        break;
    case Q2P_SVC_DAMAGE:
        PrintStruct(msg.damage);
        break;
    case Q2P_SVC_FOG:
        PrintStruct(msg.fog);
        break;
    case Q2P_SVC_POI:
        PrintStruct(msg.poi);
        break;
    case Q2P_SVC_HELP_PATH:
        PrintStruct(msg.help_path);
        break;
    case Q2P_SVC_ACHIEVEMENT:
        PrintStruct(msg.achievement);
        break;
    case Q2P_SVC_LOCPRINT:
        PrintStruct(msg.locprint);
        break;
    default:
        fmt::println("TODO: support message type {}", msg.type);
    }
}

// Print stdio error
template<typename... T>
static int print_io_error(int code, fmt::format_string<T...> fmt, T&&... args)
{
    auto ec = std::make_error_code(static_cast<std::errc>(code));
    fmt::print(stderr, fmt, std::forward<T>(args)...);
    fmt::println(stderr, ": {:s}", ec);
    return ec.value();
}

// Print q2proto error
template<typename... T>
static void print_q2proto_error(q2proto_error_t err, fmt::format_string<T...> fmt, T&&... args)
{
    fmt::print(stderr, fmt, std::forward<T>(args)...);
    fmt::println(stderr, ": {}", q2proto_error_string(err));
}

// Check result of a q2proto call
template<typename... T>
static bool check_q2proto_result(q2proto_error_t result, fmt::format_string<T...> fmt, T&&... args)
{
    if (result != Q2P_ERR_SUCCESS) {
        print_q2proto_error(result, std::move(fmt), std::forward<T>(args)...);
        return false;
    }
    return true;
}

// Check result of a q2proto call, exit in case of error
#define CHECK_Q2PROTO(EXPR)                                \
    if (!check_q2proto_result((EXPR), "failed {}", #EXPR)) \
        return -4;

static nonstd::expected<void, int> read_file(FILE* f, void* buf, size_t size)
{
    size_t num_read = fread(buf, size, 1, f);
    if (num_read != 1) {
        if (feof(f)) {
            fmt::println(stderr, "unexpected end of file");
            return nonstd::make_unexpected(-2);
        } else
            return nonstd::make_unexpected(print_io_error(errno, "read error"));
    }
    return {};
}

int main(int argc, const char* argv[])
{
    if (argc != 2) {
        fmt::println(stderr, "Syntax: {} [demofile]", argv[0]);
        return -1;
    }

    q2proto_clientcontext_t demo_context;
    if (!check_q2proto_result(q2proto_init_clientcontext(&demo_context), "failed to initialize client context"))
        return -4;

    auto *demo_file = fopen(argv[1], "rb");
    if (!demo_file) {
        return print_io_error(errno, "failed to open \"{}\"", argv[1]);
    }

    auto close_file = nonstd::make_scope_exit([&] { fclose(demo_file); });

    static constexpr size_t max_packet_size = 0x10000;
    auto buf = std::unique_ptr<std::byte>(new std::byte[max_packet_size]);

    while (true) {
        auto demo_pos = ftell(demo_file);
        fmt::println("file position: {}", demo_pos);

        uint32_t packet_size;
        auto read_result = read_file(demo_file, &packet_size, sizeof(packet_size));
        if (!read_result)
            return read_result.error();
        if constexpr (std::endian::native != std::endian::little)
            packet_size = std::byteswap(packet_size);

        if (packet_size == (uint32_t)-1)
            break;
        if (packet_size > max_packet_size) {
            fmt::println(stderr, "packet too large ({} > {})", packet_size, max_packet_size);
            return -3;
        }

        read_result = read_file(demo_file, buf.get(), packet_size);
        if (!read_result)
            return read_result.error();

        auto io_ctx = io_context(buf.get(), packet_size);
        uintptr_t io_arg = reinterpret_cast<uintptr_t>(&io_ctx);

        q2proto_svc_message_t msg;
        while (true) {
            auto client_read_result = q2proto_client_read(&demo_context, io_arg, &msg);
            if (client_read_result == Q2P_ERR_NO_MORE_INPUT)
                break;
            else if (!check_q2proto_result(client_read_result, "failed to read message"))
                return -5;
            print_message(msg);
        }
    }
    fmt::println("--- end of stream ---");

    close_file.release();

    return 0;
}