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
struct MonospaceFont
{
  LoadedBitmap glyphs[FONT_GLYPH_COUNT];
  r32 width, height;
};

struct Glyph
{
  LoadedBitmap bitmap;
  r32 align_percentage[2];
};

struct State
{
  b32 is_initialised;

  MemArena mem_arena;

  MonospaceFont font;

  r32 time; 
};

