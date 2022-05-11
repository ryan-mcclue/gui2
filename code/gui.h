// SPDX-License-Identifier: zlib-acknowledgement
#pragma once

typedef struct MemArena
{
  u8 *base;
  u64 size;
  u64 used;
} MemArena;

typedef struct State
{
  b32 is_initialised;
  b32 is_paused;

  MemArena mem_arena;

  TTF_Font *font;

  r32 time; 
} State;
