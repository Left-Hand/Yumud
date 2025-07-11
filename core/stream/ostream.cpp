#include "ostream.hpp"

#include "core/clock/clock.hpp"
#include "core/string/string_view.hpp"
#include "core/string/string_ref.hpp"

#include <source_location>

using namespace ymd;

OutputStream& OutputStream::operator<<(std::ios_base& (*func)(std::ios_base&)){
    do{
        if (func == &std::oct) {set_radix(8);break;}
        if (func == &std::dec) {set_radix(10);break;}
        if (func == &std::hex) {set_radix(16);break;}
        if (func == &std::fixed) {
            //TODO
            break;
        }
        
        if (func == &std::scientific) {
            //TODO
            break;
        }
        
        if (func == &std::boolalpha){
            config_.boolalpha = true;
            break;
        }

        if (func == &std::noboolalpha){
            config_.boolalpha = false;
            break;
        }

        if (func == &std::showpos){
            config_.showpos = true;
            break;
        }
        
        if (func == &std::noshowpos){
            config_.showpos = false;
            break;
        }

        if (func == &std::showbase){
            config_.showbase = true;
            break;
        }
        
        if (func == &std::noshowbase){
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
    PRINT_FLOAT_TEMPLATE(strconv::ftoa);
    return *this;
}

OutputStream & OutputStream::operator<<(const double value){
    PRINT_FLOAT_TEMPLATE(strconv::ftoa);
    return *this;
}

#define PRINT_INT_TEMPLATE(blen, convfunc)\
    if(unlikely(config_.showpos and val >= 0)) \
        this->write('+');\
    if(unlikely(config_.showbase and (radix() != 10))){\
        *this << get_basealpha(radix());}\
    char str[blen];\
    const auto len = convfunc(val, str, this->config_.radix);\
    this->write(str, len);\


OutputStream & OutputStream::operator<<(const uint8_t val){
    PRINT_INT_TEMPLATE(3, strconv::itoa);
    return *this;
}

void OutputStream::print_int(const int val){
    PRINT_INT_TEMPLATE(12, strconv::itoa);
}

void OutputStream::print_u64(const uint64_t val){
    PRINT_INT_TEMPLATE(24, strconv::iutoa);
}

void OutputStream::print_i64(const int64_t val){
    PRINT_INT_TEMPLATE(24, strconv::iltoa);
}

void OutputStream::print_q16(const q16 val){
    char str[12] = {0};
    const auto len = strconv::qtoa<16>(val, str, this->eps());
    print_numeric(str, len, val >= 0);
}

#undef PUT_FLOAT_TEMPLATE



OutputStream & OutputStream::flush(){
    buf_.flush([this](const std::span<const char> pbuf){this->sendout(pbuf);});
    return *this;
}

void OutputStreamByRoute::sendout(const std::span<const char> pbuf){
    if(!p_route_.has_value()) while(true);
    p_route_->writeN(pbuf.data(), pbuf.size());
}

#if 0
OutputStream & OutputStream::operator<<(const String & str){
    write_checked(str.c_str(), str.length()); return * this;}
#endif

OutputStream & OutputStream::operator<<(const StringView str){
    write_checked(str.data(), str.length()); return * this;}

OutputStream & OutputStream::operator<<(const StringRef str){
    write_checked(str.data(), str.length()); return * this;}

OutputStream & OutputStream::operator<<(const bool val){
    if(config_.boolalpha == false){
        write(val ? '1' : '0');
        return *this;
    }else{
        return *this << ((val) ? "true" : "false"); 
    }
}