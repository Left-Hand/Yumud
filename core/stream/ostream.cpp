#include "ostream.hpp"
#include "core/string/legacy/strconv.hpp"
#include "core/clock/clock.hpp"
#include "core/string/string_view.hpp"
#include "core/string/mut_string_view.hpp"


// #include "core/string/utils/strconv2.hpp"

#include <source_location>

using namespace ymd;

OutputStream& OutputStream::operator<<(std::ios_base& (*func)(std::ios_base&)){
    do{
        if (func == &std::oct) {set_radix(8);break;}
        else if (func == &std::dec) {set_radix(10);break;}
        else if (func == &std::hex) {set_radix(16);break;}
        else if (func == &std::fixed) {
            //TODO
            break;
        }

        
        else if (func == &std::scientific) {
            //TODO
            break;
        }
        
        else if (func == &std::boolalpha){
            config_.boolalpha = true;
            break;
        }

        else if (func == &std::noboolalpha){
            config_.boolalpha = false;
            break;
        }

        else if (func == &std::showpos){
            config_.showpos = true;
            break;
        }
        
        else if (func == &std::noshowpos){
            config_.showpos = false;
            break;
        }

        else if (func == &std::showbase){
            config_.showbase = true;
            break;
        }
        
        else if (func == &std::noshowbase){
            config_.showbase = false;
            break;
        }
        //TODO 支持std::flush

        // if (func == &std::flush<std::true_type, std::true_type>){
        //     this->flush();
        //     break;
        // }
        // if (func == static_cast<std::ios_base&(*)(std::ios_base&)>(&std::flush)) {
        //     this->flush();
        //     break;
        // }
        //TODO 支持std::endl

        // if (func == +[](OutputStream& os) -> OutputStream& { return std::endl(os); }){
        //     this->write("\r\n", 2);
        //     this->flush();
        //     break;
        // }
    }while(false);

    return *this;
}

OutputStream & OutputStream::operator<<(const std::endian endian){
    auto & os = *this;
    switch(endian){
        case std::endian::little: return os << "little";
        case std::endian::big: return os << "big";
    }
    __builtin_unreachable();
}

OutputStream & OutputStream::operator<<(const std::_Swallow_assign){
    return *this;
}

OutputStream & OutputStream::operator<<(const std::_Setw){
    return *this;
}

#define PRINT_FLOAT_TEMPLATE(convfunc)\
    char str[12] = {0};\
    const auto len = convfunc(value, str, this->eps());\
    if(config_.showpos and value >= 0) *this << '+';\
    this->write(str, len);\


void OutputStream::print_source_loc(const std::source_location & loc){
    const auto guard = this->create_guard();
    this->println();

    this->set_splitter('\0');
    this->set_indent(this->indent());
    
    this->println(loc.function_name());
    this->println(loc.file_name(), '(', loc.line(), ':', loc.column(), ')');
}



OutputStream & OutputStream::operator<<(const float value){
    return (*this) << literals::iq16::from(value);
}

OutputStream & OutputStream::operator<<(const double value){
    return (*this) << literals::iq16::from(value);
}

#define PRINT_INT_TEMPLATE(blen, convfunc)\
    if((config_.showpos and val >= 0)) [[unlikely]]\
        this->write('+');\
    if((config_.showbase and (radix() != 10))) [[unlikely]]{\
        *this << get_basealpha(radix());}\
    char str[blen];\
    const auto len = convfunc(val, str, this->config_.radix);\
    this->write(str, len);\


OutputStream & OutputStream::operator<<(const uint8_t val){
    if(radix() >= 4){
        PRINT_INT_TEMPLATE(4, strconv::itoa);
    }else{
        PRINT_INT_TEMPLATE(8, strconv::itoa);
    }
    return *this;
}

void OutputStream::print_u32(const uint32_t val){
    if(radix() >= 4){
        PRINT_INT_TEMPLATE(16, strconv::iutoa);
    }else{
        PRINT_INT_TEMPLATE(32, strconv::iutoa);
    }
}
void OutputStream::print_i32(const int32_t val){
    if(radix() >= 4){
        PRINT_INT_TEMPLATE(16, strconv::itoa);
    }else{
        PRINT_INT_TEMPLATE(32, strconv::itoa);
    }
}
void OutputStream::print_u64(const uint64_t val){
    if(radix() >= 4){
        PRINT_INT_TEMPLATE(32, strconv::iutoa);
    }else{
        PRINT_INT_TEMPLATE(64, strconv::iutoa);
    }
}

void OutputStream::print_i64(const int64_t val){
    if(radix() >= 4){
        PRINT_INT_TEMPLATE(32, strconv::iltoa);
    }else{
        PRINT_INT_TEMPLATE(64, strconv::iltoa);
    }
}

void OutputStream::print_iq16(const math::fixed_t<16, int32_t> val){
    char str[16] = {0};
    const auto len = strconv::qtoa<16>(val, str, this->eps());
    print_numeric(str, len, val >= 0);
}

#undef PUT_FLOAT_TEMPLATE



OutputStream & OutputStream::flush(){
    buf_.flush([this](const std::span<const char> pbuf){this->sendout(pbuf);});
    return *this;
}

void OutputStreamByRoute::sendout(const std::span<const char> pbuf){
    if(!p_route_.has_value()) __builtin_trap();
    p_route_->try_write_chars(pbuf.data(), pbuf.size());
}

OutputStream & OutputStream::operator<<(const StringView str){
    write_checked(str.data(), str.length()); return * this;}

OutputStream & OutputStream::operator<<(const MutStringView str){
    write_checked(str.data(), str.length()); return * this;}

OutputStream & OutputStream::operator<<(const bool val){
    if(config_.boolalpha == false){
        write(val ? '1' : '0');
        return *this;
    }else{
        return *this << ((val) ? "true" : "false"); 
    }
}