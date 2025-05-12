#pragma once


#include <ostream>
#include <span>
#include <ranges>
#include <cstring>

#include "stream_base.hpp"
#include "core/stream/CharOpTraits.hpp"
#include "core/buffer/ringbuf/Fifo_t.hpp"


namespace std{
    class source_location;
}

#ifndef OSTREAM_BUF_SIZE
static constexpr size_t OSTREAM_BUF_SIZE = 64;
#endif

namespace ymd{

class String;
class StringView;
class StringStream;

template<typename T>
struct derive_debug_dispatcher {};

template<typename T>
concept has_derive_debug_dispatcher = requires(OutputStream& os, const T& value) {
    derive_debug_dispatcher<T>::call(os, value);
};


#define DERIVE_DEBUG(type)\
OutputStream& operator<<(OutputStream& os,const type value) {\
    derive_debug_dispatcher<type>::call(os, value);\
    return os;\
}\


template<size_t Q>
struct iq_t;

template <typename T>
struct __needprint_helper {
    static constexpr bool value = true;
};

template <>
struct __needprint_helper<std::ios_base& (*)(std::ios_base&)>{
    static constexpr bool value = false;
};

template<>
struct __needprint_helper<std::_Setprecision>{
    static constexpr bool value = false;
};


template<>
struct __needprint_helper<std::_Setbase>{
    static constexpr bool value = false;
};

struct __Splitter{};
struct __Endl{};


template<char c>
struct __Brackets{
    static constexpr char chr = c;
};

template<>
struct __needprint_helper<__Splitter>{
    static constexpr bool value = false;
};

class OutputStream{
public:
    struct Config{
        char splitter[4];

        uint8_t radix:5;
        uint8_t eps:3;
        uint8_t indent;
        union{
            uint16_t flags;
            struct{
                uint16_t boolalpha:1;
                uint16_t showpos:1;
                uint16_t showbase:1;
                uint16_t no_brackets:1;
                uint16_t nospace:1;
                uint16_t force_sync:1;
            };
        };
    };
private:

    uint8_t sp_len;

    scexpr Config DEFAULT_CONFIG = {
        .splitter = ", ",
        .radix = 10,
        .eps = 3,
        .indent = 0,
        .flags = 0,
    };

    Config config_;

    __fast_inline void print_splt(){
        write(config_.splitter, sp_len);
    }

    template<typename T>
    __fast_inline void print_splt_then_entity(T && any){
        if constexpr(true == __needprint_helper<std::decay_t<T>>::value){
            print_splt();
        }
        *this << std::forward<T>(any);
    }

    template<typename T>
    __fast_inline void print_splt_then_entity(const char splt, T && any){
        if constexpr(true ==__needprint_helper<std::decay_t<T>>::value){
            write(splt);
        }
        *this << std::forward<T>(any);
    }

    __fast_inline void print_end(){
        flush();
        if(unlikely(config_.force_sync)){
            while(pending()) __nopn(1);
        }
    }

    __fast_inline void print_indent(){
        if(likely(config_.indent == 0)) return;
        for(size_t i = 0; i < config_.indent; i++){
            write('\t');
        }
    }

    __fast_inline void print_endl(){
        scexpr const char * enter_str = "\r\n";
        scexpr size_t enter_str_len = 2;
        
        write(enter_str, enter_str_len);
        print_end();
    }

    scexpr const char * get_basealpha(const size_t _radix){
        switch(_radix){
            default:
            case 10:
                return "";
            case 2:
                return "0b";
            case 8:
                return "0";
            case 16:
                return "0x";
        }
    }

    int transform_char(const char chr) const;
    void checked_write(const char data){
        const auto res = transform_char(data);
        if(res >= 0) write(res);
    }

    void checked_write(const char * pdata, const size_t len);

    void print_source_loc(const std::source_location & loc);

    struct Buf{
        
        char buf[OSTREAM_BUF_SIZE];
        uint8_t size = 0;
        
        
        // 用于压入数据，当数据溢满时发送数据包
        template<typename Fn>
        __fast_inline void push(const std::span<const char> pdata, Fn&& fn) {
            size_t offset = 0;
            while (offset < pdata.size()) {
                size_t available = OSTREAM_BUF_SIZE - size;
                size_t copy_size = std::min(available, pdata.size() - offset);

                std::memcpy(buf + size, pdata.data() + offset, copy_size);
                size += static_cast<uint8_t>(copy_size);
                offset += copy_size;

                if (size == OSTREAM_BUF_SIZE) {
                    fn(std::span<const char>(buf, OSTREAM_BUF_SIZE));  // 发送缓冲区数据
                    clear();  // 发送后重置缓冲区
                }
            }
        }

        // 用于压入数据，当数据溢满时发送数据包
        template<typename Fn>
        __fast_inline void push(char data, Fn&& fn) {
            buf[size++] = data;
            if (size == OSTREAM_BUF_SIZE) {
                fn(std::span<const char>(buf, OSTREAM_BUF_SIZE));  // 发送缓冲区数据
                clear();  // 发送后重置缓冲区
            }
        }

