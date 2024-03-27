#include "printer.hpp"

Printer& Printer::operator<<(const SpecToken & spec){
    switch(spec){
    
    case SpecToken::NoSpace:
        space = '\0';
        break;
    case SpecToken::Space:
        space = " ";
        break;
    case SpecToken::Comma:
        space = ",";
        break;
    case SpecToken::CommaWithSpace:
        space = ", ";
        break;
    case SpecToken::Tab:
        space = "\t";
        break;
    case SpecToken::End:
        space = "\r\n";
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
        eps = 1;
        break;
    case SpecToken::Eps2:
        eps = 2;
        break;
    case SpecToken::Eps3:
        eps = 3;
        break;
    case SpecToken::Eps4:
        eps = 4;
        break;
    case SpecToken::Eps5:
        eps = 5;
        break;
    case SpecToken::Eps6:
        eps = 6;
        break;
    }

    skipSpec = true;
    return *this;
}

String Printer::readString(const size_t & len){
    String str;
    str.reserve(len + 1);

    for(uint8_t i = 0; i < len; i++){
        char _;
        _read(_);
        str += _;
    }

    return str;
}

String Printer::readStringUntil(const char & chr) {
    String str;
    size_t cap = 16;
    str.reserve(cap);

    char _;
    while (true) {
        _read(_);

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