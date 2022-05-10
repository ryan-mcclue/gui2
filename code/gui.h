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

  MemArena mem_arena;

  r32 time; 
} State;
