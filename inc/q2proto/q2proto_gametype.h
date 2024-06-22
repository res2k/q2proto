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
 * Server game type
 */
#ifndef Q2PROTO_GAMETYPE_H_
#define Q2PROTO_GAMETYPE_H_

/// Type of game run by the server
typedef enum q2proto_game_type_e
{
    /// Vanilla/original/v3 game
    Q2PROTO_GAME_VANILLA = 0,
    /// "Q2PRO extended" game
    Q2PROTO_GAME_Q2PRO_EXTENDED,
} q2proto_game_type_t;

#endif // Q2PROTO_GAMETYPE_H_
