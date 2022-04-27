// SPDX-License-Identifier: zlib-acknowledgement

#include "types.h"
#include "debug.h"
#include "math.h"
#include "vector.h"
#include "platform.h"

#include <SDL2/SDL.h>

#include "gui.cpp"

// u64 counter = SDL_GetPerformanceCounter();
// u64 freq = SDL_GetPerformanceFrequency();
// u64 cycle_count = __rdtsc();

INTERNAL void
free_file_result(ReadFileResult *read_file_result)
{
  free(read_file_result->mem);
}

INTERNAL ReadFileResult
read_entire_file(const char *file_name)
{
  ReadFileResult result = {};

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
      while (bytes_read_total < file_size && bytes_read != 0)
      {
        bytes_read = SDL_RWread(rw_ops, (u8 *)file_mem + bytes_read, 1, bytes_to_read);

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

int
main(int argc, char *argv[])
{
  if (SDL_Init(SDL_INIT_VIDEO) == 0)
  {
    V2 window_dim = {1280, 720};
    SDL_Window *window = SDL_CreateWindow("Name", SDL_WINDOWPOS_CENTERED, 
                          SDL_WINDOWPOS_CENTERED, window_dim.w, window_dim.h, 
                          SDL_WINDOW_RESIZABLE);
    if (window != NULL)
    {
      SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC);
      if (renderer != NULL)
      {
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

        // IMPORTANT(Ryan): Mouse values line up to this logical width and height
        SDL_RenderSetLogicalSize(renderer, window_dim.w, window_dim.h);
        SDL_RenderSetIntegerScale(renderer, SDL_TRUE);

        V2 back_buffer_dim = window_dim;
        SDL_Texture *back_buffer_texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, 
                                                             SDL_TEXTUREACCESS_STREAMING,
                                                             back_buffer_dim.w, 
                                                             back_buffer_dim.h);
        if (back_buffer_texture != NULL)
        {
          u32 *back_buffer_pixels = (u32 *)malloc(back_buffer_dim.w * back_buffer_dim.h * sizeof(u32));
          if (back_buffer_pixels != NULL)
          {
            BackBuffer back_buffer = {};
            back_buffer.dim = back_buffer_dim;
            back_buffer.pixels = back_buffer_pixels;

            Input input = {};
            
            FileIO file_io = {};
            file_io.read_entire_file = read_entire_file;
            file_io.free_file_result = free_file_result;

            bool want_to_run = true;
            while (want_to_run)
            {
              SDL_Event event = {};
              while (SDL_PollEvent(&event))
              {
                if (event.type == SDL_QUIT)
                {
                  want_to_run = false;
                }
              }

              SDL_RenderClear(renderer);

              s32 pitch = 0;
              if (SDL_LockTexture(back_buffer_texture, NULL, (void **)&back_buffer.pixels, 
                    &pitch) == 0)
              {
                update_and_render(&back_buffer, &input, &file_io);
                
                SDL_UnlockTexture(back_buffer_texture);

                SDL_RenderCopy(renderer, back_buffer_texture, NULL, NULL);
              }
              else
              {
                BP_MSG(SDL_GetError());
              }
      
              SDL_RenderPresent(renderer);
            }
          }
          else
          {
            EBP();
            SDL_Quit();
            return 1;
          }
        }
        else
        {
          BP_MSG(SDL_GetError());
          SDL_Quit();
          return 1;
        }
      }
      else
      {
        BP_MSG(SDL_GetError());
        SDL_Quit();
        return 1;
      }
    }
    else
    {
      BP_MSG(SDL_GetError());
      SDL_Quit();
      return 1;
    }
  }
  else
  {
    BP_MSG(SDL_GetError());
    return 1;
  }

  SDL_Quit();

  return 0;
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
