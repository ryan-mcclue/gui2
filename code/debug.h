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
    fprintf(stderr, "ERRNO BREAKPOINT TRIGGERED! (%s:%s:%d)\n\"%s\"\n", file_name, 
            func_name, line_num, strerror(errno));
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
  u64 cycles;
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

  TimedBlock(u32 counter, const char *file_name, u32 line_number, const char *function_name, 
             u32 hit_count_increment = 1)
  {
    debug_record = debug_records + counter;
    debug_record->file_name = file_name;
    debug_record->function_name = function_name;
    debug_record->line_number = line_number;
    debug_record->cycles -= __rdtsc();
    debug_record->hit_count += hit_count_increment;
  }

  ~TimedBlock()
  {
    debug_record->cycles += __rdtsc();
  }
};

INTERNAL void
overlay_debug_records(void);
