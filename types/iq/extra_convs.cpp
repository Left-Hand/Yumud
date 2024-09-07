#include "extra_convs.hpp"

static constexpr size_t str_int_size = 16;
static constexpr size_t str_float_size = 16;


static void qtoa(char * str_int, char * str_frac, const _iq value, uint8_t eps = 3){
    eps = MIN(eps, 5);
    uint32_t abs_value = value > 0 ? value : -value;
    uint32_t int_part = abs_value >> GLOBAL_Q;
    uint32_t float_part = abs_value & ((1 << GLOBAL_Q )- 1);

    StringUtils::itoa(int_part,str_int,10);

    if(eps){
        int32_t scale = 1;

        for(uint8_t i = 0; i < eps; i++){
            scale *= 10;
        }

        float_part *= scale;
        float_part >>= GLOBAL_Q;
        StringUtils::itoas(float_part,str_frac, 10, eps);
    }

    str_frac[eps] = 0;
}


iq_t::iq_t(const char * str, const size_t len):value(0){
    int int_part = 0;
    int frac_part = 0;
    int scale = 1;

    StringUtils::disassemble_fstr(str, int_part, frac_part, scale);

    while(scale > 10000){
        frac_part /= 10;
        scale /= 10;
    }

    iq_t ret = iq_t(int_part) + iq_t(frac_part) / scale;
    *this = ret;
}

iq_t::operator String() const{
    return toString(3);
}

String toString(const iq_t iq, unsigned char eps){
    return iq.toString(MIN(eps, 4));
}

OutputStream & operator<<(OutputStream & os, const iq_t value){
    char str_int[str_int_size] = {0};
    char str_frac[str_float_size] = {0};
    qtoa(str_int, str_frac, value.value, os.eps());
    if(value < 0) os << '-';
    return os << str_int << '.' << str_frac;
}

String iq_t::toString(unsigned char eps) const{
    char str_int[str_int_size] = {0};
    char str_frac[str_float_size] = {0};
    qtoa(str_int, str_frac, value, eps);
    return String(str_int) + String(str_frac);
};