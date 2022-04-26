// SPDX-License-Identifier: zlib-acknowledgement

#include "types.h"
#include "debug.h"
#include "math.h"
#include "vector.h"

#include "gui.h"

#include <SDL2/SDL.h>

INTERNAL void
update_and_render(BackBuffer *back_buffer, Input *input)
{
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
                update_and_render(&back_buffer, &input);
                
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

