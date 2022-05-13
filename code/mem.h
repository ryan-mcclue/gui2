// SPDX-License-Identifier: zlib-acknowledgement

typedef struct MemoryArena
{
  u8 *base;
  u64 size;
  u64 used;
} MemoryArena;

#define MEM_PUSH_STRUCT(arena, struct_name) \
  (struct_name *)(obtain_mem(arena, sizeof(struct_name)))

#define MEM_PUSH_ARRAY(arena, elem, len) \
  (elem *)(obtain_mem(arena, sizeof(elem) * (len)))

