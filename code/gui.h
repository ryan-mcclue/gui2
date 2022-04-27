// SPDX-License-Identifier: zlib-acknowledgement
#pragma once

struct LoadedBitmap
{
  u32 width, height;
  void *pixels;
};

struct MemArena
{
  u8 *base;
  u64 size;
  u64 used;
};

struct State
{
  b32 is_initialised;

  MemArena mem_arena;

  LoadedBitmap font['Z' - 'A' + 1];
};
