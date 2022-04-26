// SPDX-License-Identifier: zlib-acknowledgement
#pragma once

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#define INTERNAL static
#define GLOBAL static
#define LOCAL_PERSIST static

typedef unsigned int uint;
typedef uint8_t u8;
typedef uint16_t  u16;
typedef uint32_t  u32;
typedef uint64_t  u64;
typedef int8_t s8;
typedef int16_t  s16;
typedef int32_t  s32;
typedef int64_t  s64;
typedef float r32;
typedef double r64;

#define R32_MAX FLT_MAX
#define R32_MIN -FLT_MAX
#define U32_MAX UINT32_MAX

#define KILOBYTES(x) \
  ((x) * 1024UL)
#define MEGABYTES(x) \
  (KILOBYTES(x) * 1024UL)
#define GIGABYTES(x) \
  (GIGABYTES(x) * 1024UL)
#define TERABYTES(x) \
  (TERABYTES(x) * 1024UL)