        // 强制刷新缓冲区（发送剩余数据）
        template<typename Fn>
        __fast_inline void flush(Fn&& fn) {
            if (size > 0) {
                fn(std::span<const char>(buf, size));  // 发送缓冲区数据
                clear();  // 发送后重置缓冲区
            }
        }

        // 清空缓冲区（不发送数据）
        __fast_inline void clear() {
            size = 0;
        }
    };

    Buf buf_;
public:
    OutputStream(){
        reconf(DEFAULT_CONFIG);
    }

    virtual ~OutputStream() = default;

    OutputStream(const OutputStream &) = delete;
    OutputStream(OutputStream &&) = delete;
    
    void write(const char data) {
        buf_.push(data, [this](const std::span<const char> pbuf){this->sendout(pbuf);});
    }
    void write(const char * pdata, const size_t len){
        buf_.push(std::span<const char>(pdata, len),  
        [this](const std::span<const char> pbuf){this->sendout(pbuf);});
	}

    virtual size_t pending() const = 0;

    virtual void sendout(const std::span<const char>) = 0;
    OutputStream & set_splitter(const char * splitter){
        strcpy(config_.splitter, splitter);
        sp_len = strlen(splitter);
        return *this;
    }
    
    OutputStream & set_splitter(const char splitter){
        config_.splitter[0] = splitter;
        config_.splitter[1] = 0;
        sp_len = 1;
        return *this;
    }

    OutputStream & set_radix(const uint8_t radix){
        config_.radix = radix;
        return *this;
    }

    OutputStream & set_indent(const uint8_t indent){
        config_.indent = indent;
        return *this;
    }

    uint8_t indent() const{
        return config_.indent;
    }

    OutputStream & set_eps(const uint8_t eps){
        config_.eps = eps;
        return *this;
    }

    OutputStream & no_brackets(const bool disen = true){
        config_.no_brackets = disen;
        return *this;
    }
    OutputStream & force_sync(const bool en = true){
        config_.force_sync = en;
        return *this;
    }

    OutputStream & no_space(const bool disen = true){
        config_.nospace = disen;
        return *this;
    }


    OutputStream & operator<<(const bool val);
    __inline OutputStream & operator<<(const char chr){checked_write(chr); return *this;}
    __inline OutputStream & operator<<(const wchar_t chr){checked_write(chr); return *this;}
    __inline OutputStream & operator<<(char * str){if(str) checked_write(str, strlen(str)); return *this;}
    __inline OutputStream & operator<<(const char* str){if(str) checked_write(str, strlen(str)); return *this;}
    __inline OutputStream & operator<<(const std::string & str){checked_write(str.c_str(),str.length()); return *this;}
    __inline OutputStream & operator<<(const std::string_view str){checked_write(str.data(),str.length()); return *this;}
    OutputStream & operator<<(const String & str);
    OutputStream & operator<<(const StringView & str);
    __inline OutputStream & operator<<(const std::byte chr){return *this << (uint8_t(chr));}
    
    OutputStream & operator<<(const float val);
    OutputStream & operator<<(const double val);
    OutputStream & operator<<(std::ostream& (*manipulator)(std::ostream&)) {
        if (manipulator == static_cast<std::ostream& (*)(std::ostream&)>(std::endl)) {
            *this << "\r\n";
            this->flush();
        }
        return *this;
    }

    OutputStream & operator<<(std::ios_base& (*func)(std::ios_base&));
    OutputStream & operator<<(const std::_Setprecision n){config_.eps = n._M_n; return *this;}
    OutputStream & operator<<(const std::_Setbase n){config_.radix = n._M_base; return *this;}
    OutputStream & operator<<(const __Endl){this->print_endl(); return *this;}
    
    OutputStream & operator<<(const std::nullopt_t){return *this << '/';}
    OutputStream & operator<<(const __Splitter){print_splt(); return *this;}

    template<char chr>
    OutputStream & operator<<(const __Brackets<chr>){if(!config_.no_brackets){write(chr);} return *this;}
    OutputStream & operator<<(const std::source_location & loc){print_source_loc(loc); return *this;}

    template<typename T>
    OutputStream & operator<<(const std::optional<T> v){
        if(v.has_value()) return *this << v.value();
        else return *this << '/';
    }

    template<size_t N>
    OutputStream & operator<<(const std::bitset<N> bs){
        char str[N + 1];
        for(size_t i = 0; i < N; ++i){
            str[N - 1 - i] = (bs[i]) ? '1' : '0';
        }
        str[N] = '\0';
        return *this << str;
    }

    template<typename T>
    requires has_derive_debug_dispatcher<T>
    OutputStream & operator<<(T && val){
        derive_debug_dispatcher<T>::call(*this, std::forward<T>(val));
        return *this;
    }

    //#region print integer
private:
    void print_int(const int val);
    void print_int(const uint64_t val);
    void print_int(const int64_t val);
    
    __inline void print_numeric(const char * str, const size_t len, const bool pos){
        if(config_.showpos and pos) *this << '+';
        this->write(str, len);
    }
public:

