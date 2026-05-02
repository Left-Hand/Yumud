#include "ostream.hpp"

#include "core/string/conv/fmtnum/fmtnum.hpp"
#include "core/string/view/string_view.hpp"
#include "core/string/view/mut_string_view.hpp"


#include <source_location>

using namespace ymd;


__attribute__((always_inline))
static constexpr char * put_basealpha_lower(char * p_str, const uint32_t radix){
    switch(radix){
        case 10:
            return p_str;
        case 2:
            p_str[0] = '0';
            p_str[1] = 'b';
            return p_str + 2;
        case 8:
            p_str[0] = '0';
            return p_str + 1;
        case 16:
            p_str[0] = '0';
            p_str[1] = 'x';
            return p_str + 2;
    }

    //should unreachable
    return p_str;
}

void OutputStream::write_nt_chars(const char * p_str){
    write_bytes(std::span<const uint8_t>(
        reinterpret_cast<const uint8_t *>(p_str),
        strlen(p_str))
    );
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
    return *this;
}

OutputStream & OutputStream::operator<<(const wchar_t chr){
    write_byte(chr); return *this;}



OutputStream & OutputStream::operator<<(const std::string & str){
    *this << std::string_view(str);
    return *this;
}

OutputStream & OutputStream::operator<<(const std::endian endian){
    auto & os = *this;
    switch(endian){
        case std::endian::little: return os << ("little");
        case std::endian::big: return os << ("big");
    }
    __builtin_unreachable();
}

OutputStream & OutputStream::operator<<(const std::_Swallow_assign){
    // this is std::ignore,dont print
    // _GLIBCXX17_INLINE constexpr _Swallow_assign ignore{};
    return *this;
}

OutputStream & OutputStream::operator<<(const std::_Setw){
    //TODO
    return *this;
}


OutputStream & OutputStream::operator<<(const std::_Setfill<char> setfill){
    //TODO

    (void)setfill;
    return *this;
}

OutputStream & OutputStream::operator<<(const std::monostate){
    return *this << ("monostate");
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
        return *this << ((val) ? "true" : "false");
    }
}


size_t OutputStreamByRoute::sendout(const std::span<const uint8_t> pbuf){
    if(!p_route_.has_value()) [[unlikely]]
        __builtin_trap();
    return p_route_->try_write_bytes(pbuf);
}

OutputStream & OutputStream::operator<<(const std::strong_ordering & ordering){
    if (ordering == std::strong_ordering::greater) {
        return *this << ("greater");
    } else if (ordering == std::strong_ordering::less) {
        return *this << ("less");
    } else {
        return *this << ("equal");
    }
}

OutputStream & OutputStream::operator<<(const std::weak_ordering & ordering){
    if (ordering == std::weak_ordering::greater) {
        return *this << ("greater");
    } else if (ordering == std::weak_ordering::less) {
        return *this << ("less");
    } else {
        return *this << ("equivalent");
    }
}



void OutputStream::print_source_loc(const std::source_location & loc){
    const auto guard = this->create_guard();
    this->println();

    this->config_.set_splitter('\0');

    this->println(loc.function_name());
    this->println(loc.file_name(), '(', loc.line(), ':', loc.column(), ')');
}

template<typename T>
static constexpr bool is_positive(T val){
    if constexpr(std::is_signed_v<T>) {
        return val >= 0;
    } else {
        return true;
    }
} 

#define PRINT_NUMERIC_BEGIN(cap)\
    alignas(4) std::array<char, cap> buf;\
    char * p_str = buf.data();\

#define PRINT_NUMERIC_END(convfunc, ...)\
    p_str = convfunc(p_str, val, ##__VA_ARGS__);\
    size_t len = p_str - buf.data();\
    this->write_bytes(std::span(reinterpret_cast<const uint8_t *>(buf.data()), len));\



#define PRINT_NUMERIC_TEMPLATE(val, cap, convfunc, ...)\
    PRINT_NUMERIC_BEGIN(cap)\
    if((config_.specifier.showpos and is_positive(val))) [[unlikely]]{\
        p_str[0] = ('+');\
        p_str++;}\
    PRINT_NUMERIC_END(convfunc, ##__VA_ARGS__)\


#define PRINT_INT_TEMPLATE(val, cap, convfunc, ...)\
    PRINT_NUMERIC_BEGIN(cap)\
    {\
        if((config_.specifier.showbase)) [[unlikely]]{\
            p_str = put_basealpha_lower(p_str, config_.radix);}\
        else if((config_.specifier.showpos and is_positive(val))) [[unlikely]]{\
            p_str[0] = ('+');\
            p_str++;\
        }\
    }\
    PRINT_NUMERIC_END(convfunc, ##__VA_ARGS__)\



OutputStream & OutputStream::operator<<(const float val){
    PRINT_NUMERIC_TEMPLATE(val, 64, str::fmtnum_f32, this->config_.eps)
    return *this;
}

void OutputStream::print_iq32(const int32_t val, const uint32_t Q){
    const str::FixedTypeErased type = {
        .is_signed = true,
        .q_num = static_cast<uint8_t>(Q)
    };

    PRINT_NUMERIC_TEMPLATE(val, 32, str::fmtnum_fixedpoint, this->config_.eps, type)
}

void OutputStream::print_uq32(const uint32_t val, const uint32_t Q){
    const str::FixedTypeErased type = {
        .is_signed = false,
        .q_num = static_cast<uint8_t>(Q)
    };

    PRINT_NUMERIC_TEMPLATE(val, 32, str::fmtnum_fixedpoint, this->config_.eps, type)
}

OutputStream & OutputStream::operator<<(const double val){
    return (*this) << static_cast<float>(val);
}


void OutputStream::print_int32(const uint32_t val, const str::IntTypeErased type){
    PRINT_INT_TEMPLATE(val, (32 + 12), str::fmtnum_integral32, this->config_.radix, type);
}

void OutputStream::print_int64(const uint64_t val, const str::IntTypeErased type){
    PRINT_INT_TEMPLATE(val, (64 + 12), str::fmtnum_integral64, this->config_.radix, type);
}



OutputStream& OutputStream::operator<<(std::ios_base& (*func)(std::ios_base&)){
    do{
        if (func == &std::oct) {config_.set_radix(8);break;}
        else if (func == &std::dec) {config_.set_radix(10);break;}
        else if (func == &std::hex) {config_.set_radix(16);break;}
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


// !warning, take care of your stupid null-terminated c-style string
OutputStream & OutputStream::operator<<(char * str){
    write_nt_chars(str);
    return *this;
}

// !warning, take care of your stupid null-terminated c-style string
OutputStream & OutputStream::operator<<(const char* str){
    write_nt_chars(str);
    return *this;
}