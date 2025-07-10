#include "strconv.hpp"


using namespace ymd;

int strconv::atoi(const char * str, const size_t len) {
	int ret = 0;
	bool minus = false;

	for(size_t i = 0; i < len; i++){
		char chr = str[i];
	
		if(chr == '\0') break;

		if(chr == '-'){
			if(minus) break;
			minus = true;
		}

		if(isdigit(chr)){
			ret *= 10;
			ret += chr - '0';
		}
	}

	if(ret < 0) ret = INT32_MAX;
	return minus ? (-ret) : ret;
}

float strconv::atof(const char * str, const size_t len) {
    auto [int_part, frac_part, scale] = disassemble_fstr(str, len);
    return(int_part + float(frac_part) / scale);
}

