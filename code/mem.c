// SPDX-License-Identifier: zlib-acknowledgement
#include "mem.h"

INTERNAL MemoryArena
create_mem_arena(void *mem, u64 size)
{
  MemoryArena result = {0};

  result.base = (u8 *)mem;
  result.size = size;
  result.used = 0;

  return result;
}

INTERNAL void
reset_mem_arena(MemoryArena *arena)
{
  arena->used = 0;
}

INTERNAL void *
obtain_mem(MemoryArena *arena, u64 size)
{
  void *result = NULL;
  ASSERT(arena->used + size < arena->size);

  result = (u8 *)arena->base + arena->used;
  arena->used += size;

  return result;
}
