#ifndef __STRING_UTILS_H__

#define __STRING_UTILS_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "stdint.h"
#include "stdlib.h"
#include "string.h"

void ftoa(float value, char *str, uint8_t eps);

void itoa(int value, char *str, uint8_t radix);

int stoi(const char * str);

float stof(const char * str);

#ifdef __cplusplus
}
#endif

#endif