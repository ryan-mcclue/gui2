// SPDX-License-Identifier: zlib-acknowledgement

typedef struct DigitalButton
{
  b32 is_down, was_down;
} DigitalButton;

typedef struct Input
{
  s32 mouse_x, mouse_y;
  union
  {
    struct
    {
      DigitalButton mouse_left;
      DigitalButton mouse_right; 
      DigitalButton mouse_middle;
    };
    DigitalButton buttons[3];
  };

  r32 update_dt;
} Input;

typedef struct Memory
{
  u64 size;
  void *mem;
} Memory;

typedef void (*UpdateAndRender)(SDL_Renderer *, Input *, Memory *);
void update_and_render(SDL_Renderer *renderer, Input *input, Memory *memory);
