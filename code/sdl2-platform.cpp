// SPDX-License-Identifier: zlib-acknowledgement

#include "types.h"
#include "debug.h"
#include "math.h"
#include "vector.h"
#include "platform.h"

#include <SDL2/SDL.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/sendfile.h>
#include <fcntl.h>

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

INTERNAL u32
get_refresh_rate(SDL_Window *window)
{
  u32 result = 0;

  SDL_DisplayMode display_mode = {};

  s32 display_index = SDL_GetWindowDisplayIndex(window);
  if (SDL_GetCurrentDisplayMode(display_index, &display_mode) == 0)
  {
    // TODO(Ryan): This doesn't fully handle a variable refresh rate monitor
    if (display_mode.refresh_rate == 0)
    {
      result = 60;
    }
    else
    {
      result = display_mode.refresh_rate;
    }
  }
  else
  {
    BP_MSG(SDL_GetError());
    result = 60;
  }

  return result;
}

INTERNAL u64 
get_file_modification_time(const char *file_name)
{
  u64 result = 0;

#if defined(GUI_LINUX)
  struct stat file_stat = {};
  if (stat(file_name, &file_stat) == 0)
  {
    result = file_stat.st_mtim.tv_nsec;
  }
  else
  {
    EBP();
  }
#endif

  return result;
}

INTERNAL void
copy_file(const char *src, const char *dst)
{
  ReadFileResult src_read = read_entire_file(src);
  if (src_read.mem != NULL)
  {
    SDL_RWops *dst_ops = SDL_RWFromFile(dst, "wb");  
    if (dst_ops != NULL)
    {
      if (SDL_RWwrite(dst_ops, src_read.mem, src_read.file_size, 1) < 1) 
      {
        BP_MSG(SDL_GetError());
      }

      SDL_RWclose(dst_ops);
    }

    free_file_result(&src_read);
  }
}

#if 0
INTERNAL void
copy_file(const char *source, const char *dest)
{
#if defined(GUI_LINUX)
  s32 src_fd = open(source, O_RDONLY);
  if (src_fd == 0)
  {
    s32 dst_fd = open(dest, O_CREAT | O_WRONLY | O_TRUNC, 0777);
    //s32 dst_fd = creat(dest, 0777);
    if (dst_fd == 0)
    {
      struct stat src_file_stat = {};
      if (fstat(src_fd, &src_file_stat) == 0)
      {
        s64 src_file_size = src_file_stat.st_size;
        // ioctl(dest_fd, FICLONE, src_fd);
        if (sendfile(dst_fd, src_fd, NULL, src_file_size) != src_file_size) 
        {
          EBP();
        }
        close(src_fd);
        close(dst_fd);
      }
      else
      {
        close(src_fd);
        close(dst_fd);
      }
    }
    else
    {
      close(src_fd);
      EBP();
    }
  }
  else
  {
    EBP();
  }
#endif

}
#endif

int
main(int argc, char *argv[])
{
  if (SDL_Init(SDL_INIT_VIDEO) == 0)
  {
    V2u window_dim = {1280, 720};
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

        V2u back_buffer_dim = window_dim;
        SDL_Texture *back_buffer_texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, 
                                                             SDL_TEXTUREACCESS_STREAMING,
                                                             back_buffer_dim.w, 
                                                             back_buffer_dim.h);
        if (back_buffer_texture != NULL)
        {
          u32 back_buffer_size = back_buffer_dim.w * back_buffer_dim.h * sizeof(u32);
          u32 memory_size = GIGABYTES(1);
          void *mem = calloc(back_buffer_size + memory_size, 1);
          if (mem != NULL)
          {
            BackBuffer back_buffer = {};
            back_buffer.dim = back_buffer_dim;
            back_buffer.pixels = (u32 *)mem;

            // TODO(Ryan): Will have to call again if in fullscreen mode
            Input input = {};
            u32 refresh_rate = get_refresh_rate(window);
            input.update_dt = 1.0f / (r32)refresh_rate;
            
            Memory memory = {};
            memory.size = memory_size;
            memory.mem = (u8 *)mem + back_buffer_size;

            FileIO file_io = {};
            file_io.read_entire_file = read_entire_file;
            file_io.free_file_result = free_file_result;

            u64 current_update_and_render_mod_time = 0;
            u64 new_update_and_render_mod_time = 0;
            void *current_update_and_render_so = NULL;
            void *new_update_and_render_so = NULL;
            UpdateAndRender current_update_and_render = NULL;
            UpdateAndRender new_update_and_render = NULL;


#define BASE_FILE "/home/ryan/prog/personal/gui/run/gui.so"
#define COPY_FILE BASE_FILE".copy"

            copy_file(BASE_FILE, COPY_FILE);
            current_update_and_render_so = SDL_LoadObject(COPY_FILE);
            if (current_update_and_render_so != NULL)
            {
              current_update_and_render = \
                (UpdateAndRender)SDL_LoadFunction(current_update_and_render_so, 
                                                  "update_and_render");
              if (current_update_and_render != NULL)
              {
                current_update_and_render_mod_time = get_file_modification_time(BASE_FILE);

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
                  
                  new_update_and_render_mod_time = get_file_modification_time(BASE_FILE);
                  if (new_update_and_render_mod_time != current_update_and_render_mod_time)
                  {
                    copy_file(BASE_FILE, COPY_FILE);
                    new_update_and_render_so = SDL_LoadObject(COPY_FILE);
                    if (new_update_and_render_so != NULL)
                    {
                      new_update_and_render = \
                       (UpdateAndRender)SDL_LoadFunction(new_update_and_render_so, 
                                                         "update_and_render");
                      if (new_update_and_render != NULL)
                      {
                        SDL_UnloadObject(current_update_and_render_so);
                        current_update_and_render_so = new_update_and_render_so;
                        current_update_and_render_mod_time = new_update_and_render_mod_time;
                        current_update_and_render = new_update_and_render;
                      }
                      else
                      {
                        BP_MSG(SDL_GetError());
                      }
                    }
                    else
                    {
                      BP_MSG(SDL_GetError());
                    }
                  }

                  s32 pitch = 0;
                  if (SDL_LockTexture(back_buffer_texture, NULL, (void **)&back_buffer.pixels, 
                        &pitch) == 0)
                  {
                    current_update_and_render(&back_buffer, &input, &memory, &file_io);

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
                BP_MSG(SDL_GetError());
                SDL_Quit();
              }
            }
            else
            {
              BP_MSG(SDL_GetError());
              SDL_Quit();
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

