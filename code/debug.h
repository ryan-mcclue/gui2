// SPDX-License-Identifier: zlib-acknowledgement
#pragma once

#include <x86intrin.h>

#if defined(GUI_INTERNAL)
  INTERNAL void __bp(char const *file_name, char const *func_name, int line_num,
                     char const *optional_message = "")
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
  #define BP() __bp(__FILE__, __func__, __LINE__)
  #define EBP() __ebp(__FILE__, __func__, __LINE__)
  #define ASSERT(cond) if (!(cond)) {BP();}
#else
  #define BP_MSG(msg)
  #define BP()
  #define EBP()
  #define ASSERT(cond)
#endif

struct DebugRecord
{
  const char *file_name;
  const char *function_name;
  u32 line_number;
  u64 cycle_count;
  u32 hit_count;
};

extern DebugRecord debug_records[];

#define TIMED_BLOCK__(line_number, ...) \
  TimedBlock timed_block##line_number(__COUNTER__, __FILE__, __LINE__, __func__, ## __VA_ARGS__)

#define TIMED_BLOCK_(line_number, ...) \
  TIMED_BLOCK__(line_number, ## __VA_ARGS__)

#define TIMED_BLOCK(...) \
  TIMED_BLOCK_(__LINE__, ## __VA_ARGS__)

struct TimedBlock
{
  DebugRecord *debug_record;
  u64 start_cycle_count;
  u32 counter;

  TimedBlock(u32 counter_init, const char *file_name, u32 line_number, const char *function_name, 
             u32 hit_count_increment = 1)
  {
    start_cycle_count = __rdtsc();

    counter = counter_init;

    debug_record = debug_records + counter;
    debug_record->file_name = file_name;
    debug_record->function_name = function_name;
    debug_record->line_number = line_number;
    debug_record->hit_count += hit_count_increment;

    // Logging approach below?
    // so now we record everything that happened that frame
    u32 event_index = debug_event_index++;
    ASSERT(event_index < MAX_EVENT_DEBUG_COUNT);
    DebugEvent *event = debug_event_array + event_index;
    event->clock = __rdtsc();
    event->debug_record_index = (u16)counter;
    event->debug_record_array_index = 0;
    event->type = DEBUG_EVENT_BEGIN_BLOCK;
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


enum DEBUG_EVENT_TYPE
{
  DEBUG_EVENT_BEGIN_BLOCK,
  DEBUG_EVENT_END_BLOCK
};
// this is for logging, so size is of more concern?
struct DebugEvent
{
  u64 clock;
  // u16 core_index;
  // u16 thread_index;
  u16 debug_record_index;
  // TODO(Ryan): This is only required for HH multiple builds?...
  u16 debug_record_array_index;
  u8 type;
};
// IMPORTANT(Ryan): If doing multithreading, probably have some double-buffering scheme,
// whereby one buffer is where writing to and then other is for reading
#define MAX_EVENT_DEBUG_COUNT 65536
extern DebugEvent debug_event_array[MAX_EVENT_DEBUG_COUNT];
extern u32 debug_event_index; // where we are trying to write to

struct DebugTimer
{
  const char *name;
  r32 seconds_snapshots[DEBUG_SNAPSHOT_MAX_COUNT];
};

// could change this to RootDebugInfo 
struct PlatformDebugInfo
{
  u32 timer_count;
  r32 total_seconds;
  DebugTimer debug_timers[64];
};

struct DebugCounterSnapshot
{
  u64 cycle_count;
  u32 hit_count;
};

#define DEBUG_SNAPSHOT_MAX_COUNT 120
struct DebugCounterState
{
  const char *file_name;
  const char *function_name;
  u32 line_number;

  DebugCounterSnapshot snapshots[DEBUG_SNAPSHOT_MAX_COUNT];
};

// DebugHistory
struct DebugState
{
  u32 counter_count;
  u32 snapshot_index;
  DebugCounterState counter_states[512];

  // PlatformDebugTimers platform_timers[DEBUG_SNAPSHOT_MAX_COUNT];

  // TODO(Ryan): Include layout/font information
};

struct DebugStatistic
{
  r64 min, max, avg;
  u32 count; 
};
INTERNAL DebugStatistic
begin_debug_statistic(void)
{
  DebugStatistic result = {};
  
  result.min = R32_MAX;
  result.max = R32_MIN;
  result.avg = 0.0f;

  return result;
}
INTERNAL void
end_debug_statistic(DebugStatistic *debug_statistic)
{
  if (debug_statistic->count != 0)
  {
    debug_statistic->avg /= debug_statistic->count;
  }
  else
  {
    debug_statistic->min = debug_statistic->max = 0.0f;
  }
}
INTERNAL void
update_debug_statistic(DebugStatistic *debug_statistic, r64 value)
{
  debug_statistic->count++;

  if (value < debug_statistic->min)
  {
    debug_statistic->min = value;
  }
  if (value > debug_statistic->max)
  {
    debug_statistic->max = value;
  }
  debug_statistic->avg += value;
}


// DebugInformation
#define MAX_DEBUG_TRANSLATION_UNITS 2
struct DebugTable
{
  DebugEvent debug_events[MAX_DEBUG_EVENT_COUNT];
  DebugRecords debug_records[MAX_DEBUG_TRANSLATION_UNITS][MAX_DEBUG_RECORD_COUNT];
};

// defined in platform?
extern GLOBAL DebugTable global_debug_table;

// end of timer usage
#define DEBUG_RECORDS_COUNT __COUNTER__
