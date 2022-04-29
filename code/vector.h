// SPDX-License-Identifier: zlib-acknowledgement
#pragma once

union V2
{
  __extension__ struct
  {
    r32 x, y;
  };

  r32 e[2];
};

INTERNAL V2
v2(r32 x, r32 y)
{
  V2 result = {};

  result.x = x;
  result.y = y;

  return result;
}

INTERNAL V2
operator+(V2 a, V2 b)
{
  V2 result = {};

  result.x = a.x + b.x;
  result.y = a.y + b.y;

  return result;
}

INTERNAL V2
operator+(V2 a, r32 b)
{
  V2 result = {};

  result.x = a.x + b;
  result.y = a.y + b;

  return result;
}

INTERNAL V2
operator+(r32 a, V2 b)
{
  V2 result = {};

  result.x = a + b.x;
  result.y = a + b.y;

  return result;
}

INTERNAL V2 &
operator+=(V2 &a, V2 b)
{
  a = a + b;

  return a;
}

INTERNAL V2
operator-(V2 a, V2 b)
{
  V2 result = {};

  result.x = a.x - b.x;
  result.y = a.y - b.y;

  return result;
}

INTERNAL V2
operator-(V2 a, r32 b)
{
  V2 result = {};

  result.x = a.x - b;
  result.y = a.y - b;

  return result;
}

INTERNAL V2
operator-(r32 a, V2 b)
{
  V2 result = {};

  result.x = a - b.x;
  result.y = a - b.y;

  return result;
}

INTERNAL V2 &
operator-=(V2 &a, V2 b)
{
  a = a - b;

  return a;
}

INTERNAL V2
operator-(V2 a)
{
  V2 result = {};

  result.x = -a.x;
  result.y = -a.y;

  return result;
}

INTERNAL V2
operator*(V2 a, r32 b)
{
  V2 result = {};

  result.x = a.x * b;
  result.y = a.y * b;
  
  return result;
}

INTERNAL V2
operator*(r32 a, V2 b)
{
  V2 result = {};

  result.x = a * b.x;
  result.y = a * b.y;
  
  return result;
}

INTERNAL r32
vec_dot(V2 a, V2 b)
{
  r32 result = 0.0f;

  result = (a.x * b.x) + (a.y * b.y);

  return result;
}

INTERNAL V2
vec_perp(V2 a)
{
  V2 result = {};

  result.x = -a.y;
  result.y = a.x;

  return result;
}

INTERNAL r32
vec_length_sq(V2 a)
{
  r32 result = 0.0f;

  result = vec_dot(a, a); 

  return result;
}

union V2u
{
  __extension__ struct
  {
    u32 w, h;
  };

  u32 e[2];
};

union V3
{
  __extension__ struct 
  {
    r32 x, y, z;
  };

  __extension__ struct 
  {
    r32 r, g, b;
  };

  r32 e[3];
};

union V4
{
  __extension__ struct
  {
    union
    {
      V3 xyz;
      __extension__ struct
      {
        r32 x, y, z;
      };
    };
    r32 w;
  };

  __extension__ struct
  {
    union
    {
      V3 rgb;
      __extension__ struct
      {
        r32 r, g, b;
      };
    };
    r32 a;
  };

  r32 e[4];
};

INTERNAL V4
v4(r32 r, r32 g, r32 b, r32 a)
{
  V4 result = {};

  result.r = r;
  result.g = g;
  result.b = b;
  result.a = a;

  return result;
}