    template<size_t Q>
    OutputStream & operator<<(const iq_t<Q> & val){
        char str[12] = {0};
        const auto len = qtoa<Q>(val, str, this->eps());
        print_numeric(str, len, val >= 0);
        return *this;
    }

    template<typename T>
    requires std::is_integral_v<T>
    OutputStream & operator<<(const T val){
        if constexpr(sizeof(T) <= 4){
            print_int(int(val));
        }else{
            if constexpr (std::is_signed_v<T>){
                print_int(int64_t(val));
            }else{
                print_int(uint64_t(val));
            }
        }
        return *this;
    }

    //#endregion
private:
    //#region print vased containers
    template<typename T>
    void print_arr(const T * _begin, const size_t _size){
        *this << this->brackets<'['>();
        if(_size > 0){
            for(size_t i = 0; i < size_t(_size - 1); ++i) *this << _begin[i] << ',';
            *this << _begin[_size - 1];
        }else{
            *this << '\\';
        }
        *this << this->brackets<']'>();
    }

    template <typename... Args>
    void print_tuple(const std::tuple<Args...> & t){
        using TupleType = std::tuple<Args...>;
        constexpr size_t tupleSize = std::tuple_size<TupleType>::value;
        *this << brackets<'('>();
        std::apply(
            [&](const auto&... args) {
                ((tupleSize > 1 && &args != &std::get<tupleSize - 1>(t) ? (*this << args << ',') : (*this << args)), ...);
            },
            t
        );
        *this << brackets<')'>();
    }
public:
    template <typename T>
    requires std::ranges::contiguous_range<T>
    OutputStream & operator<<(const T& range) {
        print_arr(std::ranges::data(range), std::ranges::size(range));
        return *this;
    }
    //#endregion


    template <typename... Args>
    OutputStream & operator<<(const std::tuple<Args...>& t) {
        print_tuple(t);
        return *this;
    }

    template <typename ... Args>
    OutputStream & print(Args&&... args){
        print_indent();
        if constexpr (sizeof...(args)) {
            ((*this << std::forward<Args>(args)), ...);
        }
        return *this;
    }

    
    template <typename First, typename ... Args>
    OutputStream & prints(First && first, Args&&... args){
        print_indent();
        *this << std::forward<First>(first);

        if constexpr(false == __needprint_helper<std::decay_t<First>>::value){
            return prints(std::forward<Args>(args)...);
        }else if constexpr (sizeof...(args)) {
            (print_splt_then_entity(' ', std::forward<Args>(args)), ...);
        }
        return prints();
    }

    OutputStream & prints(){
        print_endl();
        return *this;
    }

    template <typename First, typename ... Args>
    OutputStream & printt(First && first, Args&&... args){
        print_indent();
        *this << std::forward<First>(first);
        if constexpr(false == __needprint_helper<std::decay_t<First>>::value){
            return printt(std::forward<Args>(args)...);
        }else if constexpr (sizeof...(args)) {
            (print_splt_then_entity('\t', std::forward<Args>(args)), ...);
        }
        return printt();
    }

    OutputStream & printt(){
        print_endl();
        return *this;
    }

    template <typename First, typename ... Args>
    OutputStream & println(First && first, Args&&... args){
        print_indent();
        *this << std::forward<First>(first);
        if constexpr(false == __needprint_helper<std::decay_t<First>>::value){
            return println(std::forward<Args>(args)...);
        }else if constexpr (sizeof...(args)) {
            (print_splt_then_entity(std::forward<Args>(args)), ...);
        }
        return println();
    }

    OutputStream & println(){
        print_endl();
        return *this;
    }

    auto eps() const {return config_.eps;}
    auto radix() const {return config_.radix;}


    __Endl endl() const {return {};}

    __Splitter splitter() const {return {};}

    template<char chr>
    __Brackets<chr> brackets() const {return {};}

    OutputStream & flush();

    OutputStream & reconf(const Config config){
        config_ = config;
        sp_len = strlen(config_.splitter);
        return *this;
    }

    Config config() const {return config_;}

    class __Guard{
        OutputStream & os_;
        const Config config_;
    public:
        __Guard(OutputStream & os) : os_(os), config_(os.config()){}

        ~__Guard(){
            os_.reconf(config_);
        }
    };

    [[nodiscard]] __Guard create_guard(){
        return __Guard(*this);
    }
};


class OutputStreamByRoute final: public OutputStream{
private:
    using Traits = WriteCharTraits;
    using Route = pro::proxy<Traits>;
    Route p_route_;

    void sendout(const std::span<const char> pbuf);
public:
    OutputStreamByRoute(){;}

    OutputStreamByRoute(Route && route):    
        p_route_(std::move(route)){;}


    size_t pending() const {
        if(unlikely(!p_route_)) while(true);
        return p_route_->pending();
    }

    void retarget(Route && p_route){
        p_route_ = std::move(p_route);
    }

    Route & route() {
        if(unlikely(!p_route_)) while(true);
        return p_route_;
    }
};

}