#include "ostream.hpp"
#include "core/string/legacy/xtoa.hpp"
#include "core/clock/clock.hpp"
#include "core/string/view/string_view.hpp"
#include "core/string/view/mut_string_view.hpp"


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
            config_.specifier.boolalpha = true;
            break;
        }

        else if (func == &std::noboolalpha){
            config_.specifier.boolalpha = false;
            break;
        }

        else if (func == &std::showpos){
            config_.specifier.showpos = true;
            break;
        }

        else if (func == &std::noshowpos){
            config_.specifier.showpos = false;
            break;
        }

        else if (func == &std::showbase){
            config_.specifier.showbase = true;
            break;
        }

        else if (func == &std::noshowbase){
            config_.specifier.showbase = false;
            break;
        }
        //TODO 支持std::flush

        //TODO 支持std::endl

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
    // this is std::ignore
    // _GLIBCXX17_INLINE constexpr _Swallow_assign ignore{};
    return *this;
}

OutputStream & OutputStream::operator<<(const std::_Setw){
    //TODO
    return *this;
}


OutputStream & OutputStream::operator<<(const std::_Setfill<char> setfill){
    //TODO
    return *this;
}



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
    if((config_.specifier.showpos and val >= 0)) [[unlikely]]\
        this->write('+');\
    if((config_.specifier.showbase and (radix() != 10))) [[unlikely]]{\
        *this << get_basealpha(radix());}\
    char str[blen];\
    const auto len = convfunc(val, str, this->config_.radix);\
    this->write(std::span<const uint8_t>(reinterpret_cast<const uint8_t *>(str), len));\


OutputStream & OutputStream::operator<<(const uint8_t val){
    if(radix() >= 4){
        PRINT_INT_TEMPLATE(4, str::itoa);
    }else{
        PRINT_INT_TEMPLATE(8, str::itoa);
    }
    return *this;
}

void OutputStream::print_u32(const uint32_t val){
    if(radix() >= 4){
        PRINT_INT_TEMPLATE(16, str::iutoa);
    }else{
        PRINT_INT_TEMPLATE(32, str::iutoa);
    }
}
void OutputStream::print_i32(const int32_t val){
    if(radix() >= 4){
        PRINT_INT_TEMPLATE(16, str::itoa);
    }else{
        PRINT_INT_TEMPLATE(32, str::itoa);
    }
}
void OutputStream::print_u64(const uint64_t val){
    if(radix() >= 4){
        PRINT_INT_TEMPLATE(32, str::iutoa);
    }else{
        PRINT_INT_TEMPLATE(64, str::iutoa);
    }
}

void OutputStream::print_i64(const int64_t val){
    if(radix() >= 4){
        PRINT_INT_TEMPLATE(32, str::iltoa);
    }else{
        PRINT_INT_TEMPLATE(64, str::iltoa);
    }
}

void OutputStream::print_iq16(const math::fixed_t<16, int32_t> val){
    char str[32];
    const auto len = str::qtoa<16>(val, str, this->eps());
    print_numeric(str, len, val >= 0);
}

#undef PUT_FLOAT_TEMPLATE



OutputStream & OutputStream::flush(){
    buf_.flush([this](const std::span<const uint8_t> pbuf){this->sendout(pbuf);});
    return *this;
}

void OutputStreamByRoute::sendout(const std::span<const uint8_t> pbuf){
    if(!p_route_.has_value()) [[unlikely]]
        __builtin_trap();
    p_route_->try_write_bytes(pbuf);
}

OutputStream & OutputStream::operator<<(const StringView str){
    write_checked(std::span<const uint8_t>(
        reinterpret_cast<const uint8_t *>(str.data()),
        str.length())
    );
    return * this;
}

OutputStream & OutputStream::operator<<(const MutStringView str){
    write_checked(std::span<const uint8_t>(
        reinterpret_cast<const uint8_t *>(str.data()),
        str.length())
    );
    return * this;
}

OutputStream & OutputStream::operator<<(const bool val){
    if(config_.specifier.boolalpha == false){
        write(val ? '1' : '0');
        return *this;
    }else{
        return *this << ((val) ? StringView("true") : StringView("false"));
    }
}
