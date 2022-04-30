// SPDX-License-Identifier: zlib-acknowledgement
#include "types.h"
#include "debug.h"
#include "math.h"
#include "vector.h"
#include "platform.h"

#include "gui.h"

#define STBTT_STATIC
#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

INTERNAL MemArena
create_mem_arena(void *mem, u64 size)
{
  MemArena result = {};

  result.base = (u8 *)mem;
  result.size = size;
  result.used = 0;

  return result;
}

#define MEM_RESERVE_STRUCT(arena, struct_name) \
  (struct_name *)(obtain_mem(arena, sizeof(struct_name)))

#define MEM_RESERVE_ARRAY(arena, elem, len) \
  (elem *)(obtain_mem(arena, sizeof(elem) * (len)))

INTERNAL void *
obtain_mem(MemArena *arena, u64 size)
{
  void *result = NULL;
  ASSERT(arena->used + size < arena->size);

  result = (u8 *)arena->base + arena->used;
  arena->used += size;

  return result;
}

INTERNAL LoadedBitmap
create_empty_bitmap(MemArena *mem_arena, u32 width, u32 height, b32 want_to_clear = true)
{
  LoadedBitmap result = {};

  result.width = width;
  result.height = height;
  result.pixels = MEM_RESERVE_ARRAY(mem_arena, u32, width * height);

  u32 *pixel_cursor = (u32 *)result.pixels;
  if (want_to_clear)
  {
    for (u32 y = 0;
        y < height;
        ++y)
    {
      for (u32 x = 0;
          x < width;
          ++x)
      {
        *pixel_cursor++ = 0xff; 
      }
    }
  }

  return result;
}

INTERNAL void
draw_rect(BackBuffer *back_buffer, V2 origin, V2 x_axis, V2 y_axis, V4 colour)
{
  u32 colour32 = round_r32_to_u32(colour.r * 255.0f) << 24 | 
                 round_r32_to_u32(colour.g * 255.0f) << 16 | 
                 round_r32_to_u32(colour.b * 255.0f) << 8 | 
                 round_r32_to_u32(colour.a * 255.0f);

  s32 max_width = (back_buffer->dim.w - 1);
  s32 max_height = (back_buffer->dim.h - 1);

  s32 x_min = max_width;
  s32 y_min = max_height;
  s32 x_max = 0;
  s32 y_max = 0;

  V2 points[4] = {origin, origin + x_axis, origin + x_axis + y_axis, origin + y_axis};
  for (u32 p_index = 0;
       p_index < 4;
       ++p_index)
  {
    V2 point = points[p_index];
    s32 floor_x = floor_r32_to_s32(point.x);
    s32 ceil_x = floor_r32_to_s32(point.x);
    s32 floor_y = floor_r32_to_s32(point.y);
    s32 ceil_y = floor_r32_to_s32(point.y);

    if (x_min > floor_x)
    {
      x_min = floor_x;
    }
    if (y_min > floor_y)
    {
      y_min = floor_y;
    }
    if (x_max < ceil_x)
    {
      x_max = ceil_x;
    }
    if (y_max < ceil_y)
    {
      y_max = ceil_y;
    }
  }

  if (x_min < 0)
  {
    x_min = 0;
  }
  if (y_min < 0)
  {
    y_min = 0;
  }
  if (x_max > max_width)
  {
    x_max = max_width;
  }
  if (y_max > max_height)
  {
    y_max = max_height;
  }


  for (s32 y = y_min;
       y < y_max;
       ++y)
  {
    for (s32 x = x_min;
        x < x_max;
        ++x)
    {
      V2 pixel_p = v2((r32)x, (r32)y);
      V2 d = pixel_p - origin;

      // subtract origin to have rooted at appropriate rect corners
      r32 top_edge = vec_dot(pixel_p - origin, -vec_perp(x_axis));
      r32 right_edge = vec_dot(pixel_p - (origin + x_axis), -vec_perp(y_axis));
      r32 bottom_edge = vec_dot(pixel_p - (origin + x_axis + y_axis), vec_perp(x_axis));
      r32 left_edge = vec_dot(pixel_p - (origin + y_axis), vec_perp(y_axis));

      if (top_edge < 0 && right_edge < 0 && bottom_edge < 0 && left_edge < 0)
      {
        u32 *pixel = back_buffer->pixels + (back_buffer->dim.w * y + x);
        *pixel = colour32;
      }

    }
  }

}

