// SPDX-License-Identifier: zlib-acknowledgement
#pragma once

union V2
{
  __extension__ struct
  {
    r32 x, y;
  };

  __extension__ struct
  {
    r32 w, h;
  };

  r32 e[2];
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


