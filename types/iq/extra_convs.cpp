#include "extra_convs.hpp"

String iq_t::toString(const uint8_t eps) const{
    using namespace StringUtils;

    char str_int[8] = {0};
    char str_float[eps+1] = {0};

    char buf[10 + eps];

    int32_t int_part = value >> GLOBAL_Q;
    int32_t float_part = value & ((1 << GLOBAL_Q )- 1);

	if(value < 0 && int_part == 0){
		str_int[0] = '-';
		itoa(int_part,str_int + 1,10);
	}
	else itoa(int_part,str_int,10);

    if(eps){
        int32_t scale = 1;
        for(uint8_t i = 0; i < eps; i++)
            scale *= 10;

        float_part *= scale;
        float_part >>= GLOBAL_Q;
        itoas(float_part,str_float, 10, eps);
    }

    int i = strlen(str_int);
    str_int[i] = '.';
    strcat(str_int,str_float);
    strcpy(buf,str_int);

    return String(buf);
};