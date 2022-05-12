// SPDX-License-Identifier: zlib-acknowledgement
#pragma once

#include <float.h>
#include <math.h>

// TODO(Ryan): Investigate using gcc extensions for safer macros.
// Do they add any overhead?
#define MAX(x, y) \
  ((x) > (y) ? (x) : (y))
#define MIN(x, y) \
  ((x) < (y) ? (x) : (y))

#define ARRAY_COUNT(arr) \
  (sizeof(arr) / sizeof((arr)[0]))

// TODO(Ryan): Investigate replacing CRT with SIMD instructions

// TODO(Ryan): For some reason, #include <math.h> does not declare math functions?
r32 roundf(r32);
r32 ceilf(r32);
r32 floorf(r32);
r32 sqrtf(r32);
r32 cosf(r32);
r32 sinf(r32);

INTERNAL u32
round_r32_to_u32(r32 real32)
{
  u32 result = 0;

  result = (u32)roundf(real32);

  return result;
}

INTERNAL s32
round_r32_to_s32(r32 real32)
{
  s32 result = 0;

  result = (s32)roundf(real32);

  return result;
}

INTERNAL u32
floor_r32_to_u32(r32 real32)
{
  u32 result = 0;

  result = (u32)floorf(real32);

  return result;
}

INTERNAL s32
floor_r32_to_s32(r32 real32)
{
  s32 result = 0;

  result = (s32)floorf(real32);

  return result;
}

INTERNAL u32
ceil_r32_to_u32(r32 real32)
{
  u32 result = 0;

  result = ceilf(real32);

  return result;
}

INTERNAL s32
ceil_r32_to_s32(r32 real32)
{
  s32 result = 0;

  result = ceilf(real32);

  return result;
}

INTERNAL r32
square(r32 x)
{
  r32 result = 0.0f;

  result = x * x;

  return result;
}

INTERNAL r32
square_root(r32 val)
{
    r32 result = 0.0f;

    result = sqrtf(val);

    return result;
}

INTERNAL r32
lerp(r32 start, r32 end, r32 p)
{
  r32 result = 0.0f;

  result = ((end - start) * p) + start;

  return result;
}

INTERNAL r32
cosine(r32 angle)
{
  r32 result = 0.0f;

  result = cosf(angle);

  return result;
}

INTERNAL r32
sine(r32 angle)
{
  r32 result = 0.0f;

  result = sinf(angle);

  return result;
}

INTERNAL u32
xor_shift_u32(u32 *random_series)
{
	/* Algorithm "xor" from p. 4 of Marsaglia, "Xorshift RNGs" */
	u32 x = *random_series;
	x ^= x << 13;
	x ^= x >> 17;
	x ^= x << 5;
  *random_series = x;

	return x;
}