// TODO(Ryan): alignment values
INTERNAL void
draw_bitmap(BackBuffer *back_buffer, LoadedBitmap *bitmap, r32 scale, V2 origin, V4 colour)
{
  V2 x_axis = scale * v2(bitmap->width, 0);
  V2 y_axis = scale * v2(0, bitmap->height);

  r32 inv_x_axis_squared = 1.0f / vec_length_sq(x_axis);
  r32 inv_y_axis_squared = 1.0f / vec_length_sq(y_axis);

  s32 max_width = (back_buffer->dim.w - 1);
  s32 max_height = (back_buffer->dim.h - 1);

  s32 x_min = max_width;
  s32 y_min = max_height;
  s32 x_max = 0;
  s32 y_max = 0;

  V2 points[4] = {origin, origin + x_axis, origin + x_axis + y_axis, origin + y_axis};
  for (u32 p_index = 0;
       p_index < 4;
       ++p_index)
  {
    V2 point = points[p_index];
    s32 floor_x = floor_r32_to_s32(point.x);
    s32 ceil_x = floor_r32_to_s32(point.x);
    s32 floor_y = floor_r32_to_s32(point.y);
    s32 ceil_y = floor_r32_to_s32(point.y);

    if (x_min > floor_x)
    {
      x_min = floor_x;
    }
    if (y_min > floor_y)
    {
      y_min = floor_y;
    }
    if (x_max < ceil_x)
    {
      x_max = ceil_x;
    }
    if (y_max < ceil_y)
    {
      y_max = ceil_y;
    }
  }

  if (x_min < 0)
  {
    x_min = 0;
  }
  if (y_min < 0)
  {
    y_min = 0;
  }
  if (x_max > max_width)
  {
    x_max = max_width;
  }
  if (y_max > max_height)
  {
    y_max = max_height;
  }


  for (s32 y = y_min;
       y < y_max;
       ++y)
  {
    for (s32 x = x_min;
        x < x_max;
        ++x)
    {
      V2 pixel_p = v2((r32)x, (r32)y);
      V2 d = pixel_p - origin;

      // subtract origin to have rooted at appropriate rect corners
      r32 top_edge = vec_dot(pixel_p - origin, -vec_perp(x_axis));
      r32 right_edge = vec_dot(pixel_p - (origin + x_axis), -vec_perp(y_axis));
      r32 bottom_edge = vec_dot(pixel_p - (origin + x_axis + y_axis), vec_perp(x_axis));
      r32 left_edge = vec_dot(pixel_p - (origin + y_axis), vec_perp(y_axis));

      if (top_edge < 0 && right_edge < 0 && bottom_edge < 0 && left_edge < 0)
      {
        u32 *pixel = back_buffer->pixels + (back_buffer->dim.w * y + x);

        r32 u = inv_x_axis_squared * vec_dot(d, x_axis);
        r32 v = inv_y_axis_squared * vec_dot(d, y_axis);
          
        // TODO(Ryan): Smoother with lerp (day 093)
        s32 texel_x = round_r32_to_s32(u * (r32)(bitmap->width - 1));
        s32 texel_y = v * (bitmap->height - 1);
        u32 *texel = (u32 *)bitmap->pixels + (bitmap->width * texel_y + texel_x);

        // TODO(Ryan): Gamma correction
        r32 alpha_blend_t = (*texel & 0xFF) / 255.0f;

        r32 red_orig = (*pixel >> 24);
        r32 new_red = (*texel >> 24) * colour.r;
        r32 red_blended = lerp(red_orig, new_red, alpha_blend_t);

        r32 green_orig = (*pixel >> 16 & 0xFF);
        r32 new_green = (*texel >> 16 & 0xFF) * colour.g;
        r32 green_blended = lerp(green_orig, new_green, alpha_blend_t);

        r32 blue_orig = (*pixel >> 8 & 0xFF);
        r32 new_blue = (*texel >> 8 & 0xFF) * colour.b;
        r32 blue_blended = lerp(blue_orig, new_blue, alpha_blend_t);

        u32 colour32 = round_r32_to_u32(red_blended) << 24 | 
                       round_r32_to_u32(green_blended) << 16 | 
                       round_r32_to_u32(blue_blended) << 8 | 
                       0xff;

        *pixel = colour32;
      }
    }
  }
}

