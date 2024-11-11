#pragma once

#include <optional>

#include "stream_base.hpp"
#include "sys/string/string.hpp"


namespace yumud{


class String;
class StringStream;

template<typename T>
concept HasToString = requires(T t, unsigned char eps) {
    { t.toString(eps) } -> ::std::convertible_to<String>;
};

class OutputStream: virtual public BasicStream{
protected:
    uint8_t radix_ = 10;
    uint8_t eps_ = 2;

    String splitter = ", ";
    
    bool skip_split = false;
    bool b_boolalpha = false;
    bool b_showpos = false;
    bool b_showbase = false;
    


    void print_entity(auto && any){
        if(skip_split == false){
            *this << splitter;
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
public:
    OutputStream(){;}
    DELETE_COPY_AND_MOVE(OutputStream)

    virtual void write(const char data) = 0;
    virtual void write(const char * data_ptr, const size_t len){
        for(size_t i = 0; i<len; i++) write(data_ptr[i]);
	}

    virtual size_t pending() const = 0;

    void setSplitter(const StringView _sp){splitter = _sp;}
    void setRadix(const uint8_t _radix){radix_ = _radix;}
    void setEps(const uint8_t _eps){eps_ = _eps;}


    OutputStream & operator<<(const bool val);
    OutputStream & operator<<(const char chr){write(chr); return *this;}
    OutputStream & operator<<(const wchar_t chr){write(chr); return *this;}
    OutputStream & operator<<(char* str){if(str) write(str, strlen(str)); return *this;}
    OutputStream & operator<<(const char* str){if(str) write(str, strlen(str)); return *this;}
    OutputStream & operator<<(const ::std::string & str){write(str.c_str(),str.length()); return *this;}
    OutputStream & operator<<(const ::std::string_view str){write(str.data(),str.length()); return *this;}
    OutputStream & operator<<(const yumud::String & str);
    OutputStream & operator<<(const yumud::StringView str);
    
    OutputStream & operator<<(const float val);
    OutputStream & operator<<(const double val);
    OutputStream & operator<<(const iq_t val);

    OutputStream& operator<<(::std::ios_base& (*func)(::std::ios_base&));
    OutputStream& operator<<(const ::std::_Setprecision & n){eps_ = n._M_n; skip_split = true; return *this;}
    OutputStream& operator<<(const ::std::nullopt_t n){return *this << '/';}
    
    template<typename T>
    OutputStream& operator<<(const ::std::optional<T> v){
        if(bool(v)){
            return *this << v.value();
        }else{
            return *this << ::std::nullopt;        
        }
    }


    //#region print integer
    #define PUT_INT_CONTEXT_TEMPLATE(len, convfunc)\
        if(b_showpos and val >= 0) *this << '+';\
        if(b_showbase and (radix() != 10)){*this << get_basealpha(radix());}\
        char str[len];\
        convfunc(val, str, this->radix_);\
        return *this << str;\

    template<typename T>
    requires ::std::is_integral_v<T> && (sizeof(T) <= 4)
    OutputStream & operator<<(const T val){
        PUT_INT_CONTEXT_TEMPLATE(12, StringUtils::itoa)
    }

    template<integral_u64 T>
    OutputStream & operator<<(const T val){
        PUT_INT_CONTEXT_TEMPLATE(24, StringUtils::iutoa)
    }

    template<integral_s64 T>
    OutputStream & operator<<(const T val){
        PUT_INT_CONTEXT_TEMPLATE(24, StringUtils::iutoa)
    }
    //#endregion

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

    template<typename T, size_t N>
    OutputStream & operator<<(const T (&arr)[N]){
        print_arr(&arr[0], N);
        return *this;
    }

    template<typename T, size_t N>
    OutputStream & operator<<(const ::std::array<T, N> & arr){
        print_arr(arr.begin(), N);
        return *this;
    }

    template<typename T>
    OutputStream & operator<<(const ::std::vector<T> & arr){
        print_arr((const T *)&arr[0], arr.size());
        return *this;
    }

    template<typename T, size_t N>
    OutputStream & operator<<(const sstl::vector<T, N> & arr){
        print_arr(arr.begin(), N);
        return *this;
    }

    //#endregion


    template <typename... Args>
    auto& operator<<(const ::std::tuple<Args...>& t) {
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
        return *this;
    }

    template<typename T>
    requires ::std::is_enum_v<T>
    OutputStream & operator<<(T && e){return *this;}



    template<HasToString T>
    OutputStream & operator<<(const T & misc){*this << misc.toString(eps_); return *this;}



    template <class... Args>
    void println(Args&&... args){
        skip_split = true;
        (..., print_entity(args));
        *this << "\r\n";
    }

    template <class... Args>
    void prints(Args&&... args){
        (*this << ... << args) << "\r\n";
    }

    template <class... Args>
    void print(Args&&... args){
        (*this << ... << args);
    }


    auto eps() const {return eps_;}
    auto radix() const {return radix_;}

    void flush(){while(pending()){__nopn(1);};}
};

};