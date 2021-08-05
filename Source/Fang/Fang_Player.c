// Copyright (C) 2021  Antonio Lassandro

// This program is free software: you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation, either version 3 of the License, or (at your option)
// any later version.

// This program is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
// more details.

// You should have received a copy of the GNU General Public License along
// with this program.  If not, see <http://www.gnu.org/licenses/>.

void
Fang_PlayerUpdate(
    Fang_State  * const state,
    Fang_Entity * const player)
{
    assert(state);
    assert(player);

    if (player->state == FANG_ENTITYSTATE_CREATING)
        player->state = FANG_ENTITYSTATE_ACTIVE;
    else if (player->state == FANG_ENTITYSTATE_REMOVING)
        Fang_EntitySetRemove(&state->entities, player->id);
}

void
Fang_PlayerCollide(
          Fang_Entity * const player,
          Fang_Entity * const entity,
    const bool                initial_collision)
{
    assert(player);
    assert(entity);
    assert(player->type == FANG_ENTITYTYPE_PLAYER);

    (void)initial_collision;
}
