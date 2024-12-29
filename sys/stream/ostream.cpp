#include "ostream.hpp"
#include "sys/string/String.hpp"
#include "sys/string/StringView.hpp"


using namespace ymd;

OutputStream& OutputStream::operator<<(std::ios_base& (*func)(std::ios_base&)){
    do{
        if (func == &std::oct) {setRadix(8);break;}
        if (func == &std::dec) {setRadix(10);break;}
        if (func == &std::hex) {setRadix(16);break;}
        if (func == &std::fixed) {
            //TODO
            break;
        }
        
        if (func == &std::scientific) {
            //TODO
            break;
        }
        
        if (func == &std::boolalpha){
            b_boolalpha = true;
            break;
        }

        if (func == &std::noboolalpha){
            b_boolalpha = false;
            break;
        }

        if (func == &std::showpos){
            b_showpos = true;
            break;
        }
        
        if (func == &std::noshowpos){
            b_showpos = false;
            break;
        }

        if (func == &std::showbase){
            b_showbase = true;
            break;
        }
        
        if (func == &std::noshowbase){
            b_showbase = false;
            break;
        }

        // if (func == &std::flush){
        //     this->flush();
        //     break;
        // }

        // if (func == &std::endl){
        //     this->write();
        //     break;
        // }
    }while(false);

    skip_split = true;
    return *this;
}


// template<typename T>
// static __fast_inline void print_numeric_impl(OutputStream & os, T value, auto convfunc){
//     char str[12] = {0};
//     convfunc(value, str, this->eps());
//     if(b_showpos and value >= 0) *this << '+';
//     return *this << str;
// }

#define PRINT_FLOAT_TEMPLATE(type, convfunc)\
    char str[12] = {0};\
    const auto len = convfunc(value, str, this->eps());\
    if(b_showpos and value >= 0) *this << '+';\
    this->write(str, len);\

OutputStream & OutputStream::operator<<(const iq_t value){
    PRINT_FLOAT_TEMPLATE(iq_t, StringUtils::qtoa);
    return *this;
}

OutputStream & OutputStream::operator<<(const float value){
    PRINT_FLOAT_TEMPLATE(iq_t, StringUtils::ftoa);
    return *this;
}

#define PRINT_INT_TEMPLATE(blen, convfunc)\
    if(b_showpos and val >= 0) *this << '+';\
    if(b_showbase and (radix() != 10)){*this << get_basealpha(radix());}\
    char str[blen];\
    const auto len = convfunc(val, str, this->radix_);\
    this->write(str, len);\

void OutputStream::print_int(const int val){
    PRINT_INT_TEMPLATE(12, StringUtils::itoa);
}

void OutputStream::print_int(const uint64_t val){
    PRINT_INT_TEMPLATE(24, StringUtils::iutoa);
}

void OutputStream::print_int(const int64_t val){
    PRINT_INT_TEMPLATE(24, StringUtils::iltoa);
}


// PUT_FLOAT_TEMPLATE(float, StringUtils::ftoa)
// PUT_FLOAT_TEMPLATE(double, StringUtils::ftoa)

#undef PUT_FLOAT_TEMPLATE

OutputStream & OutputStream::operator<<(const bool val){
    if(b_boolalpha == false){
        write(val ? '1' : '0');
        return *this;
    }else{
        return *this << ((val) ? "true" : "false"); 
    }
}