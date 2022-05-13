// SPDX-License-Identifier: zlib-acknowledgement
#pragma once


typedef struct Texture
{
  SDL_Texture *tex;
  V4 colour_mod;
} Texture;

#define PRINTABLE_ASCII_START 33
#define PRINTABLE_ASCII_END 126
#define PRINTABLE_ASCII_RANGE (PRINTABLE_ASCII_END - PRINTABLE_ASCII_START)
#define ASCII_END 127
typedef struct CapitalMonospacedFont
{
  Texture glyphs[ASCII_END]; 
  s32 width, height;
} CapitalMonospacedFont;

typedef struct State
{
  b32 is_initialised;
  b32 is_paused;

  MemoryArena mem_arena;

  CapitalMonospacedFont font;

  r32 time; 
} State;
