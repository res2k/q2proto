/*
Copyright (C) 2024 Frank Richter

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

/**\file
 * Server command (SVC) structures and definitions
 */
#ifndef Q2PROTO_STRUCT_SVC_H_
#define Q2PROTO_STRUCT_SVC_H_

#include "q2proto_coords.h"
#include "q2proto_string.h"

#include <stdbool.h>
#include <stdint.h>

#if defined(__cplusplus)
extern "C" {
#endif

/// Supported number of inventory items
#define Q2PROTO_INVENTORY_ITEMS 256
/// Supported number of stats
#define Q2PROTO_STATS           64

/// Flag bits for fields set in a q2proto_entity_state_delta_t structure
enum q2proto_entity_state_delta_flags
{
    /// 'modelindex' is set
    Q2P_ESD_MODELINDEX = 0x1,
    /// 'modelindex2' is set
    Q2P_ESD_MODELINDEX2 = 0x2,
    /// 'modelindex3' is set
    Q2P_ESD_MODELINDEX3 = 0x4,
    /// 'modelindex4' is set
    Q2P_ESD_MODELINDEX4 = 0x8,
    /// 'frame' is set
    Q2P_ESD_FRAME = 0x10,
    /// 'skinnum' is set
    Q2P_ESD_SKINNUM = 0x20,
    /**
     * 'effects' is set.
     * Note: due to different transmit granularities in different protocols,
     * if setting either Q2P_ESD_EFFECTS or Q2P_ESD_EFFECTS_MORE during writing,
     * both 'effects' and 'effects_more' must be set!
     */
    Q2P_ESD_EFFECTS = 0x40,
    /**
     * 'effects_more' is set.
     * Note: due to different transmit granularities in different protocols,
     * if setting either Q2P_ESD_EFFECTS or Q2P_ESD_EFFECTS_MORE during writing,
     * both 'effects' and 'effects_more' must be set!
     */
    Q2P_ESD_EFFECTS_MORE = 0x80,
    /// 'renderfx' is set.
    Q2P_ESD_RENDERFX = 0x100,
    /// 'old_origin' is set
    Q2P_ESD_OLD_ORIGIN = 0x200,
    /// 'sound' is set
    Q2P_ESD_SOUND = 0x400,
    /// 'loop_attenuation' is set. Only applies if Q2P_ESD_SOUND is also set.
    Q2P_ESD_LOOP_ATTENUATION = 0x800,
    /// 'loop_volume' is set. Only applies if Q2P_ESD_SOUND is also set.
    Q2P_ESD_LOOP_VOLUME = 0x1000,
    /// 'event' is set
    Q2P_ESD_EVENT = 0x2000,
    /// 'solid' is set
    Q2P_ESD_SOLID = 0x4000,
    /// 'alpha' is set
    Q2P_ESD_ALPHA = 0x8000,
    /// 'scale' is set
    Q2P_ESD_SCALE = 0x10000,
};

/// Delta between entity states
typedef struct q2proto_entity_state_delta_s {
    /// Combination of q2proto_entity_state_delta_flags, indicating which fields are set
    uint32_t delta_bits;
    /// modelindex
    uint16_t modelindex;
    /// modelindex2
    uint16_t modelindex2;
    /// modelinde3
    uint16_t modelindex3;
    /// modelindex4
    uint16_t modelindex4;
    /// frame
    uint16_t frame;
    /// skinnum
    uint32_t skinnum;
    /// effects
    uint32_t effects;
    /// effects (upper 32 bit)
    uint32_t effects_more;
    /// renderfx
    uint32_t renderfx;
    /// origin
    q2proto_maybe_diff_coord_t origin;
    /// angle
    q2proto_angle_delta_t angle;
    /// old_origin
    q2proto_var_coord_t old_origin;
    /// sound
    uint16_t sound;
    /// sound loop volume
    uint8_t loop_volume;
    /// sound loop attenuation
    uint8_t loop_attenuation;
    /// event
    uint8_t event;
    /// solid
    uint32_t solid;
    /// alpha
    uint8_t alpha;
    /// scale
    uint8_t scale;
} q2proto_entity_state_delta_t;

/// Contents from a muzzleflash or muzzleflash2 message
typedef struct q2proto_svc_muzzleflash_s {
    /// entity
    int16_t entity;
    /// weapon
    uint16_t weapon;
    /// whether silenced flag was set
    bool silenced;
} q2proto_svc_muzzleflash_t;

/// Contents from a temp_entity message
typedef struct q2proto_svc_temp_entity_s {
    /// TEnt type. Determines which fields are set and what the mean.
    uint8_t type;
    /// position1
    q2proto_vec3_t position1;
    /// position2
    q2proto_vec3_t position2;
    /// offset
    q2proto_vec3_t offset;
    /// direction
    q2proto_vec3_t direction;
    /// count
    uint8_t count;
    /// color
    uint8_t color;
    /// entity1
    int16_t entity1;
    /// entity2
    int16_t entity2;
    /// time
    int32_t time;
} q2proto_svc_temp_entity_t;

/// Contents from a layout message
typedef struct q2proto_svc_layout_s {
    /// Layout string
    q2proto_string_t layout_str;
} q2proto_svc_layout_t;

/// Contents from an inventory message
typedef struct q2proto_svc_inventory_s {
    /// Inventory values
    int16_t inventory[Q2PROTO_INVENTORY_ITEMS];
} q2proto_svc_inventory_t;

/// Contents from a sound message
typedef struct q2proto_svc_sound_s {
    /// flags
    uint8_t flags;
    /// index
    uint16_t index;
    /// volume
    uint8_t volume;
    /// attenuation
    uint8_t attenuation;
    /// timeofs
    uint8_t timeofs;
    /// entity
    uint16_t entity;
    /// channel
    uint8_t channel;
    /// pos
    q2proto_var_coord_t pos;
} q2proto_svc_sound_t;


/// Contents from a print message
typedef struct q2proto_svc_print_s {
    /// Print level
    uint8_t level;
    /// Print string
    q2proto_string_t string;
} q2proto_svc_print_t;

/// Contents from a stufftext message

typedef struct q2proto_svc_stufftext_s {
    /// Command to stuff
    q2proto_string_t string;
} q2proto_svc_stufftext_t;

/// Contents from a configstring message
typedef struct q2proto_svc_configstring_s {
    /// Configstring index
    uint16_t index;
    /// Configstring value
    q2proto_string_t value;
} q2proto_svc_configstring_t;


/// Contents from a spawnbaseline message
typedef struct q2proto_svc_spawnbaseline_s {
    /// entity number
    uint16_t entnum;
    /// entity state delta
    q2proto_entity_state_delta_t delta_state;
} q2proto_svc_spawnbaseline_t;

/// Contents from a centerpring message
typedef struct q2proto_svc_centerprint_s {
    /// Message to print
    q2proto_string_t message;
} q2proto_svc_centerprint_t;


/// Contents from a download message
typedef struct q2proto_svc_download_s {
    /// Chunk size
    int16_t size;
    /// Percent completed
    uint8_t percent;
    /// Download data
    const void *data;
} q2proto_svc_download_t;

/// Contents from a serverdata message
typedef struct q2proto_svc_serverdata_s {
    /// Protocol
    int32_t protocol;
    /// Server count
    int32_t servercount;
    /// Attract loop flag
    bool attractloop;
    /// gamedir
    q2proto_string_t gamedir;
    /// Client entity number
    int16_t clientnum;
    /// Level name
    q2proto_string_t levelname;

    /// R1Q2, Q2PRO: protocol version
    uint16_t protocol_version;
    /// R1Q2, Q2PRO: strafejump hack enabled
    bool strafejump_hack;

    /// R1Q2 specific serverdata
    struct {
        /// R1Q2: "enhanced" flag
        bool enhanced;
    } r1q2;

    /// Q2PRO specific serverdata
    struct {
        /// Q2PRO: Server state
        uint8_t server_state;
        /// Q2PRO: QW mode enabled
        bool qw_mode;
        /// Q2PRO: Waterjump hack enabled
        bool waterjump_hack;
        /**
         * Q2PRO: Extensions enabled
         * Note: It's recommended you let q2proto_server_fill_serverdata() set this flag
         * and use \c server_game_type stored in the client context to check the game type.
         */
        bool extensions;
        /**
         * Q2PRO: "Extensions v2" enabled
         * Note: It's recommended you let q2proto_server_fill_serverdata() set this flag
         * and use \c server_game_type stored in the client context to check the game type.
         */
        bool extensions_v2;
    } q2pro;
} q2proto_svc_serverdata_t;

/// Flag bits for fields set in a q2proto_svc_playerstate_t structure
enum q2proto_playerstate_delta_flags
{
    /// 'pm_type' is set
    Q2P_PSD_PM_TYPE = 0x1,
    /// 'pm_time' is set
    Q2P_PSD_PM_TIME = 0x2,
    /// 'pm_flags' is set
    Q2P_PSD_PM_FLAGS = 0x4,
    /// 'pm_gravity' is set
    Q2P_PSD_PM_GRAVITY = 0x8,
    /// 'pm_delta_angles' is set
    Q2P_PSD_PM_DELTA_ANGLES = 0x10,
    /// 'viewoffset' is set
    Q2P_PSD_VIEWOFFSET = 0x20,
    /// 'kick_angles' is set
    Q2P_PSD_KICKANGLES = 0x40,
    /// 'gunindex' is set
    Q2P_PSD_GUNINDEX = 0x80,
    /**
     * 'gunframe' is set.
     * Note: due to different transmit granularities in different protocols,
     * if setting just one of the 'gun'  _bits_ during writing,
     * all of gunframe, gunoffset, gunangles have to be set!
     */
    Q2P_PSD_GUNFRAME = 0x100,
    /**
     * 'gunoffset' is set.
     * Note: due to different transmit granularities in different protocols,
     * if setting just one of the 'gun'  _bits_ during writing,
     * all of gunframe, gunoffset, gunangles have to be set!
     */
    Q2P_PSD_GUNOFFSET = 0x200,
    /**
     * 'gunangles' is set.
     * Note: due to different transmit granularities in different protocols,
     * if setting just one of the 'gun'  _bits_ during writing,
     * all of gunframe, gunoffset, gunangles have to be set!
     */
    Q2P_PSD_GUNANGLES = 0x400,
    /// 'fov' is set
    Q2P_PSD_FOV = 0x800,
    /// 'rdflags' is set
    Q2P_PSD_RDFLAGS = 0x1000,
    /// 'clientnum' is set. Requires q2proto_servercontext_t::features.playerstate_clientnum!
    Q2P_PSD_CLIENTNUM = 0x2000,
};

/// Player state delta, as contained in frame messages
typedef struct q2proto_svc_playerstate_s {
    /// Combination of q2proto_playerstate_delta_flags, indicating which fields are set
    uint32_t delta_bits;
    /// pmove type
    uint8_t pm_type;
    /// pmove origin
    q2proto_maybe_diff_coord_t pm_origin;
    /// pmove velocity
    q2proto_maybe_diff_coord_t pm_velocity;
    /// pmove time
    uint16_t pm_time;
    /// pmove flags
    uint16_t pm_flags;
    /// pmove gravity
    int16_t pm_gravity;
    /// pmove delta angles
    q2proto_var_angle_t pm_delta_angles;
    /// viewoffset
    q2proto_var_small_offset_t viewoffset;
    /// viewangles
    q2proto_angle_delta_t viewangles;
    /// kick_angles
    q2proto_var_small_angle_t kick_angles;
    /// gunindex
    uint16_t gunindex;
    /// gunframe
    uint8_t gunframe;
    /// gunoffset
    q2proto_var_small_offset_t gunoffset;
    /// gunangles
    q2proto_var_small_angle_t gunangles;
    /// screen blend
    q2proto_blend_delta_t blend;
    /// damage blend (for Q2PRO extended v2 games)
    q2proto_blend_delta_t damage_blend;
    /// fov
    uint8_t fov;
    /// renderflags
    uint8_t rdflags;
    /// Bit mask indicating which stats entries are set
    uint64_t statbits;
    /// stats entries
    int16_t stats[Q2PROTO_STATS];
    /// client number
    int16_t clientnum;
} q2proto_svc_playerstate_t;

/**
 * Contents of a frame message.
 * Frame messages are special as this struct does _not_ contain all the
 * data from the message.
 * Upon reading, the entity deltas will be returned as a sequence of Q2P_SVC_FRAME_ENTITY_DELTA
 * pseudo-message.
 * Upon writing, the entity deltas have to be immediately written as a sequence of
 * Q2P_SVC_FRAME_ENTITY_DELTA pseudo-messages, and terminated by such a message with
*  newnum == 0.
 */
typedef struct q2proto_svc_frame_s {
    /// server frame
    int32_t serverframe;
    /// delta frame
    int32_t deltaframe;
    /// suppress count
    uint8_t suppress_count;
    /// frame flags (Q2PRO)
    uint8_t q2pro_frame_flags;
    /// length, in bytes, of areabits data
    uint8_t areabits_len;
    /// areabits data
    const void *areabits;
    /// Player state delta
    q2proto_svc_playerstate_t playerstate;
} q2proto_svc_frame_t;

/// Entity delta from a frame message
typedef struct q2proto_svc_frame_entity_delta_s {
    /**
     * Entity number.
     * A zero entity number indicates the "end of entity deltas"
     * (and is not a valid entity delta itself).
     */
    uint16_t newnum;
    /// Whether to remove entity
    bool remove;
    /// Entity state delta
    q2proto_entity_state_delta_t entity_delta;
} q2proto_svc_frame_entity_delta_t;

/// R1Q2, Q2PRO server setting
typedef struct q2proto_svc_setting_s {
    /// Setting index
    int32_t index;
    /// Setting value
    int32_t value;
} q2proto_svc_setting_t;

/// Types of message from server
typedef enum q2proto_svc_message_type_e
{
    /// Invalid
    Q2P_SVC_INVALID,
    /// Muzzle flashes (player)
    Q2P_SVC_MUZZLEFLASH,
    /// Muzzle flashes (monsters)
    Q2P_SVC_MUZZLEFLASH2,
    /// Temp entity
    Q2P_SVC_TEMP_ENTITY,
    /// No-op
    Q2P_SVC_NOP,
    /// Disconnect notification
    Q2P_SVC_DISCONNECT,
    /// Reconnect notification
    Q2P_SVC_RECONNECT,
    /// Play a sound
    Q2P_SVC_SOUND,
    /// Print to console
    Q2P_SVC_PRINT,
    /// Stuff commands into client command buffer
    Q2P_SVC_STUFFTEXT,
    /// Server data (for initial protocol initialization)
    Q2P_SVC_SERVERDATA,
    /// Config string
    Q2P_SVC_CONFIGSTRING,
    /// Spawn baseline
    Q2P_SVC_SPAWNBASELINE,
    /// Print to center of screen
    Q2P_SVC_CENTERPRINT,
    /// File download piece
    Q2P_SVC_DOWNLOAD,
    /**
     * Frame update info.
     * Frame messages are special as q2proto_svc_frame_t does _not_ contain all the
     * data from the message.
     * Upon reading, the entity deltas will be returned as a sequence of Q2P_SVC_FRAME_ENTITY_DELTA
     * pseudo-message.
     * Upon writing, the entity deltas have to be immediately written as a sequence of
     * Q2P_SVC_FRAME_ENTITY_DELTA pseudo-messages, and terminated by such a message with
     * newnum == 0.
     */
    Q2P_SVC_FRAME,
    /// Inventory update info
    Q2P_SVC_INVENTORY,
    /// Layout update info
    Q2P_SVC_LAYOUT,
    /// Pseudo-message used for reading or writing a single frame entity delta
    Q2P_SVC_FRAME_ENTITY_DELTA,
    /// R1Q2, Q2PRO server setting
    Q2P_SVC_SETTING,
}
q2proto_svc_message_type_t;

/// A single message, received from the server
typedef struct q2proto_svc_message_s {
    /**
     * Type of message. Determines the contained data.
     * Note: Some messages don't have any additional data beyond the type.
     */
    q2proto_svc_message_type_t type;

    union
    {
        /// Q2P_SVC_MUZZLEFLASH, Q2P_SVC_MUZZLEFLASH2 message
        q2proto_svc_muzzleflash_t muzzleflash;
        /// Q2P_SVC_MUZZLEFLASH2 message
        q2proto_svc_temp_entity_t temp_entity;
        /// Q2P_SVC_SOUND message
        q2proto_svc_sound_t sound;
        /// Q2P_SVC_PRINT message.
        q2proto_svc_print_t print;
        /// Q2P_SVC_STUFFTEXT message
        q2proto_svc_stufftext_t stufftext;
        /// Q2P_SVC_SERVERDATA message
        q2proto_svc_serverdata_t serverdata;
        /// Q2P_SVC_CONFIGSTRING message
        q2proto_svc_configstring_t configstring;
        /// Q2P_SVC_SPAWNBASELINE message
        q2proto_svc_spawnbaseline_t spawnbaseline;
        /// Q2P_SVC_CENTERPRINT message
        q2proto_svc_centerprint_t centerprint;
        /// Q2P_SVC_DOWNLOAD message
        q2proto_svc_download_t download;
        /**
         * Q2P_SVC_FRAME message.
         * Frame messages are special as q2proto_svc_frame_t does _not_ contain all the
         * data from the message.
         * Upon reading, the entity deltas will be returned as a sequence of Q2P_SVC_FRAME_ENTITY_DELTA
         * pseudo-message.
         * Upon writing, the entity deltas have to be immediately written as a sequence of
         * Q2P_SVC_FRAME_ENTITY_DELTA pseudo-messages, and terminated by such a message with
         * newnum == 0.
         */
        q2proto_svc_frame_t frame;
        /// Q2P_SVC_INVENTORY message
        q2proto_svc_inventory_t inventory;
        /// Q2P_SVC_LAYOUT message
        q2proto_svc_layout_t layout;
        /// Q2P_SVC_FRAME_ENTITY_DELTA pseudo-message
        q2proto_svc_frame_entity_delta_t frame_entity_delta;
        /// Q2P_SVC_SETTING message
        q2proto_svc_setting_t setting;
    };
} q2proto_svc_message_t;

#if defined(__cplusplus)
} // extern "C"
#endif

#endif // Q2PROTO_STRUCT_SVC_H_