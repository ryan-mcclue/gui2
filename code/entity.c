// SPDX-License-Identifier: zlib-acknowledgement

typedef struct Entity
{
  b32 exists;
  TileMapPosition pos;
  V2 dpos;
  u32 facing_direction;
} Entity;

typedef struct State
{
  u32 player_index_for_controller[ARRAY_COUNT(controller)];
  Entity entities[256];
} State;

int
main(int argc, char *argv[])
{
  for (Controller c: controllers)
  {
    // mapping of these indexes more robust?
    // wrap into get_entity()
    Entity *entity = state->entities[state->player_index_for_controller[c_index]];
    b32 are_controlling_an_entity = entity->exists;
    if (are_controlling_an_entity)
    {

    }
    else
    {
      if (c->start.down)
      {
        initialise_player();
      }
    }

    // character is four bitmaps
    if (absolute_value(dplayer.x) > absolute_value(dplayer.y))
    {
      if (dplayer.x > 0)
      {
        hero_facing_direction = 0;
      }
      else
      {
        hero_facing_direction = 0;
      }
    }
    else
    {
      if (dplayer.y > 0)
      {
        hero_facing_direction = 0;
      }
      else
      {
        hero_facing_direction = 0;
      }
    }

    if (c.left.is_down)
    {
      ddplayer.y = 1.0f;
    }
  }

  for (Entity e: entities)
  {
    if (e->exists)
    {
      draw_entity();
    }
  }

  return 0;
}
