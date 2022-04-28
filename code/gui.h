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

  r32 time; // a joining of separate r32 scale, angle;
  // every frame: 
  // state->time += input->frame_delta;
  // V2 origin = {0, 0}; make screen centre
  // V2 x_axis = 100.0f * V2(cosine(time), sine(time)); (the 100.0f is the scaling of these axis)
  // V2 y_axis = V2(x_axis.y, x_axis.x); (this is perpendicular to the x axis)
  // (could skew, if say y_axis = V2(cosine(time) + 0.5f, sine(time) + 0.5f);)
  // draw_coordinate(back_buffer, origin, x_axis, y_axis, {1, 1, 0, 1});
};

INTERNAL void
draw_coordinate()
{
  V2 dim = {2, 2};
  V2 p = origin;
  draw_rect(back_buffer, p - dim, p + dim, colour.r, colour.g, colour.b);

  // this is 1-unit along the x-axis?
  p = origin + x_axis;
  draw_rect(back_buffer, p - dim, p + dim, colour.r, colour.g, colour.b);

  p = origin + y_axis;
  draw_rect(back_buffer, p - dim, p + dim, colour.r, colour.g, colour.b);

  for (u32 point_index = 0;
       point_index < ARRAY_COUNT(points);
       ++point_index)
  {
    p = origin + point->x * x_axis + point->y * y_axis;
    draw_rect(back_buffer, p - dim, p + dim, colour.r, colour.g, colour.b);
  }
}

INTERNAL void
draw_rect(BackBuffer *back_buffer, V2 min, V2 max, V4 colour)
{

}

struct DrawCoordinate
{
  V2 origin, x_axis, y_axis;
  V4 colour;
  
  V2 points[16];
};
