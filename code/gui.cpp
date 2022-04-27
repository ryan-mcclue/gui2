// SPDX-License-Identifier: zlib-acknowledgement

#define STBTT_STATIC
#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

INTERNAL void
update_and_render(BackBuffer *back_buffer, Input *input, FileIO *file_io)
{
  TIMED_BLOCK();

  ReadFileResult text_file = file_io->read_entire_file("file.txt");
  BP();

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
