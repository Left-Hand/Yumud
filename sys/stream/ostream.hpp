#pragma once


#include "stream_base.hpp"
#include "sys/string/string.hpp"
#include <ostream>
#include <span>
#include <ranges>

namespace std{
    class source_location;
}

namespace ymd{

class String;
class StringStream;

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

template<>
struct __needprint_helper<__Splitter>{
    static constexpr bool value = false;
};

class OutputStream: virtual public BasicStream{
public:
    struct Config{
        char splitter[4];

        uint8_t radix:4;
        uint8_t eps:4;
        uint8_t indent;
        union{
            uint16_t flags;
            struct{
                uint16_t boolalpha:1;
                uint16_t showpos:1;
                uint16_t showbase:1;
                uint16_t nobrackets:1;
                uint16_t nospace:1;
                uint16_t forcesync:1;
            };
        };
    };
private:
    uint8_t sp_len;


    scexpr Config default_config = {
        .splitter = ", ",
        .radix = 10,
        .eps = 3,
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
        if(unlikely(config_.forcesync)) flush();
    }

    __fast_inline void print_indent(){
        if(likely(config_.indent == 0)) return;
        for(size_t i = 0; i < config_.indent; i++){
            write('\t');
        }
    }

    __fast_inline void print_enter(){
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
    void checked_write(const char data);
    void checked_write(const char * pdata, const size_t len);

    void print_source_loc(const std::source_location & loc);
public:
    OutputStream(){
        reconf(default_config);
    }

    OutputStream(const OutputStream &) = delete;
    OutputStream(OutputStream &&) = delete;

    virtual void write(const char data) = 0;
    virtual void write(const char * pdata, const size_t len){
        for(size_t i = 0; i<len; i++) write(pdata[i]);
	}

    virtual size_t pending() const = 0;

    OutputStream & setSplitter(const char * splitter){
        strcpy(config_.splitter, splitter);
        sp_len = strlen(splitter);
        return *this;
    }
    
    OutputStream & setSplitter(const char splitter){
        config_.splitter[0] = splitter;
        config_.splitter[1] = 0;
        sp_len = 1;
        return *this;
    }

    OutputStream & setRadix(const uint8_t radix){
        config_.radix = radix;
        return *this;
    }

    OutputStream & setIndent(const uint8_t indent){
        config_.indent = indent;
        return *this;
    }

    uint8_t indent() const{
        return config_.indent;
    }

    OutputStream & setEps(const uint8_t eps){
        config_.eps = eps;
        return *this;
    }

    OutputStream & noBrackets(const bool disen = true){
        config_.nobrackets = disen;
        return *this;
    }

    OutputStream & forceSync(const bool en = true){
        config_.forcesync = en;
        return *this;
    }

    OutputStream & noSpace(const bool disen = true){
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
    __inline OutputStream & operator<<(const String & str){checked_write(str.c_str(), str.length()); return * this;}
    __inline OutputStream & operator<<(const StringView str){checked_write(str.data(), str.length()); return * this;}
    
    OutputStream & operator<<(const float val);
    OutputStream & operator<<(const double val);

    template<size_t Q>
    OutputStream & operator<<(const iq_t<Q> val){
        char str[12] = {0};
        const auto len = StringUtils::qtoa<Q>(val, str, this->eps());
        if(config_.showpos and val >= 0) *this << '+';
        this->write(str, len);
        return *this;
    }

    OutputStream& operator<<(std::ostream& (*manipulator)(std::ostream&)) {
        if (manipulator == static_cast<std::ostream& (*)(std::ostream&)>(std::endl)) {
            *this << "\r\n";
            this->flush();
        }
        return *this;
    }

    OutputStream& operator<<(std::ios_base& (*func)(std::ios_base&));
    OutputStream& operator<<(const std::_Setprecision n){config_.eps = n._M_n; return *this;}
    OutputStream& operator<<(const std::_Setbase n){config_.radix = n._M_base; return *this;}
    
    OutputStream& operator<<(const std::nullopt_t){return *this << '/';}
    OutputStream& operator<<(const __Splitter){print_splt(); return *this;}
    OutputStream& operator<<(const std::source_location & loc){print_source_loc(loc); return *this;}
    
    template<typename T>
    OutputStream& operator<<(const std::optional<T> v){
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


    //#region print integer
private:
    void print_int(const int val);
    void print_int(const uint64_t val);
    void print_int(const int64_t val);
public:


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
        *this << '[';
        if(_size > 0){
            for(size_t i = 0; i < size_t(_size - 1); ++i) *this << _begin[i] << ',';
            *this << _begin[_size - 1];
        }else{
            *this << '\\';
        }
        *this << ']';
    }

    template <typename... Args>
    void print_tuple(const std::tuple<Args...> & t){
        using TupleType = std::tuple<Args...>;
        constexpr size_t tupleSize = std::tuple_size<TupleType>::value;
        *this << '(';
        std::apply(
            [&](const auto&... args) {
                ((tupleSize > 1 && &args != &std::get<tupleSize - 1>(t) ? (*this << args << ',') : (*this << args)), ...);
            },
            t
        );
        *this << ')';
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

    template<typename T>
    requires std::is_enum_v<T>
    OutputStream & operator<<(T && e){
        print_int(static_cast<int>(e));
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
        print_enter();
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
        print_enter();
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
        print_enter();
        return *this;
    }

    auto eps() const {return config_.eps;}
    auto radix() const {return config_.radix;}



    __Splitter splitter() const {return {};}

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

    [[nodiscard]] __Guard createGuard(){
        return __Guard(*this);
    }
};

};