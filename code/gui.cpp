// SPDX-License-Identifier: zlib-acknowledgement

#define STBTT_STATIC
#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

INTERNAL LoadedBitmap
make_empty_bitmap()
{
  result.mem = push_size(arena, size, 16);
}

INTERNAL void
draw_bitmap()
{

}

INTERNAL LoadedBitmap
load_font(FileIO *file_io, const char *file_name)
{
  LoadedBitmap result = {};

  ReadFileResult text_file = file_io->read_entire_file("file.txt");
  if (text_file.mem != NULL)
  {
    stbtt_fontinfo font = {};
    stbtt_InitFont(&font, (u8 *)text_file.mem, 
                   stbtt_GetFontOffsetForIndex((u8 *)text_file.mem, 0));

    // codepoint uniquely identifies glyph
    //
    // 128.0f --> 128pixel height
    int width, height, offset_x, offset_y;
    u8 *monochrome_bitmap = stbtt_GetCodepointBitmap(&font, 0.0f, 
                                                     stbtt_ScaleForPixelHeight(&font, 128.0f),
                                                     'S', &width, &height, &offset_x, 
                                                     &offset_y);

    result = make_empty_bitmap(mem_arena, width, height, false);

    u8 *bitmap_mem = monochrome_bitmap;
    u32 *dest_mem = result.mem;
    for (u32 y = 0;
         y < height;
         y++)
    {
      for (u32 x = 0;
          x < width;
          x++)
      {
        u8 alpha = *bitmap_mem++;
        *dest_mem++ = (alpha << 24 | alpha << 16 | alpha << 8);
      }
    }

    stbtt_FreeBitmap(monochrome_bitmap, NULL);
  }
}


INTERNAL void
update_and_render(BackBuffer *back_buffer, Input *input, FileIO *file_io)
{
  TIMED_BLOCK();

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
}
