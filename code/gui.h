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

#define FONT_GLYPH_COUNT 127
#define PRINTABLE_FONT_GLYPH_START 33
#define PRINTABLE_FONT_GLYPH_END 126
struct Font
{
  LoadedBitmap glyphs[FONT_GLYPH_COUNT];
};

struct State
{
  b32 is_initialised;

  MemArena mem_arena;

  Font font;
};
