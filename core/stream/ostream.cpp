#include "ostream.hpp"
#include "core/string/xtoa/xtoa.hpp"
#include "core/clock/clock.hpp"
#include "core/string/view/string_view.hpp"
#include "core/string/view/mut_string_view.hpp"


// #include "core/string/utils/strconv2.hpp"

#include <source_location>

using namespace ymd;


__attribute__((always_inline))
static constexpr char * put_basealpha(char * str, const size_t _radix){
    switch(_radix){
        default:
        case 10:
            return str;
        case 2:
            str[0] = '0';
            str[1] = 'b';
            return str + 2;
        case 8:
            str[0] = '0';
            return str + 1;
        case 16:
            str[0] = '0';
            str[1] = 'x';
            return str + 2;
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

#define PRINT_NUMERIC_BEGIN(cap)\
    std::array<char, cap> buf;\
    char * str = buf.data();\

#define PRINT_NUMERIC_END(convfunc, ...)\
    char * end = convfunc(str, val, ##__VA_ARGS__);\
    this->write_bytes(std::span<const uint8_t>(reinterpret_cast<const uint8_t *>(buf.data()), reinterpret_cast<const uint8_t *>(end)));\

#define PRINT_NUMERIC_TEMPLATE(val, cap, convfunc, ...)\
    PRINT_NUMERIC_BEGIN(cap)\
    if((config_.specifier.showpos and val >= 0)) [[unlikely]]{\
        str[0] = ('+');\
        str++;}\
    PRINT_NUMERIC_END(convfunc, ##__VA_ARGS__)\

#define PRINT_INT_TEMPLATE(val, cap, convfunc, ...)\
    PRINT_NUMERIC_BEGIN(cap)\
    if((config_.specifier.showbase)) [[unlikely]]{\
        str = put_basealpha(str, config_.radix);}\
    else {if((config_.specifier.showpos and val >= 0)) [[unlikely]]{\
        str[0] = ('+');\
        str++;}}\
    PRINT_NUMERIC_END(convfunc, ##__VA_ARGS__)\



OutputStream & OutputStream::operator<<(const float val){
    PRINT_NUMERIC_TEMPLATE(val, 32, str::fmtstr_f32, this->config_.eps)
    return *this;
}

void OutputStream::print_iq32(const int32_t val, const uint32_t Q){
    PRINT_NUMERIC_TEMPLATE(val, 32, str::fmtstr_fixed<int32_t>, this->config_.eps, Q)
}

void OutputStream::print_uq32(const uint32_t val, const uint32_t Q){
    PRINT_NUMERIC_TEMPLATE(val, 32, str::fmtstr_fixed<uint32_t>, this->config_.eps, Q)
}

OutputStream & OutputStream::operator<<(const double val){
    return (*this) << static_cast<float>(val);
}

void OutputStream::print_u32(const uint32_t val){
    PRINT_INT_TEMPLATE(val, 32, str::fmtstr_u32, this->config_.radix);
}

void OutputStream::print_i32(const int32_t val){
    PRINT_INT_TEMPLATE(val, 32, str::fmtstr_i32, this->config_.radix);
}

void OutputStream::print_u64(const uint64_t val){
    PRINT_INT_TEMPLATE(val, 64, str::fmtstr_u64, this->config_.radix);
}

void OutputStream::print_i64(const int64_t val){
    PRINT_INT_TEMPLATE(val, 64, str::fmtstr_i64, this->config_.radix);
}

OutputStream & OutputStream::operator<<(const uint8_t val){
    PRINT_INT_TEMPLATE(val, 8, str::fmtstr_u32, this->config_.radix);
    return *this;
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