// SPDX-License-Identifier: zlib-acknowledgement
#pragma once

#include "io.h"

INTERNAL void
free_file_result(ReadFileResult *read_file_result)
{
  free(read_file_result->mem);
}

INTERNAL ReadFileResult
read_entire_file(const char *file_name)
{
  ReadFileResult result = {0};

  SDL_RWops *rw_ops = SDL_RWFromFile(file_name, "rb");  
  if (rw_ops != NULL)
  {
    u64 file_size = SDL_RWsize(rw_ops); 

    void *file_mem = malloc(file_size);
    if (file_mem != NULL)
    {
      u64 bytes_read_total = 0;
      u64 bytes_read = 1;
      u64 bytes_to_read = file_size;
      u8 *mem_iterator = (u8 *)file_mem;
      while (bytes_read_total < file_size && bytes_read != 0)
      {
        bytes_read = SDL_RWread(rw_ops, mem_iterator, 1, bytes_to_read);

        mem_iterator += bytes_read;
        bytes_read_total += bytes_read;
        bytes_to_read -= bytes_read_total; 
      }
      SDL_RWclose(rw_ops);

      result.file_size = file_size;
      result.mem = file_mem;
    }
    else
    {
      EBP();
      SDL_RWclose(rw_ops);
    }
  }
  else
  {
    BP_MSG(SDL_GetError());
  }

  return result;
}

INTERNAL u64 
get_file_modification_time(const char *file_name)
{
  u64 result = 0;

#if defined(GUI_LINUX)
  struct stat file_stat = {0};
  if (stat(file_name, &file_stat) == 0)
  {
    result = file_stat.st_mtim.tv_sec;
  }
  else
  {
    EBP();
  }
#endif

  return result;
}
