// SPDX-License-Identifier: zlib-acknowledgement

typedef struct LowEntity
{

} LowEntity;

typedef struct DormantEntity
{
  TileMapPosition pos;
  u32 width, height;
} DormantEntity;

typedef struct HighEntity
{
  V2 pos; // this is camera relative already
  V2 dpos;
  u32 facing_direction;
} Entity;

typedef enum
{
  ENTITY_RESIDENCE_NONEXISTANT,
  ENTITY_RESIDENCE_HIGH,
  ENTITY_RESIDENCE_LOW,
  ENTITY_RESIDENCE_DORMANT,
} ENTITY_RESIDENCE;

typedef enum
{
  ENTITY_TYPE_HERO,
  ENTITY_TYPE_FAMILIAR,
  ENTITY_TYPE_MONSTER,
} ENTITY_TYPE;

// this Entity struct is what is passed around and the specific entity values are modified appropriatley
typedef struct Entity
{
  ENTITY_RESIDENCE residence;
  HighEntity *high_entity;
  LowEntity *low_entity;
  DormantEntity *dormant_entity;
} Entity;

// draw_entity(), update_entity()
typedef struct State
{
  u32 player_index_for_controller[ARRAY_COUNT(controller)];
  u32 entity_count;
  ENTITY_RESIDENCE entity_residences[256];
  HighEntity high_entities[256];
  LowEntity low_entities[256];
  DormantEntity dormant_entities[256];
} State;

Entity
get_entity(State *state, ENTITY_RESIDENCE residence, u32 index)
{
  // this implicitly sets NONEXISTANT residence
  Entity result = {0};

  if (index >= 0 && index < state->entity_count)
  {
    entity.residence = residence;
    entity.high_entity = &state->high_entities[index];
    entity.low_entity = &state->low_entities[index];
    entity.dormant_entity = &state->dormant_entities[index];
  }

  return result;
}

// TODO(Ryan): Draw from camera

int
main(int argc, char *argv[])
{
  for (Controller c: controllers)
  {
    // so, at the moment we are only distinguish between non existant and in residence
    Entity entity = get_entity(state, HIGH_RESIDENCE, c_index);

    if (entity.residence != RESIDENCE_NOT_EXISTANT)
    {

    }
    else
    {
      if (c->start.down)
      {
        initialise_player(); // add_entity(dormant_residence); values... ; set_entity_residence(high)
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
    if (state->entity_residences[e_index] == ENTITY_RESIDENCE_HIGH)
    {
      HighEntity high_entity = state->high_entities[e_index];
      draw_entity();
    }
  }

  return 0;
}
