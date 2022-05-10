// SPDX-License-Identifier: zlib-acknowledgement
#include <SDL2/SDL.h>

#include "types.h"
#include "debug.h"
//#include "math.h"
#include "vector.h"
#include "platform.h"

#include "gui.h"


INTERNAL MemArena
create_mem_arena(void *mem, u64 size)
{
  MemArena result = {0};

  result.base = (u8 *)mem;
  result.size = size;
  result.used = 0;

  return result;
}

#define MEM_RESERVE_STRUCT(arena, struct_name) \
  (struct_name *)(obtain_mem(arena, sizeof(struct_name)))

#define MEM_RESERVE_ARRAY(arena, elem, len) \
  (elem *)(obtain_mem(arena, sizeof(elem) * (len)))

INTERNAL void *
obtain_mem(MemArena *arena, u64 size)
{
  void *result = NULL;
  ASSERT(arena->used + size < arena->size);

  result = (u8 *)arena->base + arena->used;
  arena->used += size;

  return result;
}

INTERNAL SDL_Colour
v4_to_sdl_colour(V4 colour)
{
  SDL_Colour result = {0};

  result.r = round_r32_to_s32(255.0f * colour.r);
  result.g = round_r32_to_s32(255.0f * colour.g);
  result.b = round_r32_to_s32(255.0f * colour.b);
  result.a = round_r32_to_s32(255.0f * colour.a);

  return result;
}

INTERNAL void
draw_rect(SDL_Renderer *renderer, V2 pos, V2 dim, V4 colour)
{
  SDL_Colour sdl_colour = v4_to_sdl_colour(colour);

  SDL_SetRenderDrawColor(renderer, sdl_colour.r, sdl_colour.g, sdl_colour.b, sdl_colour.a);

  SDL_Rect render_rect = {0};
  render_rect.x = pos.x;
  render_rect.y = pos.y;
  render_rect.w = dim.w;
  render_rect.h = dim.h;
  SDL_RenderFillRect(renderer, &render_rect);
}

#if 0
INTERNAL void
draw_rect_on_axis(SDL_Renderer *renderer, V2 origin, V2 x_axis, V2 y_axis, V4 colour)
{
  SDL_Colour sdl2_colour = v4_to_sdl_colour(colour);

  SDL_Vertex vertices[4] = {0};
  vertices[0].position.x = origin.x;
  vertices[0].position.y = origin.y;
  vertices[0].colour = sdl2_colour;

  vertices[1].position.x = origin.x + x_axis.x;
  vertices[1].position.y = origin.y + x_axis.y;
  vertices[1].colour = sdl2_colour;

  vertices[2].position.x = origin.x + x_axis.x + y_axis.x;
  vertices[2].position.y = origin.y + x_axis.y + y_axis.y;
  vertices[2].colour = sdl2_colour;

  vertices[3].position.x = origin.x + y_axis.x;
  vertices[3].position.y = origin.y + y_axis.y;
  vertices[3].colour = sdl2_colour;

  SDL_RenderGeometry(renderer, NULL, vertices, 4, NULL, 0);
}
#endif

void
update_and_render(SDL_Renderer *renderer, Input *input, Memory *memory)
{
  // DEBUG_FRAME_MARKER();

  State *state = (State *)memory->mem;
  if (!state->is_initialised)
  {
    u8 *start_mem = (u8 *)memory->mem + sizeof(State);
    u64 start_mem_size = memory->size - sizeof(State);
    state->mem_arena = create_mem_arena(start_mem, start_mem_size);

    state->is_initialised = true;
  }

  state->time += input->update_dt;

  V2 pos = v2(100, 100);
  V2 dim = v2(300, 300);
  V4 colour = v4(1, 0, 1, 1);
  draw_rect(renderer, pos, dim, colour);

  // square_orbits();

  // debug_collate_and_present();
  // debug_reset(); called inside here
}

#if 0

__extension__ DebugRecord debug_records[__COUNTER__];


