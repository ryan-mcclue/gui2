// SPDX-License-Identifier: zlib-acknowledgement

struct BackBuffer
{
  V2u dim;
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

  r32 update_dt;
};

struct Memory
{
  u64 size;
  void *mem;

  u64 debug_size;
  void *debug_memory;
};

struct ReadFileResult
{
  u64 file_size;
  void *mem;
};

struct DebugFrameEndInfo
{
  r32 events_processed; 
  r32 end_of_frame; 
};

typedef ReadFileResult (*ReadEntireFile)(const char *file_name);
typedef void (*FreeFileResult)(ReadFileResult *read_file_result);
struct Functions
{
  ReadEntireFile read_entire_file;
  FreeFileResult free_file_result;
};

typedef void (*UpdateAndRender)(BackBuffer *, Input *, Memory *, Functions *);
extern "C" void
update_and_render(BackBuffer *back_buffer, Input *input, Memory *memory, Functions *functions);
