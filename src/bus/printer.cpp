#include "printer.hpp"

Printer& Printer::operator<<(const SpecToken & spec){
    switch(spec){
    
    case SpecToken::NoSpace:
        space[0] = '\0';
        break;
    case SpecToken::Space:
        space[0] = ' ';
        space[1] = '\0';
        break;
    case SpecToken::Comma:
        space[0] = ',';
        space[1] = '\0';
        break;
    case SpecToken::CommaWithSpace:
        space[0] = ',';
        space[1] = ' ';
        space[2] = '\0';
        break;
    case SpecToken::Tab:
        space[0] = '\t';
        space[1] = '\0';
        break;
    case SpecToken::End:
        space[0] = '\r';
        space[1] = '\n';
        space[2] = '\0';
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

String Printer::read(const size_t & len){
    char * head_ptr = _get_read_ptr() - len;
    if(head_ptr){
        String str = String(head_ptr, len);
        _fake_read(len);
        return str;
    }
    else{
        head_ptr = new char[len+1];
        if(head_ptr){
            _read(head_ptr, len);
            String str = String(head_ptr, len);
            delete []head_ptr;
            return str;
        }else{
            return String('\0');
        }
    }
}