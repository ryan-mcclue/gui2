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
    result = file_stat.st_mtim.tv_sec;
  }
  else
  {
    EBP();
  }
#endif

  return result;
}

struct LoadableUpdateAndRender
{
  const char *base_file;
  const char *toggle_file; 
  u64 base_mod_time, toggle_mod_time, current_mod_time;
  void *load_handle;
  UpdateAndRender update_and_render;
};

INTERNAL UpdateAndRender
reload_update_and_render(LoadableUpdateAndRender *loadable_update_and_render)
{
  UpdateAndRender result = loadable_update_and_render->update_and_render;

  u64 base_mod_time = get_file_modification_time(loadable_update_and_render->base_file);
  u64 toggle_mod_time = get_file_modification_time(loadable_update_and_render->toggle_file);
  u64 most_recent_mod_time = MAX(base_mod_time, toggle_mod_time);

  if (most_recent_mod_time > loadable_update_and_render->current_mod_time)
  {
    const char *load_file = NULL;

    if (base_mod_time > toggle_mod_time)
    {
      load_file = loadable_update_and_render->base_file;
    }
    else
    {
      load_file = loadable_update_and_render->toggle_file;
    }

    void *load_handle = SDL_LoadObject(load_file);
    if (load_handle != NULL)
    {
      // load("debug_frame_end");
      UpdateAndRender update_and_render = \
        (UpdateAndRender)SDL_LoadFunction(load_handle, "update_and_render"); 
      if (update_and_render != NULL)
      {
        if (loadable_update_and_render->load_handle != NULL)
        {
          SDL_UnloadObject(loadable_update_and_render->load_handle);
        }

        loadable_update_and_render->load_handle = load_handle; 
        loadable_update_and_render->current_mod_time = most_recent_mod_time;
        loadable_update_and_render->base_mod_time = base_mod_time;
        loadable_update_and_render->toggle_mod_time = toggle_mod_time;
        loadable_update_and_render->update_and_render = update_and_render;

        result = update_and_render;
      }
      else
      {
        BP_MSG(SDL_GetError());
      }
    }
    else
    {
      // TODO(Ryan): Distinguish between the false positive whereby we attempt to load
      // the shared object before it has been completley written too. This generates
      // ".so file too short".
      // BP_MSG(SDL_GetError());
    }
  }

  return result;
}

INTERNAL r32
get_elapsed_seconds(u64 last_timer, u64 current_timer)
{
  r32 result = 0;

  return (current_timer - last_timer) / SDL_GetPerformanceFrequency();

  return result;
}

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

            Functions functions = {};
            functions.read_entire_file = read_entire_file;
            functions.free_file_result = free_file_result;

            LoadableUpdateAndRender loadable_update_and_render = {};
            loadable_update_and_render.base_file = "/home/ryan/prog/personal/gui/run/gui.so";
            loadable_update_and_render.toggle_file = "/home/ryan/prog/personal/gui/run/gui.so.toggle";

            UpdateAndRender current_update_and_render = \
              reload_update_and_render(&loadable_update_and_render);
            if (current_update_and_render != NULL)
            {
              DebugFrameEndInfo debug_info = {};

              u64 last_counter = SDL_GetPerformanceCounter(); 

              bool want_to_run = true;
              while (want_to_run)
              {
                // FRAME_MARKER();
                
                SDL_Event event = {};
                // BEGIN_BLOCK(EventProcessing);
                while (SDL_PollEvent(&event))
                {
                  if (event.type == SDL_QUIT)
                  {
                    want_to_run = false;
                  }
                }
                // END_BLOCK();

                debug_info.events_processed = get_elapsed_seconds(last_counter, 
                                                                  SDL_GetPerformanceCounter());

                SDL_RenderClear(renderer);

                current_update_and_render = \
                  reload_update_and_render(&loadable_update_and_render);

                s32 pitch = 0;
                if (SDL_LockTexture(back_buffer_texture, NULL, (void **)&back_buffer.pixels, 
                      &pitch) == 0)
                {
                  current_update_and_render(&back_buffer, &input, &memory, &functions);

                  SDL_UnlockTexture(back_buffer_texture);

                  SDL_RenderCopy(renderer, back_buffer_texture, NULL, NULL);
                }
                else
                {
                  BP_MSG(SDL_GetError());
                }

                SDL_RenderPresent(renderer);

                u64 end_counter = SDL_GetPerformanceCounter();
                last_counter = end_counter;

                debug_info.end_of_frame = get_elapsed_seconds(last_counter, end_counter);

                // if (game_functions->debug_frame_end)
                // {
                // IMPORTANT(Ryan): No longer need this, as store in globals
                // game_functions->debug_frame_end(&memory, &debug_info)
                // };
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

