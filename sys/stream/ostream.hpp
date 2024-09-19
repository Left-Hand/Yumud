#pragma once

#include "stream_base.hpp"
#include "sys/string/string_utils.hpp"

class String;
class StringStream;

class OutputStream: virtual public BasicStream{
protected:
    uint8_t radix_ = 10;
    uint8_t eps_ = 2;

    const char * splitter = ", ";
    volatile bool skip_split = false;

    template<typename T>
    void print_arr(const T * _begin, const T * _end){
        const int _size = _end - _begin;
        *this << '[';
        if(_size > 0){
            for(size_t i = 0; i < size_t(_size - 1); ++i) *this << _begin[i] << ',';
            *this << _begin[_size - 1];
        }else{
            *this << '\\';
        }
        *this << ']';
    }


    void print_entity(auto && any){
        if(skip_split == false){
            *this << splitter;
        }
        skip_split = false;
        *this << any;
    }
public:

    virtual void write(const char data) = 0;
    virtual void write(const char * data_ptr, const size_t len){
        for(size_t i = 0; i<len; i++) write(data_ptr[i]);
	}

    virtual size_t pending() const = 0;

    void setSpace(const String & _space);
    void setRadix(const uint8_t _radix){radix_ = _radix;}
    void setEps(const uint8_t _eps){eps_ = _eps;}


    template<typename T>
    requires std::is_integral_v<T> && (sizeof(T) <= 4)
    OutputStream & operator<<(const T val){
        char str[12];
        StringUtils::itoa(val, str, this->radix_);
        return *this << str;
    }

    template<integral_u64 T>
    OutputStream & operator<<(const T val){
        char str[24];
        StringUtils::iutoa(val, str, this->radix_);
        return *this << str;
    }

    template<integral_s64 T>
    OutputStream & operator<<(const T val){
        char str[24];
        StringUtils::itoa(val, str, this->radix_);
        return *this << str;
    }
    OutputStream & operator<<(const bool val){write(val ? '1' : '0'); return *this;}
    OutputStream & operator<<(const char chr){write(chr); return *this;}
    OutputStream & operator<<(const wchar_t chr){write(chr); return *this;}
    OutputStream & operator<<(char* str){if(str) write(str, strlen(str)); return *this;}
    OutputStream & operator<<(const char* str){if(str) write(str, strlen(str)); return *this;}
    OutputStream & operator<<(const std::string & str){write(str.c_str(),str.length()); return *this;}
    OutputStream & operator<<(const String & str);
    OutputStream & operator<<(const StringView & str);
    OutputStream & operator<<(const float val);
    OutputStream & operator<<(const double val);
    OutputStream & operator<<(const iq_t val);
    OutputStream & operator<<(const SpecToken & spec);

    OutputStream& operator<<(std::ios_base& (*func)(std::ios_base&));
    OutputStream& operator<<(const std::_Setprecision & n){eps_ = n._M_n; skip_split = true; return *this;}

    template<typename T, size_t size>
    OutputStream & operator<<(const T (&arr)[size]){
        print_arr(&arr[0], &arr[size]);
        return *this;
    }

    template<typename T, size_t size>
    OutputStream & operator<<(const std::array<T, size> & arr){
        print_arr(arr.begin(), arr.end());
        return *this;
    }

    template<typename T>
    OutputStream & operator<<(const std::vector<T> & arr){
        print_arr((const T *)&arr[0],(const T *)&arr[arr.size()]);
        return *this;
    }

    template<typename T, size_t arr_size>
    OutputStream & operator<<(const sstl::vector<T, arr_size> & arr){
        print_arr(arr.begin(), arr.end());
        return *this;
    }

    template<HasToString T>
    OutputStream & operator<<(const T & misc){*this << misc.toString(eps_); return *this;}

    template <typename... Args>
    auto& operator<<(const std::tuple<Args...>& t) {
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
        return *this;
    }

    template<typename T>
    requires std::is_enum_v<T>
    OutputStream & operator<<(T && misc){*this << int(misc); return *this;}


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
};
