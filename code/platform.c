// SPDX-License-Identifier: zlib-acknowledgement
#include "SDL.h"
#include <SDL2/SDL_ttf.h>

#include "types.h"
#include "debug.h"
#include "math.h"
#include "vector.h"
#include "platform.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "io.c"


INTERNAL u32
get_refresh_rate(SDL_Window *window)
{
  u32 result = 0;

  SDL_DisplayMode display_mode = {0};

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

typedef struct LoadableUpdateAndRender
{
  const char *base_file;
  const char *toggle_file; 
  u64 base_mod_time, toggle_mod_time, current_mod_time;
  void *load_handle;
  UpdateAndRender update_and_render;
} LoadableUpdateAndRender;

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
      __extension__ UpdateAndRender update_and_render = \
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

INTERNAL void
map_window_mouse_to_render_mouse(SDL_Window *window, SDL_Renderer *renderer, Input *input)
{
  s32 current_window_width, current_window_height = 0;
  SDL_GetWindowSize(window, &current_window_width, &current_window_height);

  s32 logical_render_width, logical_render_height = 0;
  SDL_RenderGetLogicalSize(renderer, &logical_render_width, &logical_render_height);

  s32 window_mouse_x, window_mouse_y = 0;
  u32 mouse_state = SDL_GetMouseState(&window_mouse_x, &window_mouse_y);

  r32 mouse_x_norm = (r32)window_mouse_x / (r32)current_window_width;
  r32 mouse_y_norm = (r32)window_mouse_y / (r32)current_window_height;

  s32 corrected_mouse_x = round_r32_to_s32(mouse_x_norm * logical_render_width); 
  s32 corrected_mouse_y = round_r32_to_s32(mouse_y_norm * logical_render_height); 

  input->mouse_x = corrected_mouse_x;
  input->mouse_y = corrected_mouse_y;
}

int
main(int argc, char *argv[])
{
  if (SDL_Init(SDL_INIT_VIDEO) == 0)
  {
    if (TTF_Init() == 0)
    {
      V2u window_dim = {0};
      window_dim.w = 1280; 
      window_dim.h = 720;
      SDL_Window *window = SDL_CreateWindow("Name", SDL_WINDOWPOS_CENTERED, 
          SDL_WINDOWPOS_CENTERED, window_dim.w, window_dim.h, 
          SDL_WINDOW_RESIZABLE);
      if (window != NULL)
      {
        SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC);
        if (renderer != NULL)
        {
          // IMPORTANT(Ryan): Mouse values line up to this logical width and height
          SDL_RenderSetLogicalSize(renderer, window_dim.w, window_dim.h);
          // SDL_RenderSetIntegerScale(renderer, SDL_TRUE);

          u32 memory_size = GIGABYTES(1);
          void *mem = calloc(memory_size, 1);
          if (mem != NULL)
          {
            Memory memory = {0};
            memory.size = memory_size;
            memory.mem = mem;

            // TODO(Ryan): Will have to call again if in fullscreen mode
            Input input[2] = {0};
            Input *cur_input = &input[0];
            Input *prev_input = &input[1];
            u32 refresh_rate = get_refresh_rate(window);
            cur_input->update_dt = 1.0f / (r32)refresh_rate;

            LoadableUpdateAndRender loadable_update_and_render = {0};
            loadable_update_and_render.base_file = "/home/ryan/prog/personal/gui/run/gui.so";
            loadable_update_and_render.toggle_file = "/home/ryan/prog/personal/gui/run/gui.so.toggle";

            UpdateAndRender current_update_and_render = \
                                                        reload_update_and_render(&loadable_update_and_render);
            if (current_update_and_render != NULL)
            {
              b32 want_to_run = true;
              while (want_to_run)
              {
                SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);


                SDL_Event event = {0};
                while (SDL_PollEvent(&event))
                {
                  switch (event.type)
                  {
                    case SDL_QUIT:
                      {
                        want_to_run = false;
                      } break;
                    case SDL_MOUSEBUTTONDOWN:
                    case SDL_MOUSEBUTTONUP:
                      {
                        if (event.button.button == SDL_BUTTON_LEFT) 
                        {
                          cur_input->mouse_left.is_down = (event.button.state == SDL_PRESSED);
                        }
                        if (event.button.button == SDL_BUTTON_MIDDLE)
                        {
                          cur_input->mouse_middle.is_down = (event.button.state == SDL_PRESSED);
                        }
                        if (event.button.button == SDL_BUTTON_RIGHT)
                        {
                          cur_input->mouse_right.is_down = (event.button.state == SDL_PRESSED);
                        }
                      } break;
                  }
                }

                map_window_mouse_to_render_mouse(window, renderer, cur_input);

                current_update_and_render = \
                                            reload_update_and_render(&loadable_update_and_render);

                SDL_RenderClear(renderer);

                for (u32 input_button_i = 0;
                    input_button_i < ARRAY_COUNT(cur_input->buttons);
                    ++input_button_i)
                {
                  DigitalButton *cur_button = &cur_input->buttons[input_button_i];
                  DigitalButton *prev_button = &prev_input->buttons[input_button_i];

                  if (prev_button->is_down && !cur_button->is_down)
                  {
                    cur_button->was_down = true;
                  }
                }

                current_update_and_render(renderer, cur_input, &memory);

                for (u32 input_button_i = 0;
                    input_button_i < ARRAY_COUNT(cur_input->buttons);
                    ++input_button_i)
                {
                  DigitalButton *cur_button = &cur_input->buttons[input_button_i];
                  cur_button->was_down = false;
                }

                *prev_input = *cur_input;

                SDL_RenderPresent(renderer);
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
      BP_MSG(TTF_GetError());
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

