#include "extra_convs.hpp"

String iq_t::toString(const uint8_t eps) const{

    char str_int[8] = {0};
    char str_float[eps+1] = {0};

    char buf[10 + eps];

    uint32_t abs_value = abs(value);
    uint32_t int_part = abs_value >> GLOBAL_Q;
    uint32_t float_part = abs_value & ((1 << GLOBAL_Q )- 1);

	if(value < 0){
		str_int[0] = '-';
		StringUtils::itoa(int_part,str_int + 1,10);
	}
	else StringUtils::itoa(int_part,str_int,10);

    if(eps){
        int32_t scale = 1;
        for(uint8_t i = 0; i < eps; i++)
            scale *= 10;

        float_part *= scale;
        float_part >>= GLOBAL_Q;
        StringUtils::itoas(float_part,str_float, 10, eps);
    }

    int i = strlen(str_int);
    str_int[i] = '.';
    strcat(str_int,str_float);
    strcpy(buf,str_int);

    return String(buf);
};


iq_t::iq_t(const String & str){
    int int_part = 0;
    int frac_part = 0;
    int scale = 1;

    StringUtils::disassemble_fstr(str.c_str(), int_part, frac_part, scale);

    while(scale > 10000){
        frac_part /= 10;
        scale /= 10;
    }
    *this = int_part + iq_t(frac_part) / scale;
}

iq_t::operator String() const{
    return toString(3);
}

String toString(const iq_t & iq, const uint8_t & eps){
    return iq.toString(MIN(eps, 4));
}