// SPDX-License-Identifier: zlib-acknowledgement

// it's ok to use == for floats if they are set to values, rather than computed (which could be variable based on precision)

// to avoid expensive copies, have a sort-buffer
typedef struct SortEntry
{
  // IMPORTANT(Ryan): For 32bit, we want to separate the values into high and low.
  // If r32, could multiply one value to place it in high area, e.g. 1024.0f * pos_z + pos_y;
  // For u32, just shift
  u32 key; 


  u32 entry_group_index; // if using u8*, this would be offset
} SortEntry;

// IMPORTANT(Ryan): The 'key' may become a series of values
// In this case, a simple comparison of keys cannot be done
// Introduce a function for this:
b32
is_in_front_of(SortEntry *a, SortEntry *b)
{
  
}

void
bubble_sort(SortEntry *sort_entries, u32 sort_entry_count)
{
  for (u32 sort_entry_i = 0;
       sort_entry_i < sort_entry_count;
       ++sort_entry_i)
  {
    SortEntry *sort_entry_outer = sort_entries + sort_entry_i;

    for (u32 inner_i = sort_entry_i;
         inner_i < sort_entry_count - 1;
         ++inner_i)
    {
      SortEntry *sort_entry_inner = sort_entries + inner_i;
      if (sort_entry_outer->key > sort_entry_inner->key)
      {
        SortEntry *temp = sort_entry_inner;
        *sort_entry_inner = *sort_entry_outer;
        *sort_entry_outer = *sort_entry_inner;
      }
    }

  }
}

typedef struct Entry
{
  ENTRY_TYPE type;
  union
  {
    EntrySomething something;
  };
} Entry;

typedef struct EntrySomething
{
  Entry *header;
  // type specific values ...
} EntrySomething;

typedef struct EntryGroup
{
  // More efficient to just use u8 *mem. This eliminates the need for a count.
  // (also can enforce better locality?)
  // We just advance pointer by type size.
  Entry entries[32];
  u32 entry_count;

  SortEntry sorting[32];
  u32 sorting_count;

} EntryGroup;

// 229, 230

// each function is specific to entry type
void
push_entry_type(EntryGroup *entry_group, u32 sort_key, ...)
{

}

// currently, this is back-to-front
void
handle_entry_group()
{

  for (u32 entry_i = 0;
       entry_i < entry_group->entry_count;
       ++entry_i)
  {
    Entry *entry = entry_group->entries + entry_count; 

    switch (entry->type)
    {
      case ENTRY_TYPE_SOMETHING:
      {
        EntrySomething *entry_something = (EntrySomething *)entry;
      } break;
      
      INVALID_DEFAULT_CASE
    }
  }
}
