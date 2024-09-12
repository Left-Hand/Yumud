#include "ostream.hpp"
#include "sys/string/string.hpp"
#include "sys/string/string_view.hpp"

OutputStream& OutputStream::operator<<(const SpecToken & spec){
    switch(spec){
    
    case SpecToken::NoSpace:
        splitter = "\0";
        break;
    case SpecToken::Space:
        splitter = " ";
        break;
    case SpecToken::Comma:
        splitter = ",";
        break;
    case SpecToken::CommaWithSpace:
        splitter = ", ";
        break;
    case SpecToken::Tab:
        splitter = "\t";
        break;
    case SpecToken::End:
        splitter = "\r\n";
        break;
    
    case SpecToken::Bin:
        radix_ = 2;
        break;
    case SpecToken::Oct:
        radix_ = 8;
        break;
    case SpecToken::Dec:
        radix_ = 10;
        break;
    case SpecToken::Hex:
        radix_ = 16;
        break;

    case SpecToken::Eps1:
        eps_ = 1;
        break;
    case SpecToken::Eps2:
        eps_ = 2;
        break;
    case SpecToken::Eps3:
        eps_ = 3;
        break;
    case SpecToken::Eps4:
        eps_ = 4;
        break;
    case SpecToken::Eps5:
        eps_ = 5;
        break;
    case SpecToken::Eps6:
        eps_ = 6;
        break;
    }

    skip_split = true;
    return *this;
}


OutputStream& OutputStream::operator<<(std::ios_base& (*func)(std::ios_base&)){
    do{
        if (func == &std::oct) {setRadix(8);break;}
        if (func == &std::dec) {setRadix(10);break;}
        if (func == &std::hex) {setRadix(16);break;}
        if (func == &std::fixed) {
            //TODO
            break;}
        if (func == &std::scientific) {
            //TODO
            break;}
    }while(false);

    skip_split = true;
    return *this;
}

OutputStream & OutputStream::operator<<(const iq_t value){
    char str_int[str_int_size] = {0};
    char str_frac[str_float_size] = {0};
    StringUtils::qtoa(str_int, str_frac, value.value, this->eps());
    if(value < 0) *this << '-';
    return *this << str_int << '.' << str_frac;
}

void OutputStream::setSpace(const String & _space){splitter= _space.c_str();}

OutputStream & OutputStream::operator<<(const String & str){write(str.c_str(), str.length()); return *this;}
OutputStream & OutputStream::operator<<(const StringView & str){write(str.data(), str.length()); return * this;}
OutputStream & OutputStream::operator<<(const float val){*this << String(val); return * this;}
OutputStream & OutputStream::operator<<(const double val){*this << String(val); return * this;}
