#pragma once

#include "stdint.h"
#include "string.h"

#include <concepts>
#include <tuple>

#include "sys/math/iq/iq_t.hpp"

namespace ymd::StringUtils {

void reverse_str(char * str,const size_t len);
::std::tuple<int,int,int> disassemble_fstr(const char * str, const size_t len);
int kmp_find(char *src, const size_t src_len, const char *match, const size_t match_len);
void str_replace(char *src, const size_t src_len, const char *match, const char *replace, const size_t dst_len);

bool is_numeric(const char * str, const size_t len);
bool is_digit(const char * str, const size_t len);
__fast_inline bool is_digit(const char chr){return chr >= '0' && chr <= '9';}
__fast_inline bool is_alpha(const char chr) {
    return (chr >= 'a' && chr <= 'z') || (chr >= 'A' && chr <= 'Z');
}

size_t itoa(int32_t value, char * str, uint8_t radix);
size_t iutoa(uint64_t value, char * str, uint8_t radix);
size_t iltoa(int64_t value, char * str, uint8_t radix);


size_t ftoa(float value, char * str, uint8_t eps);
size_t qtoa(const iq_t value, char * str, uint8_t eps);


int atoi(const char * str, const size_t len);
float atof(const char * str, const size_t len);
iq_t atoq(const char * str, const size_t len);

__inline int atoi(const char * str){return atoi(str, strlen(str));}
__inline float atof(const char * str){return atof(str, strlen(str));};
}