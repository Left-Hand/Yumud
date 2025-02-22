#pragma once


#include "stream_base.hpp"
#include "sys/string/string.hpp"
#include <ostream>
#include <span>
#include <ranges>

namespace ymd{


class String;
class StringStream;


class OutputStream: virtual public BasicStream{
private:

    bool skip_split = false;

    struct Config{
        char splitter[4] = ", ";

        uint8_t radix = 10;
        uint8_t eps = 3;

        union{
            uint8_t flags;
            struct{
                bool boolalpha:1 = false;
                bool showpos:1 = false;
                bool showbase:1 = false;
                bool nobrackets:1 = false;
                bool nospace:1 = false;
            };
        };
    };

    Config config_;

    
    void print_entity(auto && any){
        if(likely(skip_split == false) and likely(config_.splitter[0])){
            write(config_.splitter, strlen(config_.splitter));
        }

        skip_split = false;
        *this << any;
    }

    scexpr const char * get_basealpha(const size_t _radix){
        switch(_radix){
            default:
            case 10:
                return "\0";
            case 2:
                return "0b";
            case 8:
                return "0";
            case 16:
                return "0x";
        }
    }

    int transform(const char chr) const;
    void checked_write(const char data);
    void checked_write(const char * pdata, const size_t len);
public:
    OutputStream() = default;
    OutputStream(const OutputStream &) = delete;
    OutputStream(OutputStream &&) = delete;

    virtual void write(const char data) = 0;
    virtual void write(const char * pdata, const size_t len){
        for(size_t i = 0; i<len; i++) write(pdata[i]);
	}

    virtual size_t pending() const = 0;

    OutputStream & setSplitter(const char * splitter){
        strcpy(config_.splitter, splitter);
        return *this;
    }

    OutputStream & setRadix(const uint8_t radix){
        config_.radix = radix;
        return *this;
    }

    OutputStream & setEps(const uint8_t eps){
        config_.eps = eps;
        return *this;
    }

    OutputStream & noBrackets(const bool disen = true){
        config_.nobrackets = disen;
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
    OutputStream & operator<<(const iq_t val);

    OutputStream& operator<<(std::ostream& (*manipulator)(std::ostream&)) {
        if (manipulator == static_cast<std::ostream& (*)(std::ostream&)>(std::endl)) {
            *this << "\r\n";
            this->flush();
        }
        return *this;
    }

    OutputStream& operator<<(::std::ios_base& (*func)(::std::ios_base&));
    OutputStream& operator<<(const ::std::_Setprecision & n){config_.eps = n._M_n; skip_split = true; return *this;}
    OutputStream& operator<<(const ::std::nullopt_t n){return *this << '/';}
    
    template<typename T>
    OutputStream& operator<<(const ::std::optional<T> v){
        if(v.has_value()) return *this << v.value();
        else return *this << '/';
    }

    template<size_t N>
    OutputStream & operator<<(const ::std::bitset<N> & bs){
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
    requires ::std::is_integral_v<T>
    OutputStream & operator<<(const T val){
        if constexpr(sizeof(T) <= 4){
            print_int(int(val));
        }else{
            if constexpr (::std::is_signed_v<T>){
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
    void print_tuple(const ::std::tuple<Args...> & t){
        using TupleType = ::std::tuple<Args...>;
        constexpr size_t tupleSize = ::std::tuple_size<TupleType>::value;
        *this << '(';
        ::std::apply(
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
    OutputStream & operator<<(const ::std::tuple<Args...>& t) {
        print_tuple(t);
        return *this;
    }

    template<typename T>
    requires ::std::is_enum_v<T>
    OutputStream & operator<<(T && e){
        print_int(static_cast<int>(e));
        return *this;
    }

    template<HasToString T>
    OutputStream & operator<<(const T & misc){*this << misc.toString(config_.eps); return *this;}

    template <typename ... Args>
    void println(Args&&... args){
        skip_split = true;
        (..., print_entity(args));
        *this << "\r\n";
    }

    template <typename ... Args>
    void prints(Args&&... args){
        (*this << ... << args) << "\r\n";
    }

    template <typename ... Args>
    void print(Args&&... args){
        (*this << ... << args);
    }


    auto eps() const {return config_.eps;}
    auto radix() const {return config_.radix;}
    OutputStream & flush();

    OutputStream & reconf(const Config config){
        config_ = config;
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

    __Guard guard(){
        return __Guard(*this);
    }
};

};