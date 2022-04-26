// SPDX-License-Identifier: zlib-acknowledgement

struct BackBuffer
{
  V2 dim;
  u32 *pixels;
};

struct DigitalButton
{
  bool is_down, was_down;
};

struct Input
{
  s32 mouse_x, mouse_y;
  DigitalButton mouse_left, mouse_right, mouse_middle;

  r32 update_delta;
};