INTERNAL void
debug_overlay_snapshots(BackBuffer *back_buffer, MonospaceFont *font, DebugState *debug_state)
{
  for (u32 debug_counter = 0;
       debug_counter < debug_state->counter_count;
       ++debug_counter)
  {
    DebugCounterState counter_state = debug_state->counters[debug_counter];

    DebugStatistic hit_count_statistic = begin_debug_statistic();
    DebugStatistic cycle_count_statistic = begin_debug_statistic();
    DebugStatistic cycles_over_hits_statistic = begin_debug_statistic();
    for (u32 snapshot_i = 0;
         snapshot_i < DEBUG_SNAPSHOT_MAX_COUNT;
         ++snapshot_i)
    {
      DebugCounterSnapshot counter_snapshot = counter_state.snapshots[snapshot_i]; 
      update_debug_statistic(&hit_count_statistic, counter_snapshot.hit_count); 
      if (counter_snapshot.hit_count > 0)
      {
        update_debug_statistic(&cycles_over_hits_statistic, 
                               counter_snapshot.cycles / counter_snapshot.hit_count); 
      }
    }

    if (cycle_count_statistic.max > 0.0f)
    {
      for (u32 snapshot_i = 0;
          snapshot_i < DEBUG_SNAPSHOT_MAX_COUNT;
          ++snapshot_i)
      {
        DebugCounterSnapshot counter_snapshot = counter_state.snapshots[snapshot_i]; 

        r32 chart_height = debug_font->height * font_scale;
        r32 scale = 1.0f / counter_snapshot[snapshot_i].cycle_count;

        r32 this_height = scale * chart_height;
        r32 chart_min_y = at_y;

        r32 chart_left = 0.0f;

        V2 origin = v2(chart_left, chart_min_y);
        // have separate routine for drawing rotated
        draw_rect(back_buffer, origin, origin + v2(1, 0), origin + v2(0, this_height) , v4(scale, 1, 0, 1));
      }
    }

    if (hit_count_statistic.max)
    {
      char buf[256] = {};
      snprintf(buf, sizeof(buf), "%32s(%4d): %10ldcy | %4dh | %10ldcy/h", 
          counter_state.function_name, counter_state.line_number, 
          counter_snapshot.cycle_count, counter_snapshot.hit_count, 
          counter_snapshot.cycle_count / counter_snapshot.hit_count);
      draw_debug_text(back_buffer, buf, font);
    }

    V4 colors[] =
    {
      // primaries and secondaries
      {1, 0, 0, 0},
      {0, 1, 0, 0},
      {0, 0, 1, 0},
      {1, 1, 0, 0},
      {1, 0, 1, 0},
      {0, 1, 1, 0}
    };

    r32 prev_seconds = 0.0f;
    r32 prev_min_y;
    r32 chart_height = 300.0f;
    // We want the chart to be 33.3ms
    r32 scale = root_debug_info->total_seconds / 0.0333f; 
    for (u32 timer_i = 0;
         timer_i < root_debug_info->timer_count;
         ++timer_i)
    {
      V4 active_colour = colours[timer_i % ARRAY_COUNT(colours)];
      DebugTimer debug_timer = root_debug_info->timers[timer_i];
      r32 this_seconds = debug_timer.seconds - prev_seconds;
      prev_seconds = this_seconds;
      r32 this_scale = scale / this_seconds;

      r32 bar_height = chart_height * this_scale;

    }

  }
}

void
debug_frame_end(Memory *memory, PlatformDebugInfo *platform_debug_info)
{
  u32 event_count = debug_event_index; 
  debug_event_index = 0;   

  global_debug_table->current_event_array_index++;
  // IMPORTANT(Ryan): The size of the array determines how many frames of data we collect 
  // Now, we don't need the snapshots
  if (global_debug_table->current_event_array_index >= MAX_DEBUG_EVENT_COUNT)
  {
    global_debug_table->current_event_array_index = 0;
  }

  global_debug_table->frame_event_count[current_event_array_index] = global_event_count;

  DebugState *debug_state = (DebugState *)memory->debug_memory;
  // allows setting to 0 for debug builds
  if (debug_state != NULL)
  {
    if (!debug_state->is_initialised)
    {
      debug_state->is_initialised = true;
    }
    
    begin_temporary_memory(debug_state->arena);

    debug_state->first_free_block = NULL;
    // collate_debug_records(debug_state, current_event_array_index);
    end_temporary_memory(debug_state->arena);
    debug_state->counter_count = 0;

#if 0
    update_debug_records(debug_state);
    update_platform_debug_records();
#else
    // collate_debug_records() below
    for (u32 counter_i = 0;
         counter_i < debug_state->counter_count;
         ++counter_i)
    {
      DebugCounterState *counter_states = debug_state->counter_states + counter_i;
      counter_states[debug_state->snapshot_index].hit_count = 0;
      counter_states[debug_state->snapshot_index].cycle_count = 0;
    }
    
    for (u32 event_i = 0;
         event_i < event_count;
         event_i++)
    {
      DebugEvent *event = debug_events + event_count;

      DebugCounterState *dst = debug_state->counter_states + event->debug_record_index;
      DebugRecord *src = debug_records + event->debug_record_index; 

      dst->file_name = src->file_name;
      dst->function_name = src->function_name;
      dst->line_number = src->line_number;

      if (event->type == DEBUG_EVENT_BEGIN_BLOCK)
      {
        dst->snapshots[debug_state->snapshot_index].hit_count++;
        dst->snapshots[debug_state->snapshot_index].cycle_count -= event->clocks; 
      }
      else
      {
        dst->snapshots[debug_state->snapshot_index].cycle_count += event->clocks; 
      }
    }
#endif
    // IMPORTANT: just updating that is done in platform

    debug_state->snapshot_index++;
    if (debug_state->snapshot_index >= DEBUG_SNAPSHOT_MAX_COUNT)
    {
      debug_state->snapshot_index = 0;
    }
  }
}

