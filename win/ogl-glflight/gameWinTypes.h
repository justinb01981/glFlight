#pragma once

#define _NOT_POSIX 1

#include <stdint.h>

typedef unsigned char u_int8_t;
typedef unsigned short u_int16_t;
typedef unsigned long u_int32_t;

#define strdup _strdup

#define strtok_r strtok_s
