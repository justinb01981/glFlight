#pragma once

#include <stdint.h>

#if MSVC
typedef unsigned char u_int8_t;
typedef unsigned short u_int16_t;
typedef unsigned long u_int32_t;
#else
#endif

#define strdup _strdup

#define strtok_r strtok_s
