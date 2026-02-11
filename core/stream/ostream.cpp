#include "ostream.hpp"
#include "core/string/legacy/xtoa.hpp"
#include "core/clock/clock.hpp"
#include "core/string/view/string_view.hpp"
#include "core/string/view/mut_string_view.hpp"


// #include "core/string/utils/strconv2.hpp"

#include <source_location>

using namespace ymd;


static constexpr StringView get_basealpha(const size_t _radix){
    switch(_radix){
        default:
        case 10:
            return StringView("");
        case 2:
            return StringView("0b");
        case 8:
            return StringView("0");
        case 16:
            return StringView("0x");
    }
}

void OutputStream::write_byte(const uint8_t byte){
    const auto bytes = std::span<const uint8_t, 1>{&byte, 1};
    return write_bytes(bytes);
}

void OutputStream::write_bytes(std::span<const uint8_t> bytes){
    #if 0
    buf_.push_bytes(
        bytes,
        [this](const std::span<const uint8_t> _pbuf){
            size_t written_len = 0;
            while(written_len < _pbuf.size()){
                const size_t rem_len = static_cast<size_t>(_pbuf.size() - written_len);
                written_len += this->sendout(std::span(_pbuf.data() + written_len, rem_len));
            }
        }
    );
    #else
    size_t written_len = 0;
    while(written_len < bytes.size()){
        const size_t rem_len = static_cast<size_t>(bytes.size() - written_len);
        written_len += this->sendout(std::span(bytes.data() + written_len, rem_len));
    }
    #endif
}

OutputStream & OutputStream::flush(){
    // buf_.flush([this](const std::span<const uint8_t> pbuf){this->sendout(pbuf);});
    return *this;
}

OutputStream & OutputStream::operator<<(const wchar_t chr){
    write_byte(chr); return *this;}

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


    }while(false);

    return *this;
}


OutputStream & OutputStream::operator<<(const std::string & str){
    *this << std::string_view(str);
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



#define PRINT_INT_TEMPLATE(cap, convfunc)\
    if((config_.specifier.showpos and val >= 0)) [[unlikely]]\
        this->write_byte('+');\
    if((config_.specifier.showbase and (radix() != 10))) [[unlikely]]{\
        *this << get_basealpha(radix());}\
    char buf[cap];\
    const auto len = convfunc(val, buf, this->config_.radix);\
    this->write_bytes(std::span<const uint8_t>(reinterpret_cast<const uint8_t *>(buf), len));\

#define PRINT_NUMERIC(buf, len, is_positive)\
    if(config_.specifier.showpos and is_positive) *this << '+';\
    this->write_bytes(std::span<const uint8_t>(reinterpret_cast<const uint8_t *>(buf), len));\
\

OutputStream & OutputStream::operator<<(const uint8_t val){
    if(radix() >= 4){
        PRINT_INT_TEMPLATE(4, str::itoa);
    }else{
        PRINT_INT_TEMPLATE(8, str::itoa);
    }
    return *this;
}

OutputStream & OutputStream::operator<<(const float val){
    char buf[32];
    const auto len = str::ftoa(val, buf, this->eps());
    PRINT_NUMERIC(buf, len, (val >= 0));
    return *this;
}

OutputStream & OutputStream::operator<<(const double val){
    return (*this) << static_cast<float>(val);
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



void OutputStream::print_iq32(const int32_t bits, const uint32_t Q){
    char buf[32];
    const auto len = str::qtoa(bits, Q, buf, this->eps());
    PRINT_NUMERIC(buf, len, (bits >= 0));
}

void OutputStream::print_uq32(const uint32_t bits, const uint32_t Q){
    char buf[32];
    const auto len = str::qtoa(bits, Q, buf, this->eps());
    PRINT_NUMERIC(buf, len, (bits >= 0));
}


OutputStream & OutputStream::operator<<(const std::monostate){
    return *this << StringView("monostate");
}


OutputStream & OutputStream::operator<<(const StringView str){
    write_bytes(std::span<const uint8_t>(
        reinterpret_cast<const uint8_t *>(str.data()),
        str.length())
    );
    return * this;
}

OutputStream & OutputStream::operator<<(const MutStringView str){
    write_bytes(std::span<const uint8_t>(
        reinterpret_cast<const uint8_t *>(str.data()),
        str.length())
    );
    return * this;
}

OutputStream & OutputStream::operator<<(const bool val){
    if(config_.specifier.boolalpha == false){
        write_byte('0' + val);
        return *this;
    }else{
        return *this << ((val) ? StringView("true") : StringView("false"));
    }
}


size_t OutputStreamByRoute::sendout(const std::span<const uint8_t> pbuf){
    if(!p_route_.has_value()) [[unlikely]]
        __builtin_trap();
    return p_route_->try_write_bytes(pbuf);
}

OutputStream & OutputStream::operator<<(const std::strong_ordering & ordering){
    if (ordering == std::strong_ordering::greater) {
        return *this << StringView("greater");
    } else if (ordering == std::strong_ordering::less) {
        return *this << StringView("less");
    } else {
        return *this << StringView("equal");
    }
}

OutputStream & OutputStream::operator<<(const std::weak_ordering & ordering){
    if (ordering == std::weak_ordering::greater) {
        return *this << StringView("greater");
    } else if (ordering == std::weak_ordering::less) {
        return *this << StringView("less");
    } else {
        return *this << StringView("equivalent");
    }
}