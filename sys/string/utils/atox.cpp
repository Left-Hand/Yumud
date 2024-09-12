#include "../string_utils.hpp"

int StringUtils::atoi(const char * str, const size_t len) {
	int ret = 0;
	bool minus = false;

	for(size_t i = 0; i < len; i++){
		char chr = str[i];
	
		if(chr=='-'){
			minus = true;
		}

		while(chr>='0' and chr<='9'){
			ret *= 10;
			ret += chr - '0';
			if(ret < 0){
				ret= __INT_MAX__;
				break;
			}
		}
	}

	return minus ? (-ret) : ret;
}

float StringUtils::atof(const char * str, const size_t len) {
    auto [int_part, frac_part, scale] = disassemble_fstr(str, len);
    return(int_part + (float)frac_part / scale);
}

iq_t StringUtils::atoq(const char * str, const size_t len){
    auto [int_part, frac_part, scale] = StringUtils::disassemble_fstr(str, len);

    while(scale > 10000){
        frac_part /= 10;
        scale /= 10;
    }

    return iq_t(int_part) + iq_t(frac_part) / scale;
}