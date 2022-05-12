// SPDX-License-Identifier: zlib-acknowledgement
#pragma once

#include <x86intrin.h>

#if defined(GUI_INTERNAL)
  INTERNAL void __bp(char const *file_name, char const *func_name, int line_num,
                     char const *optional_message)
  { 
    fprintf(stderr, "BREAKPOINT TRIGGERED! (%s:%s:%d)\n\"%s\"\n", file_name, func_name, 
            line_num, optional_message);
#if !defined(GUI_DEBUGGER)
    exit(1);
#endif
  }
  INTERNAL void __ebp(char const *file_name, char const *func_name, int line_num)
  { 
    char *errno_msg = strerror(errno);
    fprintf(stderr, "ERRNO BREAKPOINT TRIGGERED! (%s:%s:%d)\n\"%s\"\n", file_name, 
            func_name, line_num, errno_msg);
#if !defined(GUI_DEBUGGER)
    exit(1);
#endif
  }
  #define BP_MSG(msg) __bp(__FILE__, __func__, __LINE__, msg)
  #define BP() __bp(__FILE__, __func__, __LINE__, "")
  #define EBP() __ebp(__FILE__, __func__, __LINE__)
  #define ASSERT(cond) if (!(cond)) {BP();}
#else
  #define BP_MSG(msg)
  #define BP()
  #define EBP()
  #define ASSERT(cond)
#endif

typedef struct TimedBlock
{
  u32 counter;
  const char *file_name;
  const char *block_name;
  u32 line_num;
  u64 starting_ticks;
} TimedBlock;

typedef struct TimedRecord
{
  const char *file_name;
  const char *block_name;
  u32 line_number;
  r64 seconds;
} TimedRecord;

TimedRecord *global_timed_records;

INTERNAL void
close_timed_block(TimedBlock *timed_block)
{
  TimedRecord *timed_record = global_timed_records + timed_block->counter;
  timed_record->file_name = timed_block->file_name;
  timed_record->block_name = timed_block->block_name;
  timed_record->line_number = timed_block->line_num;
  timed_record->seconds = \
    (r64)(SDL_GetPerformanceCounter() - timed_block->starting_ticks) / (r64)SDL_GetPerformanceFrequency(); 
}

#if defined(GUI_INTERNAL)
#define TIMED_FUNCTION__(line_number) \
  TimedBlock timed_block##line_number __attribute__((__cleanup__(close_timed_block))) = { \
    .counter = __COUNTER__, \
    .file_name = __FILE__, \
    .block_name = __func__, \
    .line_num = line_number, \
    .starting_ticks = SDL_GetPerformanceCounter() \
  }; \

#define TIMED_FUNCTION_(line_number) \
  TIMED_FUNCTION__(line_number)

#define TIMED_FUNCTION() \
  TIMED_FUNCTION_(__LINE__)

#define TIMED_BLOCK_START(name) \
  u32 counter##name = __COUNTER__; \
  TimedBlock

#define TIMED_BLOCK_END(name) \
  TimedRecord record = records + counter##name; \

#else
#define TIMED_FUNCTION()
#define TIMED_BLOCK_START(name)
#define TIMED_BLOCK_END(name)
#endif

#if 0
struct DebugBlockStaticInfo
{
  const char *file_name;
  const char *block_name;
  u32 line_number;
};
extern DebugBlockStaticInfo global_debug_block_static_infos[];

enum DEBUG_EVENT_TYPE
{
  DEBUG_EVENT_BEGIN_BLOCK,
  DEBUG_EVENT_END_BLOCK,
  DEBUG_EVENT_FRAME_MARKER,
};
struct DebugEvent
{
  u64 clock;
  u16 static_info_index;
  DEBUG_EVENT_TYPE type;
};
struct DebugEventTable
{
  DebugEvent per_frame_events[][];
  u32 per_frame_event_index;
  u32 per_frame_event_count[];
  u32 frame_index;
};

// TODO(Ryan): Look in handmade_platform.h (day183) for macro definitions

// TIMED_FUNCTION() instead so don't have to provide name
#define TIMED_BLOCK__(line_number, ...) \
  TimedBlock timed_block##line_number(__COUNTER__, __FILE__, __LINE__, __func__, ## __VA_ARGS__)

#define TIMED_BLOCK_(line_number, ...) \
  TIMED_BLOCK__(line_number, ## __VA_ARGS__)

#define TIMED_BLOCK(...) \
  TIMED_BLOCK_(__LINE__, ## __VA_ARGS__)

struct TimedBlock
{
  u32 counter;

  TimedBlock(u32 counter_init, const char *file_name, u32 line_number, const char *block_name)
  {
    counter = counter_init;

    DebugBlockStaticInfo *block_static_info = global_debug_block_static_infos + counter;
    block_static_info->file_name = file_name;
    block_static_info->function_name = function_name;
    block_static_info->line_number = line_number;

    u32 event_index = global_debug_event_table->per_frame_event_index++;
    ASSERT(event_index < MAX_EVENT_DEBUG_COUNT);

    u32 frame_index = global_debug_event_table->frame_index++;
    DebugEvent *event = global_debug_event_table->per_frame_ + event_index;
    event->clock = __rdtsc();
    event->debug_record_index = (u16)counter;
    event->debug_record_array_index = 0;
    event->type = DEBUG_EVENT_BEGIN_BLOCK;

    if (frame_index >= MAX_DEBUG_FRAME_COUNT)
    {
      global_debug_event_table->frame_index = 0;
    }

    // at end we set event count etc.
  }

  ~TimedBlock()
  {
    debug_record->cycle_count += (__rdtsc() - start_cycle_count);

    // Logging approach below?
    // #define RECORD_DEBUG_EVENT(counter, event_type)
    u32 event_index = debug_event_index++;
    ASSERT(event_index < MAX_EVENT_DEBUG_COUNT);
    DebugEvent *event = debug_event_array + event_index;
    event->clock = __rdtsc();
    event->debug_record_index = (u16)counter;
    event->debug_record_array_index = 0;
    event->type = DEBUG_EVENT_END_BLOCK;
  }
};

#if defined(INTERNAL)
#define BEGIN_BLOCK_()
#define BEGIN_BLOCK()
#define END_BLOCK()
#else
#define TIMED_BLOCK()
#define TIMED_FUNCTION()
#define BEGIN_BLOCK()
#define END_BLOCK()
#endif

#define MAX_REGIONS_PER_FRAME 256
#define DEBUG_FRAME_COUNT 8
#define MAX_DEBUG_TRANSLATION_UNITS 2
#define MAX_DEBUG_EVENT_ARRAY_COUNT 64

#define MAX_EVENT_DEBUG_COUNT 65536
extern DebugEvent debug_event_array[MAX_EVENT_DEBUG_COUNT];

struct DebugFrameRegion
{
  r32 min_t, max_t;
};

struct DebugFrame
{
  u64 begin_clock, end_clock;
  u32 region_count;
  DebugRegion *regions;
};

// DebugHistory
struct DebugState
{
  b32 is_initialised, is_paused;

  r32 frame_bar_scale;
  DebugFrames *frames;

  // this is for tracking hierarchical information
  DebugBlockNode *blocks;
  // TODO(Ryan): Include layout/font information
};

struct DebugBlockNode
{
  u32 frame_index;
  DebugEvent *opening_event;
  DebugBlock *parent;
};
#endif
