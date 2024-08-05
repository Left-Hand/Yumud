#include "stream.hpp"


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
        radix = 2;
        break;
    case SpecToken::Oct:
        radix = 8;
        break;
    case SpecToken::Dec:
        radix = 10;
        break;
    case SpecToken::Hex:
        radix = 16;
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


String InputStream::readString(const size_t & len){
    String str;
    str.reserve(len + 1);

    for(uint8_t i = 0; i < len; i++){
        char _;
        read(_);
        str += _;
    }

    return str;
}

String InputStream::readStringUntil(const char & chr) {
    String str;
    size_t cap = 16;
    str.reserve(cap);

    char _;
    while (true) {
        read(_);

        if (_ == chr) {
            break;
        }

        str += _;

        if ((size_t)str.length() == cap) {
            cap *= 2;
            str.reserve(cap);
        }
    }

    return str;
}