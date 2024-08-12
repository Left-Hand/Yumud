#ifndef __STRING_UTILS_HPP__

#define __STRING_UTILS_HPP__

#include "stdint.h"
#include "stdlib.h"
#include "string.h"

#include <concepts>


class String;
class StringView;

namespace StringUtils {

void reverse_str(char * str, size_t len);

void disassemble_fstr(const char * str, int & int_part, int & frac_part, int & scale);
bool is_numeric(const char* str);
bool is_digit(const char * str);
bool is_digit(const char chr);

void ftoa(float value, char *str, uint8_t eps);

void itoa(int64_t value, char *str, uint8_t radix);
void itoas(int value, char *str, uint8_t radix, uint8_t size);
void iutoa(uint64_t value, char *str, uint8_t radix);

int kmp_find(const char *src, const size_t src_len, const char *match, const size_t match_len);
void str_replace(const char *src, const size_t src_len, const char *match, const char *replace, const size_t dst_len);

int atoi(const char * str, const size_t len);
float atof(const char * str, const size_t len);
__inline int atoi(const char * str){return atoi(str, strlen(str));}
__inline float atof(const char * str){return atof(str, strlen(str));};
}


template<typename T>
concept HasToString = requires(T t, unsigned char eps) {
    { t.toString(eps) } -> std::convertible_to<String>;
};
#endif