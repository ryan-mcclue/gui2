// SPDX-License-Identifier: zlib-acknowledgement
#include "types.h"
#include "debug.h"
#include "math.h"
#include "vector.h"
#include "platform.h"

#include "gui.h"

#include <ctype.h>
#include <stdio.h>

#define STBTT_STATIC
#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

INTERNAL void
debug_overlay(BackBuffer *back_buffer, MonospaceFont *font);

INTERNAL MemArena
create_mem_arena(void *mem, u64 size)
{
  MemArena result = {};

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

INTERNAL LoadedBitmap
create_empty_bitmap(MemArena *mem_arena, u32 width, u32 height, b32 want_to_clear = true)
{
  LoadedBitmap result = {};

  result.width = width;
  result.height = height;
  result.pixels = MEM_RESERVE_ARRAY(mem_arena, u32, width * height);

  u32 *pixel_cursor = (u32 *)result.pixels;
  if (want_to_clear)
  {
    for (u32 y = 0;
        y < height;
        ++y)
    {
      for (u32 x = 0;
          x < width;
          ++x)
      {
        *pixel_cursor++ = 0xff; 
      }
    }
  }

  return result;
}

INTERNAL void
draw_rect(BackBuffer *back_buffer, V2 origin, V2 x_axis, V2 y_axis, V4 colour)
{
  TIMED_BLOCK();

  u32 colour32 = round_r32_to_u32(colour.r * 255.0f) << 24 | 
                 round_r32_to_u32(colour.g * 255.0f) << 16 | 
                 round_r32_to_u32(colour.b * 255.0f) << 8 | 
                 round_r32_to_u32(colour.a * 255.0f);

  s32 max_width = (back_buffer->dim.w - 1);
  s32 max_height = (back_buffer->dim.h - 1);

  s32 x_min = max_width;
  s32 y_min = max_height;
  s32 x_max = 0;
  s32 y_max = 0;

  V2 points[4] = {origin, origin + x_axis, origin + x_axis + y_axis, origin + y_axis};
  for (u32 p_index = 0;
       p_index < 4;
       ++p_index)
  {
    V2 point = points[p_index];
    s32 floor_x = floor_r32_to_s32(point.x);
    s32 ceil_x = floor_r32_to_s32(point.x);
    s32 floor_y = floor_r32_to_s32(point.y);
    s32 ceil_y = floor_r32_to_s32(point.y);

    if (x_min > floor_x)
    {
      x_min = floor_x;
    }
    if (y_min > floor_y)
    {
      y_min = floor_y;
    }
    if (x_max < ceil_x)
    {
      x_max = ceil_x;
    }
    if (y_max < ceil_y)
    {
      y_max = ceil_y;
    }
  }

  if (x_min < 0)
  {
    x_min = 0;
  }
  if (y_min < 0)
  {
    y_min = 0;
  }
  if (x_max > max_width)
  {
    x_max = max_width;
  }
  if (y_max > max_height)
  {
    y_max = max_height;
  }


  for (s32 y = y_min;
       y < y_max;
       ++y)
  {
    for (s32 x = x_min;
        x < x_max;
        ++x)
    {
      V2 pixel_p = v2((r32)x, (r32)y);
      V2 d = pixel_p - origin;

      // subtract origin to have rooted at appropriate rect corners
      r32 top_edge = vec_dot(pixel_p - origin, -vec_perp(x_axis));
      r32 right_edge = vec_dot(pixel_p - (origin + x_axis), -vec_perp(y_axis));
      r32 bottom_edge = vec_dot(pixel_p - (origin + x_axis + y_axis), vec_perp(x_axis));
      r32 left_edge = vec_dot(pixel_p - (origin + y_axis), vec_perp(y_axis));

      if (top_edge < 0 && right_edge < 0 && bottom_edge < 0 && left_edge < 0)
      {
        u32 *pixel = back_buffer->pixels + (back_buffer->dim.w * y + x);
        *pixel = colour32;
      }

    }
  }

}

// TODO(Ryan): alignment values
INTERNAL void
draw_bitmap(BackBuffer *back_buffer, LoadedBitmap *bitmap, r32 scale, V2 origin, V4 colour)
{
  TIMED_BLOCK();

  V2 x_axis = scale * v2(bitmap->width, 0);
  V2 y_axis = scale * v2(0, bitmap->height);

  r32 inv_x_axis_squared = 1.0f / vec_length_sq(x_axis);
  r32 inv_y_axis_squared = 1.0f / vec_length_sq(y_axis);

  s32 max_width = (back_buffer->dim.w - 1);
  s32 max_height = (back_buffer->dim.h - 1);

  s32 x_min = max_width;
  s32 y_min = max_height;
  s32 x_max = 0;
  s32 y_max = 0;

  V2 points[4] = {origin, origin + x_axis, origin + x_axis + y_axis, origin + y_axis};
  for (u32 p_index = 0;
       p_index < 4;
       ++p_index)
  {
    V2 point = points[p_index];
    s32 floor_x = floor_r32_to_s32(point.x);
    s32 ceil_x = floor_r32_to_s32(point.x);
    s32 floor_y = floor_r32_to_s32(point.y);
    s32 ceil_y = floor_r32_to_s32(point.y);

    if (x_min > floor_x)
    {
      x_min = floor_x;
    }
    if (y_min > floor_y)
    {
      y_min = floor_y;
    }
    if (x_max < ceil_x)
    {
      x_max = ceil_x;
    }
    if (y_max < ceil_y)
    {
      y_max = ceil_y;
    }
  }

  if (x_min < 0)
  {
    x_min = 0;
  }
  if (y_min < 0)
  {
    y_min = 0;
  }
  if (x_max > max_width)
  {
    x_max = max_width;
  }
  if (y_max > max_height)
  {
    y_max = max_height;
  }


  for (s32 y = y_min;
       y < y_max;
       ++y)
  {
    for (s32 x = x_min;
        x < x_max;
        ++x)
    {
      V2 pixel_p = v2((r32)x, (r32)y);
      V2 d = pixel_p - origin;

      // subtract origin to have rooted at appropriate rect corners
      r32 top_edge = vec_dot(pixel_p - origin, -vec_perp(x_axis));
      r32 right_edge = vec_dot(pixel_p - (origin + x_axis), -vec_perp(y_axis));
      r32 bottom_edge = vec_dot(pixel_p - (origin + x_axis + y_axis), vec_perp(x_axis));
      r32 left_edge = vec_dot(pixel_p - (origin + y_axis), vec_perp(y_axis));

      if (top_edge < 0 && right_edge < 0 && bottom_edge < 0 && left_edge < 0)
      {
        u32 *pixel = back_buffer->pixels + (back_buffer->dim.w * y + x);

        r32 u = inv_x_axis_squared * vec_dot(d, x_axis);
        r32 v = inv_y_axis_squared * vec_dot(d, y_axis);
          
        // TODO(Ryan): Smoother with lerp (day 093)
        s32 texel_x = round_r32_to_s32(u * (r32)(bitmap->width - 1));
        s32 texel_y = v * (bitmap->height - 1);
        u32 *texel = (u32 *)bitmap->pixels + (bitmap->width * texel_y + texel_x);

        // TODO(Ryan): Gamma correction
        r32 alpha_blend_t = (*texel & 0xFF) / 255.0f;

        r32 red_orig = (*pixel >> 24);
        r32 new_red = (*texel >> 24) * colour.r;
        r32 red_blended = lerp(red_orig, new_red, alpha_blend_t);

        r32 green_orig = (*pixel >> 16 & 0xFF);
        r32 new_green = (*texel >> 16 & 0xFF) * colour.g;
        r32 green_blended = lerp(green_orig, new_green, alpha_blend_t);

        r32 blue_orig = (*pixel >> 8 & 0xFF);
        r32 new_blue = (*texel >> 8 & 0xFF) * colour.b;
        r32 blue_blended = lerp(blue_orig, new_blue, alpha_blend_t);

        u32 colour32 = round_r32_to_u32(red_blended) << 24 | 
                       round_r32_to_u32(green_blended) << 16 | 
                       round_r32_to_u32(blue_blended) << 8 | 
                       0xff;

        *pixel = colour32;
      }
    }
  }
}

INTERNAL MonospaceFont
load_monospace_font(MemArena *mem_arena, Functions *functions, const char *file_name, r32 pixel_height)
{
  MonospaceFont result = {};

  ReadFileResult font_file = functions->read_entire_file(file_name);
  if (font_file.mem != NULL)
  {
    stbtt_fontinfo font = {};
    stbtt_InitFont(&font, (u8 *)font_file.mem, 
                   stbtt_GetFontOffsetForIndex((u8 *)font_file.mem, 0));
    // hh is currently aligned to middle. we are at top
    // align_percentage_x = 0.0f;
    // align_percentage_y = descent / bitmap_height;
    r32 font_scale = stbtt_ScaleForPixelHeight(&font, pixel_height);
    //int font_ascent = 0;
    //stbtt_GetFontVMetrics(&font, &font_ascent, NULL, NULL);
    //int font_baseline = (int)(font_scale * font_ascent);

    for (u32 codepoint = PRINTABLE_FONT_GLYPH_START;
         codepoint != PRINTABLE_FONT_GLYPH_END;
         codepoint++)
    {
      //int x_advance = 0;
      //stbtt_GetCodepointHMetrics(&font, codepoint, &x_advance);
      //stbtt_GetCodepointKernAdvance()
      int width, height, offset_x, offset_y = 0;
      u8 *monochrome_bitmap = stbtt_GetCodepointBitmap(&font, 0.0f, 
          font_scale,
          codepoint, &width, &height, &offset_x, 
          &offset_y);

      LoadedBitmap ch_bitmap = create_empty_bitmap(mem_arena, width, height, false);

      u8 *bitmap_mem = monochrome_bitmap;
      u32 *dest_mem = (u32 *)ch_bitmap.pixels;
      for (s32 y = 0;
          y < height;
          y++)
      {
        for (s32 x = 0;
            x < width;
            x++)
        {
          u8 alpha = *bitmap_mem++;
          *dest_mem++ = (alpha << 24 | alpha << 16 | alpha << 8 | alpha);
        }
      }

      result.glyphs[codepoint] = ch_bitmap;

      stbtt_FreeBitmap(monochrome_bitmap, NULL);
    }

    functions->free_file_result(&font_file);
  }

  result.width = result.glyphs['A'].width;
  result.height = result.glyphs['|'].height;

  return result;
}

// global variables won't work for hotloading
GLOBAL r32 at_y = 0.0f;
GLOBAL r32 left_edge = 0.0f;

INTERNAL void
debug_reset(void)
{
  at_y = 0.0f;
}

INTERNAL void
draw_debug_text(BackBuffer *back_buffer, const char *text, MonospaceFont *font)
{
  r32 scale = 1.0f;

  r32 at_x = 0.0f;
  for (const char *ch_cursor = text; *ch_cursor != '\0'; ++ch_cursor)
  {
    if (*ch_cursor != ' ')
    {
      LoadedBitmap ch_bitmap = font->glyphs[toupper(*ch_cursor)];
      draw_bitmap(back_buffer, &ch_bitmap, scale, v2(left_edge + at_x, at_y), v4(1, 1, 1, 1));
    }

    at_x += (scale * font->width); 
  }

  at_y += (scale * font->height);
}

extern "C" void
update_and_render(BackBuffer *back_buffer, Input *input, Memory *memory, Functions *functions)
{

  State *state = (State *)memory->mem;
  if (!state->is_initialised)
  {
    u8 *start_mem = (u8 *)memory->mem + sizeof(State);
    u64 start_mem_size = memory->size - sizeof(State);
    state->mem_arena = create_mem_arena(start_mem, start_mem_size);

    state->font = load_monospace_font(&state->mem_arena, functions, 
                            "/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf",
                            24.0f);

    state->is_initialised = true;
  }

  state->time += input->update_dt;

  u32 *pixels = back_buffer->pixels;

  debug_reset();

  // could remove this clearing
  for (u32 y = 0;
       y < back_buffer->dim.h;
       ++y)
  {
    for (u32 x = 0;
        x < back_buffer->dim.w;
        ++x)
    {
      *pixels++ = 0xff0000ff;     
    }
  }

  V2 origin = v2(back_buffer->dim.w * 0.5f, back_buffer->dim.h * 0.5f);
  V2 x_axis = (50.0f + 50.0f * cosine(state->time)) * v2(cosine(state->time), sine(state->time));
  V2 y_axis = vec_perp(x_axis);

  draw_rect(back_buffer, origin, x_axis, y_axis, v4(1, 1, 1, 1));

  overlay_debug_records(back_buffer, &state->font);

}

__extension__ DebugRecord debug_records[__COUNTER__];

INTERNAL void
update_debug_records_snapshots(DebugState *debug_state)
{
  for (u32 debug_i = 0;
       debug_i < ARRAY_COUNT(debug_records);
       ++debug_i)
  {
    DebugRecord *src = debug_records + debug_i;
    if (src->hit_count > 0)
    {
      DebugCounterState *dst = &debug_state->counter_states[debug_state->counter_count]; 

      dst->file_name = src->file_name;
      dst->function_name = src->function_name;
      dst->line_number = src->line_number;
      dst->snapshots[debug_state->snapshot_index].cycle_count = src->cycle_count;
      dst->snapshots[debug_state->snapshot_index].hit_count = src->hit_count;

      debug_state->counter_count++;

      src->hit_count = 0;
      src->cycle_count = 0;
    }
  }
   
}


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
    
    // collate_debug_records(debug_state, current_event_array_index); below
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

INTERNAL void
collate_debug_records(DebugState *debug_state, u32 most_recent_event_index)
{
  debug_state->frames = PUSH_ARRAY(debug_state->arena, MAX_DEBUG_EVENT_ARRAY_COUNT * 4, DebugFrame);

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
         event_i < MAX_DEBUG_EVENT_COUNT;
         ++event_i)
    {
      DebugEvent *debug_event = global_debug_table->events[event_array_i] + event_i;

      DebugRecord *source = global_debug_table->records + debug_event->record_index;

      if (debug_event->type == DEBUG_EVENT_FRAME_MARKER)
      {
        if (current_frame != NULL)
        {
          current_frame->end_clock = event->clock;
        }

        current_frame = debug_state->frames + debug_state->frame_count;
        current_frame->begin_clock = event->clock;
        current_frame->end_clock = 0;
        current_frame->region_count = 0;
        // TODO(Ryan): reset memory arena to make it 'temporary'
        current_frame->regions = PUSH_ARRAY(debug_state->arena, MAX_DEBUG_RECORD_COUNT, 
                                                 DebugFrameRegion);
        debug_state->frame_count++;
      }
      else
      {
        if (current_frame != NULL)
        {
          u64 relative_clock = debug_event->clock - current_frame->begin_clock;

          if (debug_event->type == DEBUG_EVENT_FRAME_MARKER)
          {

          }
          else if (debug_event->type == DEBUG_EVENT_FRAME_MARKER)
          {

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

INTERNAL void
new_debug_overlay()
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
      r32 this_min_y = scale * debug_region->min_t;
    }
  }
}
