#ifndef __STRING_UTILS_H__

#define __STRING_UTILS_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "stdint.h"
#include "stdlib.h"
#include "string.h"

void reverse_str(char * str, size_t len);

void ftoa(float value, char *str, uint8_t eps);

void itoa(int64_t value, char *str, uint8_t radix);
void itoas(int value, char *str, uint8_t radix, uint8_t size);
void iutoa(uint64_t value, char *str, uint8_t radix);

int kmp_find(const char *src, const size_t src_len, const char *match, const size_t match_len);
void str_replace(const char *src, const size_t src_len, const char *match, const char *replace, const size_t dst_len);

int stoi(const char * str);

float stof(const char * str);

#ifdef __cplusplus
}
#endif

#endif