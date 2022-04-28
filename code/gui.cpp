// SPDX-License-Identifier: zlib-acknowledgement

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
draw_bitmap(BackBuffer *back_buffer, LoadedBitmap *bitmap, r32 x, r32 y, 
            s32 align_x = 0, s32 align_y = 0)
{
  x -= (r32)align_x;
  y -= (r32)align_y;

  s32 min_x = round_r32_to_s32(x);
  s32 max_x = round_r32_to_s32(x + bitmap->width);
  s32 min_y = round_r32_to_s32(y);
  s32 max_y = round_r32_to_s32(y + bitmap->height);

  if (max_x > (s32)back_buffer->dim.w) 
  {
    max_x = back_buffer->dim.w;
  }
  if (max_y > (s32)back_buffer->dim.h)
  {
    max_y = back_buffer->dim.h;
  }

  s32 left_offset_x = 0;
  if (min_x < 0) 
  {
    left_offset_x = -min_x;
    min_x = 0;
  }

  s32 top_offset_y = 0;
  if (min_y < 0) 
  {
    top_offset_y = -min_y;
    min_y = 0;
  }

  u32 *bitmap_row = (u32 *)bitmap->pixels;
  bitmap_row += (bitmap->width * top_offset_y) + left_offset_x;

  u32 *back_buffer_row = (u32 *)back_buffer->pixels + (back_buffer->dim.w * min_y + min_x);
  for (s32 y = min_y; 
       y < max_y; 
       ++y)
  {
    u32 *buffer_cursor = back_buffer_row;
    u32 *bitmap_cursor = bitmap_row;

    for (s32 x = min_x; 
         x < max_x; 
         ++x)
    {
      // TODO(Ryan): Gamma correction
      r32 alpha_blend_t = (*bitmap_cursor & 0xFF) / 255.0f;
      
      r32 red_orig = (*buffer_cursor >> 24);
      r32 new_red = (*bitmap_cursor >> 24);
      r32 red_blended = lerp(red_orig, new_red, alpha_blend_t);

      r32 green_orig = (*buffer_cursor >> 16 & 0xFF);
      r32 new_green = (*bitmap_cursor >> 16 & 0xFF);
      r32 green_blended = lerp(green_orig, new_green, alpha_blend_t);

      r32 blue_orig = (*buffer_cursor >> 8 & 0xFF);
      r32 new_blue = (*bitmap_cursor >> 8 & 0xFF);
      r32 blue_blended = lerp(blue_orig, new_blue, alpha_blend_t);

      *buffer_cursor = round_r32_to_u32(red_blended) << 24 | 
                       round_r32_to_u32(green_blended) << 16 | 
                       round_r32_to_u32(blue_blended) << 8 | 
                       0xff;

      bitmap_cursor++;
      buffer_cursor++;
    }

    back_buffer_row += back_buffer->dim.w;
    bitmap_row += bitmap->width;
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
          stbtt_ScaleForPixelHeight(&font, 256.0f),
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

/*
 * 1. Piece contains bitmap and x, y location
 */

// draw_bitmap(scale/height, color);

INTERNAL void
draw_text(BackBuffer *back_buffer, const char *text, Font *font, r32 x, r32 y)
{
  LOCAL_PERSIST r32 moving_y = 0.0f;

  r32 moving_x = 0.0f;
  for (const char *ch_cursor = text; *ch_cursor != '\0'; ++ch_cursor)
  {
    LoadedBitmap ch_bitmap = font->glyphs[*ch_cursor];

    draw_bitmap(back_buffer, &ch_bitmap, moving_x, moving_y);

    moving_x += ch_bitmap.width;
  }

  moving_y
}

INTERNAL void
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

  u32 *pixels = back_buffer->pixels;

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

  draw_text(back_buffer, "HI THERE", &state->font, 100, 100);

}
