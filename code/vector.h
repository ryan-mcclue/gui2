// SPDX-License-Identifier: zlib-acknowledgement
#pragma once

#define VECTOR_SIZE(amount, type) \
  vector_size(amount * sizeof(type))

typedef r32 __v2 __attribute__((VECTOR_SIZE(2, r32)));
typedef union V2
{
  __v2 vec;
  struct
  {
    r32 x, y;
  };
  struct
  {
    r32 a, b;
  };
  struct
  {
    r32 w, h;
  };
  r32 e[2];
} V2;

INTERNAL V2
v2(r32 x, r32 y)
{
  V2 result = {0};

  result.x = x;
  result.y = y;

  return result;
}

INTERNAL r32
v2_dot(V2 vec1, V2 vec2)
{
  r32 result = 0.0f;

  result = (vec1.x * vec2.x) + (vec1.y * vec2.y);

  return result;
}

INTERNAL r32
v2_length_sq(V2 vec)
{
  r32 result = 0.0f;

  result = v2_dot(vec, vec);

  return result;
}

INTERNAL r32
v2_length(V2 vec)
{
  r32 result = 0.0f;

  result = square_root(v2_length_sq(vec));

  return result;
}

INTERNAL V2
v2_norm(V2 vec)
{
  V2 result = {vec.vec * (1.0f / v2_length(vec))};

  return result;
}

INTERNAL V2
v2_perp(V2 vec)
{
  V2 result = {0};

  result.x = -vec.y;
  result.y = vec.x;

  return result;
}

INTERNAL V2
v2_arm(r32 angle)
{
  V2 result = {0};

  result.x = cosine(angle);
  result.y = sine(angle);

  return result;
}

INTERNAL V2
v2_centered(V2 centre, V2 dim)
{
  V2 result = {0};
  
  result.x = centre.x - 0.5f * dim.x;
  result.y = centre.y + 0.5f * dim.y;

  return result;
}

typedef u32 v2u __attribute__((VECTOR_SIZE(2, u32)));
typedef union V2u
{
  v2u vec;
  struct
  {
    u32 x, y;
  };
  struct
  {
    u32 a, b;
  };
  struct
  {
    u32 w, h;
  };
  u32 e[2];
} V2u;

typedef s32 v2s __attribute__((VECTOR_SIZE(2, s32)));
typedef union V2s
{
  v2s vec;
  struct
  {
    s32 x, y;
  };
  struct
  {
    s32 a, b;
  };
  struct
  {
    s32 w, h;
  };
  s32 e[2];
} V2s;

typedef r32 __v4 __attribute__((VECTOR_SIZE(4, r32)));
typedef union V4
{
  __v4 vec;
  struct
  {
    r32 x, y, z, w;
  };
  struct
  {
    r32 r, g, b, a;
  };
  r32 e[4];
} V4;
INTERNAL V4
v4(r32 x, r32 y, r32 z, r32 w)
{
  V4 result = {0};

  result.x = x;
  result.y = y;
  result.z = z;
  result.w = w;

  return result;
}

INTERNAL V4
v4_hadamard(V4 vec1, V4 vec2)
{
  V4 result = {0};

  result.x = vec1.x * vec2.x;
  result.y = vec1.y * vec2.y;
  result.z = vec1.z * vec2.z;
  result.w = vec1.w * vec2.w;

  return result;
}
typedef V4 V3;

typedef u32 v4u __attribute__((VECTOR_SIZE(4, u32)));
typedef union V4u
{
  v4u vec;
  struct
  {
    u32 x, y, z, w;
  };
  struct
  {
    u32 r, g, b, a;
  };
  u32 e[4];
} V4u;
typedef V4u V3u;

typedef s32 v4s __attribute__((VECTOR_SIZE(4, s32)));
typedef union V4s
{
  v4s vec;
  struct
  {
    s32 x, y, z, w;
  };
  struct
  {
    s32 r, g, b, a;
  };
  s32 e[4];
} V4s;
typedef V4s V3s;
