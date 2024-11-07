#include "StringUtils.hpp"

using namespace yumud;

static inline void swap_char(char & a, char & b){
	char temp = a;
	a = b;
	b = temp;
}

void StringUtils::reverse_str(char * str, size_t len){
	if(len == 0) return;

	len -= 1;
	for(size_t i = 0; i < len / 2 + (len % 2); i++){
		swap_char(str[i],str[len - i]);
	}

	str[len + 1] = '\0';
}

std::tuple<int, int, int> StringUtils::disassemble_fstr(const char * str, const size_t len){
	
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

bool StringUtils::is_numeric(const char* str, const size_t len) {
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

bool StringUtils::is_digit(const char * str, const size_t len){
    for(size_t i = 0; i < len; i++){
		char chr = str[i];
        if(!is_digit(chr)) return false;
		if(chr == '\0') break;
    }
    return true;
}
int StringUtils::kmp_find(char *src, const size_t src_len, const char *match, const size_t match_len) {
	size_t *table = (size_t *)malloc(match_len * sizeof(size_t));
	size_t i = 0, j = 1;
	table[0] = 0;
	while (j < match_len) {
		if (match[i] == match[j]) {
			table[j] = i + 1;
			i++; j++;
		} else {
			if (i != 0) {
				i = table[i - 1];
			} else {
				table[j] = 0;
				j++;
			}
		}
	}
	
	i = 0;
	j = 0;
	while (i < src_len && j < match_len) {
		if (src[i] == match[j]) {
			i++; j++;
		} else {
			if (j != 0) {
				j = table[j - 1];
			} else {
				i++;
			}
		}
	}
	
	free(table);
	
	if (j == match_len) {
		return i - j;
	} else {
		return -1;
	}
}

void StringUtils::str_replace(char *src, const size_t src_len, const char *match, const char *replace, const size_t dst_len){
	char * find_ptr = src;
	size_t find_len = src_len;
	const char * replace_ptr = match;
	
	while(1){
		int pos = kmp_find(find_ptr, find_len, replace_ptr, dst_len);
		if(pos < 0) break;
		else{
			find_ptr += pos;
			for(size_t i = 0; i < dst_len; i++){
				find_ptr[i] = replace[i];
			}
			find_ptr += dst_len;
		}
	}
}




[[maybe_unused]] static void mystof(const char * str, const size_t len, int & int_part, int & frac_part, int & scale){
    scale = 1;
    bool minus = false;

	const size_t default_begin_index = len; 
	const size_t default_point_index = len;
	size_t begin_index = default_begin_index;
	size_t point_index = default_point_index;

	for(size_t i = 0; i < len; i++){
		char chr = str[i];
		
		if(begin_index == default_begin_index){
			if(StringUtils::is_digit(chr)){
				begin_index = i;
			}
		}
	
		if(chr=='\0'){
			int_part = 0;
			frac_part = 0;
			return;
		}

		if(chr == '.'){
			point_index = i + 1;
            break;
		}

		if(chr=='-'){
			minus = true;
		}
	}

	if(point_index < len){
		frac_part = StringUtils::atoi(str + point_index , len - (point_index));
	}

	int_part = StringUtils::atoi(str + begin_index, len - (point_index + 1));


	for(size_t i = 0; i < len - point_index; i++){
		scale *= 10;
	}

	if(minus){
		int_part = -int_part;
		frac_part = -frac_part;
	}
}
