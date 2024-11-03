#include "ostream.hpp"
#include "sys/string/string.hpp"
#include "sys/string/string_view.hpp"


using namespace yumud;

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

#define PUT_FLOAT_TEMPLATE(type, convfunc)\
OutputStream & OutputStream::operator<<(const type value){\
    char str[12] = {0};\
    convfunc(value, str, this->eps());\
    if(b_showpos and value >= 0) *this << '+';\
    return *this << str;\
}\

PUT_FLOAT_TEMPLATE(iq_t, StringUtils::qtoa)
PUT_FLOAT_TEMPLATE(float, StringUtils::ftoa)
PUT_FLOAT_TEMPLATE(double, StringUtils::ftoa)

#undef PUT_FLOAT_TEMPLATE
OutputStream & OutputStream::operator<<(const String & str){write(str.c_str(), str.length()); return * this;}
OutputStream & OutputStream::operator<<(const StringView str){write(str.data(), str.length()); return * this;}

OutputStream & OutputStream::operator<<(const bool val){
    if(b_boolalpha == false){
        write(val ? '1' : '0');
        return *this;
    }else{
        return *this << ((val) ? "true" : "false"); 
    }
}