INTERNAL Font
load_font(MemArena *mem_arena, FileIO *file_io, const char *file_name)
{
  Font result = {};

  ReadFileResult font_file = file_io->read_entire_file(file_name);
  if (font_file.mem != NULL)
  {
    stbtt_fontinfo font = {};
    stbtt_InitFont(&font, (u8 *)font_file.mem, 
                   stbtt_GetFontOffsetForIndex((u8 *)font_file.mem, 0));

    for (u32 codepoint = PRINTABLE_FONT_GLYPH_START;
         codepoint != PRINTABLE_FONT_GLYPH_END;
         codepoint++)
    {
      int width, height, offset_x, offset_y = 0;
      u8 *monochrome_bitmap = stbtt_GetCodepointBitmap(&font, 0.0f, 
          stbtt_ScaleForPixelHeight(&font, 128.0f),
          codepoint, &width, &height, &offset_x, 
          &offset_y);

      LoadedBitmap ch_bitmap = create_empty_bitmap(mem_arena, width, height, false);

      u8 *bitmap_mem = monochrome_bitmap;
      u32 *dest_mem = (u32 *)ch_bitmap.pixels;
      for (s32 y = 0;
          y < height;
          y++)
      {
        for (s32 x = 0;
            x < width;
            x++)
        {
          u8 alpha = *bitmap_mem++;
          *dest_mem++ = (alpha << 24 | alpha << 16 | alpha << 8 | alpha);
        }
      }

      result.glyphs[codepoint] = ch_bitmap;

      stbtt_FreeBitmap(monochrome_bitmap, NULL);
    }

    file_io->free_file_result(&font_file);
  }

  return result;
}

INTERNAL void
draw_text(BackBuffer *back_buffer, const char *text, Font *font, V2 origin, V2 x_axis)
{
  LOCAL_PERSIST r32 moving_y = 0.0f;

  r32 moving_x = 0.0f;
  for (const char *ch_cursor = text; *ch_cursor != '\0'; ++ch_cursor)
  {
    LoadedBitmap ch_bitmap = font->glyphs[*ch_cursor];

    //draw_bitmap(back_buffer, &ch_bitmap, moving_x, moving_y);

    moving_x += ch_bitmap.width;
  }
}

extern "C" void
update_and_render(BackBuffer *back_buffer, Input *input, Memory *memory, FileIO *file_io)
{
  TIMED_BLOCK();

  State *state = (State *)memory->mem;
  if (!state->is_initialised)
  {
    u8 *start_mem = (u8 *)memory->mem + sizeof(State);
    u64 start_mem_size = memory->size - sizeof(State);
    state->mem_arena = create_mem_arena(start_mem, start_mem_size);

    state->font = load_font(&state->mem_arena, file_io, 
                            "/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf");

    state->is_initialised = true;
  }

  state->time += input->update_dt;


  u32 *pixels = back_buffer->pixels;

  // could remove this clearing
  for (u32 y = 0;
       y < back_buffer->dim.h;
       ++y)
  {
    for (u32 x = 0;
        x < back_buffer->dim.w;
        ++x)
    {
      *pixels++ = 0xff0000ff;     
    }
  }

  V2 origin = v2(back_buffer->dim.w * 0.5f, back_buffer->dim.h * 0.5f);
  //V2 x_axis = (50.0f + 50.0f * cosine(state->time)) * v2(cosine(state->time), sine(state->time));
  V4 colour = v4(sine(state->time), 0, 1, 1);

  draw_bitmap(back_buffer, &state->font.glyphs['A'], 1.2f, origin, v4(1, 1, 1, 1));
}

__extension__ DebugRecord debug_records[__COUNTER__];

INTERNAL void
overlay_debug_records(void)
{
  for (u32 debug_i = 0;
       debug_i < ARRAY_COUNT(debug_records);
       ++debug_i)
  {
    DebugRecord record = debug_records[debug_i];
    if (record.hit_count > 0)
    {
      printf("%s\n", record.function_name);
    }
  }
}
