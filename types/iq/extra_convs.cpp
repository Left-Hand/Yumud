#include "extra_convs.hpp"

String iq_t::toString(unsigned char eps) const{

    char str_int[16] = {0};
    char str_frac[8] = {0};

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
        StringUtils::itoas(float_part,str_frac, 10, eps);
    }

    // int i = 0;
    // for(;;i < int(strlen(str_int))){buf[i] = str_int[i];}
    // i++;
    // strcpy(buf,str_int);
    str_int[strlen(str_int)] = '.';
    // buf[strlen(buf)] = '.';
    str_frac[eps] = 0;
    // strcat(buf, str_frac);

    // for(int j = 0; j < eps; j++){buf[i + j] = str_frac[j];}

    return String(str_int) + String(str_frac);
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

String toString(const iq_t & iq, unsigned char eps){
    return iq.toString(MIN(eps, 4));
}