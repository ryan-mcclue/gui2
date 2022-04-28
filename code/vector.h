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
  return (b + a);
}

INTERNAL V2
operator+=(V2 a, V2 b)
{

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