INTERNAL DebugFrameRegion *
add_region(DebugState *debug_state, DebugFrame *current_frame)
{
  ASSERT(current_frame->region_count < MAX_DEBUG_REGIONS_PER_FRAME);
  DebugFrameRegion *result = current_frame->regions + current_frame->region_count;

  return result;
}

INTERNAL void
collate_debug_records(DebugState *debug_state, u32 most_recent_event_index)
{
  debug_state->frames = PUSH_ARRAY(debug_state->arena, MAX_DEBUG_EVENT_ARRAY_COUNT * 4, DebugFrame);
  debug_state->frame_bar_scale = 1.0f;

  DebugFrame *current_frame = NULL;

  // IMPORTANT(Ryan): Visit in temporal order, i.e. oldest to most recent
  for (u32 event_array_i = most_recent_event_index + 1;
       event_array_i != most_recent_event_index;
       ++event_array_i)
  {
    if (event_array_i == MAX_DEBUG_EVENT_ARRAY_COUNT)
    {
      event_array_i = 0;
    }

    for (u32 event_i = 0;
         event_i < global_debug_table->event_count[event_array_i];
         ++event_i)
    {
      DebugEvent *debug_event = global_debug_table->events[event_array_i] + event_i;

      DebugRecord *source = global_debug_table->records + debug_event->record_index;

      if (debug_event->type == DEBUG_EVENT_FRAME_MARKER)
      {
        if (current_frame != NULL)
        {
          current_frame->end_clock = event->clock;
          r32 clock_range = (r32)(current_frame->end_clock - current_frame->begin_clock);
          if (clock_range > 0.0f)
          {
            r32 frame_bar_scale = 1.0f / clock_range;
            // store largest clock_range value 
            if (debug_state->frame_bar_scale > frame_bar_scale)
            {
              debug_state->frame_bar_scale = frame_bar_scale;
            }
          }
        }

        current_frame = debug_state->frames + debug_state->frame_count;
        current_frame->begin_clock = event->clock;
        current_frame->end_clock = 0;
        current_frame->region_count = 0;
        // TODO(Ryan): reset memory arena to make it 'temporary'
        current_frame->regions = PUSH_ARRAY(debug_state->arena, MAX_REGIONS_PER_FRAME, 
                                                 DebugFrameRegion);
        debug_state->frame_count++;
      }
      else
      {
        if (current_frame != NULL)
        {
          OpenDebugBlock *block = get_debug_block(debug_state);

          u64 relative_clock = debug_event->clock - current_frame->begin_clock;

          if (debug_event->type == DEBUG_EVENT_BEGIN_BLOCK)
          {
            OpenDebugBlock *debug_block = debug_state->first_free_block;
            if (debug_block != NULL)
            {
              debug_block = debug_block->next_free; 
            }
            else
            {
              debug_block = PUSH_STRUCT(debug_state->arena, OpenDebugBlock); 
            }

            debug_block->opening_event = debug_event; 
            debug_block->parent = debug_state->first_free_block;
            debug_state->first_free_block = debug_block;
          }
          else if (debug_event->type == DEBUG_EVENT_END_BLOCK)
          {
            // TODO(Ryan): Want a tree structure to store hierarchy
            for (u32 past_block_i = num_blocks - 1;
                 past_block_i >= 0;
                 past_block--)
            {
              DebugEvent *opening_event = debug_state->blocks[past_block_i]->event;
              if (opening_event->type == DEBUG_EVENT_OPEN_BLOCK)
              {
                // matching open block found 
                if (opening_eventing_debug_block->parent == NULL)
                {
                  // we draw from regions
                  r32 min_t = (r32)(opening_event->clock - current_frame->begin_clock);
                  r32 max_t = (r32)(debug_event->clock - current_frame->begin_clock);
                  r32 threshold = 0.01f;
                  // only draw if visible, i.e. more than 1 100th of a bar
                  if (max_t - min_t > threshold_t)
                  {
                    DebugFrameRegion *region = add_region(debug_state, current_frame); 
                    region->min_t = min_t;
                    region->max_t = max_t;
                  }
                }
              }
            }
                 
          }
          else
          {
            ASSERT(!"Invalid event type");
          }
        }
      }

    }
  }
}

// only the drawing part gets input?
INTERNAL void
new_debug_overlay(Input *input)
{
  r32 scale = debug_state->frame_bar_scale;
  for (u32 frame_index = 0;
       frame_index < debug_state->frame_count;
       ++frame_index)
  {
    DebugFrame *debug_frame = debug_state->frames + frame_index;
    for (u32 region_index = 0;
        region_index < debug_state->region_count;
        ++region_index)
    {
      DebugRegion *debug_region = debug_frame->regions + region_index;
      r32 this_min_y = some_y + scale * debug_region->min_t * chart_height;
      r32 this_max_y = some_y + scale * debug_region->max_t * chart_height;

      if (mouse_inside_rectangle())
      {
        draw_debug_text();
      }
    }
  }
}

#endif
