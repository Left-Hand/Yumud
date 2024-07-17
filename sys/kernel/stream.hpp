#ifndef __PRINTABLE_HPP__

#define __PRINTABLE_HPP__

#include "../sys/core/system.hpp"
#include "../types/buffer/buffer.hpp"
#include "../types/string/String.hpp"
#include "../thirdparty/sstl/include/sstl/vector.h"
#include "string_utils.hpp"

#include <vector>
#include <array>
#include <cstdint>
#include <string>
#include <string_view>
#include <type_traits>
#include <tuple>

#include <utility>

enum class SpecToken {
    Space,
    NoSpace,
    Comma,
    CommaWithSpace,
    Tab,
    End,

    Bin,
    Oct,
    Dec,
    Hex,

    Eps1,
    Eps2,
    Eps3,
    Eps4,
    Eps5,
    Eps6
};

class BasicStream{

};

class InputStream:virtual public BasicStream{
public:
    virtual void read(char & data) = 0;
    virtual void read(char * data_ptr, const size_t len){
        for(size_t i=0;i<len;i++) read(data_ptr[i]);
    }

    char read(){char data; read(data); return data;};

    String readString(const size_t & len);
    String readStringUntil(const char & chr);
    String readString(){return readString(available());}
    virtual size_t available() const = 0;
};

class OutputStream: virtual public BasicStream{
private:
    // void write(const char * data_ptr){
    //     for(size_t i=0;data_ptr[i] != 0; i++) write(data_ptr[i]);
	// }
public:
    uint8_t radix = 10;
    uint8_t eps = 2;
protected:
    String space = ", ";

    bool skipSpec = false;

public:

    virtual void write(const char data) = 0;
    virtual void write(const char * data_ptr, const size_t len){
        for(size_t i=0;i<len;i++) write(data_ptr[i]);
	}

    virtual size_t pending() const = 0;

    void setSpace(const String _space){space = _space;}
    void setRadix(const uint8_t _radix){radix = _radix;}
    void setEps(const uint8_t _eps){eps = _eps;}


    template<typename T>
    requires std::is_integral_v<T> && (sizeof(T) <= 4)
    OutputStream & operator<<(const T val){
        char str[12];
        StringUtils::itoa(val, str, this->radix);
        return *this << str;
    }

    template<integral_64 T>
    OutputStream & operator<<(const T val){
        char str[24];
        StringUtils::iutoa(val, str, this->radix);
        return *this << str;
    }

    // template<>
    OutputStream & operator<<(bool val){write(val ? '1' : '0'); return *this;}
    // template<>
    OutputStream & operator<<(const char chr){write(chr); return *this;}
    // template<>
    OutputStream & operator<<(const wchar_t chr){write(chr); return *this;}
    OutputStream & operator<<(char* str){write(str, strlen(str)); return *this;}
    OutputStream & operator<<(const char* str){write(str, strlen(str)); return *this;}
    OutputStream & operator<<(const String & str){write(str.c_str(), str.length()); return *this;}
    OutputStream & operator<<(const std::string & str){write(str.c_str(),str.length()); return *this;}
    OutputStream & operator<<(const std::string_view & str){write(str.data(), str.length()); return * this;}
    OutputStream & operator<<(const float val){*this << String(val); return * this;}
    OutputStream & operator<<(const double val){*this << String(val); return * this;}

    OutputStream & operator<<(const SpecToken & spec);

    #define PRINT_ARR_TEMPLATE\
        size_t _size = arr.size();\
        *this << '[';\
        if(_size > 0){\
            for(size_t i = 0; i < _size - 1; ++i) *this << arr[i] << ',';\
            *this << arr[_size - 1];\
        }\
        *this << ']';\
        return *this;\

    template<typename T, size_t size>
    OutputStream & operator<<(const T (&arr)[size]){
        PRINT_ARR_TEMPLATE
    }

    template<typename T, size_t size>
    OutputStream & operator<<(const std::array<T, size> & arr){
        PRINT_ARR_TEMPLATE
    }

    template<typename T>
    OutputStream & operator<<(const std::vector<T> & arr){
        PRINT_ARR_TEMPLATE
    }

    template<typename T, size_t arr_size>
    OutputStream & operator<<(const sstl::vector<T, arr_size> & arr){
        PRINT_ARR_TEMPLATE
    }

    template<StringUtils::HasToString T>
    OutputStream & operator<<(const T & misc){*this << misc.toString(eps); return *this;}


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
    OutputStream & operator<<(const T & misc){*this << int(misc); return *this;}

    void print(){}

	template <typename T>
	void print(const T & first) {
		*this << first;
		print();
	}

    template <typename real, typename... Args>
    void print(real first, Args... args) {
        *this << first;
        if(!skipSpec) *this << space;
        else skipSpec = false;
        print(args...);
    }

    void println(){*this << "\r\n";}

	template <typename T>
	void println(const T & first) {
		*this << first;
        *this << "\r\n";
	}

    template <typename T, typename... Args>
    void println(T first, Args... args) {
        *this << first;
        if(!skipSpec) *this << space;
        else skipSpec = false;
        println(args...);
    }

    void prints(){*this << "\r\n";}

	template <typename T>
	void prints(const T & first) {
		*this << first;
        *this << "\r\n";
	}

    template <typename T, typename... Args>
    void prints(T first, Args... args) {
        *this << first;
        prints(args...);
    }
private:
};

class IOStream:public OutputStream, public InputStream{
public:
    using InputStream::read;
};

#endif