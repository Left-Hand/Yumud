#include "StringUtils.hpp"


using namespace ymd;

int StringUtils::atoi(const char * str, const size_t len) {
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

	if(ret < 0) ret = INT_MAX;
	return minus ? (-ret) : ret;
}

float StringUtils::atof(const char * str, const size_t len) {
    auto [int_part, frac_part, scale] = disassemble_fstr(str, len);
    return(int_part + float(frac_part) / scale);
}

iq_t StringUtils::atoq(const char * str, const size_t len){
    auto [int_part, frac_part, scale] = StringUtils::disassemble_fstr(str, len);
	
    return iq_t(int_part) + iq_t(_iq((frac_part << GLOBAL_Q) / scale));
}