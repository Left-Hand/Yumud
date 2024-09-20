#ifndef __STRING_UTILS_HPP__

#define __STRING_UTILS_HPP__

#include "stdint.h"
#include "string.h"

#include <concepts>
#include <tuple>

#include "sys/math/iq/iqt.hpp"


class String;
class StringView;

namespace StringUtils {

void reverse_str(char * str,const size_t len);
std::tuple<int,int,int> disassemble_fstr(const char * str, const size_t len);
int kmp_find(char *src, const size_t src_len, const char *match, const size_t match_len);
void str_replace(char *src, const size_t src_len, const char *match, const char *replace, const size_t dst_len);

bool is_numeric(const char * str, const size_t len);
bool is_digit(const char * str, const size_t len);
bool is_digit(const char chr);

void itoa(int64_t value, char * str, uint8_t radix);
void iutoa(uint64_t value, char * str, uint8_t radix);
void itoas(int value, char * str, uint8_t radix, uint8_t size);


void ftoa(float value, char * str, uint8_t eps);
void qtoa(const iq_t value, char * str, uint8_t eps);


int atoi(const char * str, const size_t len);
float atof(const char * str, const size_t len);
iq_t atoq(const char * str, const size_t len);

__inline int atoi(const char * str){return atoi(str, strlen(str));}
__inline float atof(const char * str){return atof(str, strlen(str));};
}


template<typename T>
concept HasToString = requires(T t, unsigned char eps) {
    { t.toString(eps) } -> std::convertible_to<String>;
};
#endif