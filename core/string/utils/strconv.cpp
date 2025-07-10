#include "strconv.hpp"

using namespace ymd;

void strconv::reverse_str(char * str, size_t len){
	if(len == 0) return;

	len -= 1;
	for(size_t i = 0; i < len / 2 + (len % 2); i++){
		std::swap(str[i],str[len - i]);
	}

	str[len + 1] = '\0';
}

std::tuple<int, int, int> strconv::disassemble_fstr(const char * str, const size_t len){
	
    int int_part = 0;
	int frac_part = 0;
	int scale = 1;

	bool into_f = false;
    bool minus = false;

	size_t base = 0;

	for(;base < len; base++){
		char chr = str[base];

		switch(chr){
			case '\0':
				goto ret;
			case '.':
				into_f = true;
				break;
			case '-':
				minus = true;
				break;
			case '+':
			default:
				goto conv;
		}
	}

conv:
	for(;base < len; base++){
		char chr = str[base];

		switch(chr){
			case '\0':
				goto ret;
			case '.':
				into_f = true;
				break;
			case '-':
			case '+':
				break;
			default:
				if(is_digit(chr)){
					if(into_f){
						frac_part = frac_part * 10 + (chr - '0');
						scale *= 10;
					}else{
						int_part = int_part * 10 + (chr - '0');
					}
				}else{
					goto ret;
				}
				break;
		}
	}

	if(minus){
		int_part = -int_part;
		frac_part = -frac_part;
	}

ret:
	return {int_part, frac_part, scale};
}

bool strconv::is_numeric(const char* str, const size_t len) {
	bool hasDigit = false;
	bool hasDot = false;
	bool hasSign = false;

	for (size_t i = 0; i < len; i++) {
		char chr = str[i];
		if(chr == '\0'){
			break;
		} else if (is_digit(chr)) {
			hasDigit = true;
		} else if (chr == '.') {
			if (hasDot || !hasDigit) {
				return false; // ���С�������С����ǰû�����֣����� false
			}
			hasDot = true;
		} else if (chr == '+' || chr == '-') {
			if (hasSign || hasDigit || hasDot) {
				return false; // ��������Ż��������ų���������֮�󣬷��� false
			}
			hasSign = true;
		} else {
			return false; // �����֡������Ż�С���㣬���� false
		}
	}
	return hasDigit;
}

bool strconv::is_digit(const char * str, const size_t len){
    for(size_t i = 0; i < len; i++){
		char chr = str[i];
        if(!is_digit(chr)) return false;
		if(chr == '\0') break;
    }
    return true;
}
