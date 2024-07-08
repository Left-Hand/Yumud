#ifndef __PRINTABLE_HPP__

#define __PRINTABLE_HPP__

#include "sys/platform.h"
#include "types/buffer/buffer.hpp"
#include "types/string/String.hpp"


#include <vector>
#include <array>

#include <cstdint>
#include <string>
#include <string_view>
#include <type_traits>

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

    char test(){return 'b';}


    String readString(const size_t & len);
    String readStringUntil(const char & chr);
    String readString(){return readString(available());}
    virtual size_t available() const = 0;
};

class OutputStream: virtual public BasicStream{
protected:
    String space = ", ";
    uint8_t radix = 10;
    uint8_t eps = 2;
    bool skipSpec = false;

    __fast_inline void printString(const String & str){write(str.c_str(), str.length());}
    __fast_inline void printString(const std::string & str){write(str.c_str(), str.length());}
    __fast_inline void printString(const std::string_view & str){write(str.data(), str.length());}
    __fast_inline void printString(const char * str){write(str, strlen(str));}
public:

    virtual void write(const char data) = 0;
    virtual void write(const char * data_ptr, const size_t len){
        for(size_t i=0;i<len;i++) write(data_ptr[i]);
	}

    virtual size_t pending() const = 0;

    void setSpace(const String & _space){space = _space;}
    void setRadix(const uint8_t & _radix){radix = _radix;}
    void setEps(const uint8_t & _eps){eps = _eps;}

    OutputStream & operator<<(uint8_t val){printString(String((unsigned long long)val, radix)); return *this;}
    OutputStream & operator<<(uint16_t val){printString(String((unsigned long long)val, radix)); return *this;}
    OutputStream & operator<<(uint32_t val){printString(String((unsigned long long)val, radix)); return *this;}
    OutputStream & operator<<(uint64_t val){printString(String((unsigned long long)val, radix)); return *this;}

    OutputStream & operator<<(int8_t val){printString(String((long long)val, radix)); return *this;}
    OutputStream & operator<<(int16_t val){printString(String((long long)val, radix)); return *this;}
    OutputStream & operator<<(int32_t val){printString(String((long long)val, radix)); return *this;}
    OutputStream & operator<<(int64_t val){printString(String((long long)(val), radix)); return *this;}

    OutputStream & operator<<(bool val){write(val ? '1' : '0'); return *this;}
    OutputStream & operator<<(int val){printString(String(val, radix)); return *this;}
    OutputStream & operator<<(unsigned int val){printString(String(val, radix)); return *this;}
    OutputStream & operator<<(float val){printString(String(val, eps)); return *this;}
    OutputStream & operator<<(double val){printString(String(val, eps)); return *this;}

    OutputStream & operator<<(const char chr){write(chr); return *this;}
    OutputStream & operator<<(char* pStr){printString(String(pStr)); return *this;}
    OutputStream & operator<<(const char* pStr){printString(String(pStr)); return *this;}
    OutputStream & operator<<(const String & str){printString(str); return *this;}
    OutputStream & operator<<(const std::string & str){printString(str); return *this;}
    OutputStream & operator<<(const std::string_view & str){printString(str); return * this;}

    OutputStream & operator<<(const SpecToken & spec);

    template<typename T, size_t size>
    OutputStream & operator<<(const T (&arr)[size]){
        *this << '[';
        for(size_t i = 0; i < size - 1; ++i)
            *this << arr[i] << ',';
        if(size > 0)
            *this << arr[size - 1];
        *this << ']';
        return *this;
    }

    template<typename T, size_t size>
    OutputStream & operator<<(const std::array<T, size> & arr){
        *this << '[';
        for(size_t i = 0; i < size - 1; ++i)
            *this << arr[i] << ',';
        if(size > 0)
            *this << arr[size - 1];
        *this << ']';
        return *this;
    }

    template<typename T>
    OutputStream & operator<<(const std::vector<T> & arr){
        size_t size = arr.size();
        *this << '[';
        for(size_t i = 0; i < size - 1; ++i)
            *this << arr[i] << ',';
        if(size > 0)
            *this << arr[size - 1];
        *this << ']';
        return *this;
    }

    template<typename T>
    // requires (!std::is_pointer_v<T>)
    OutputStream & operator<<(const T & misc){*this << misc.toString(eps); return *this;}


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
        if(!skipSpec) *this << space;
        else skipSpec = false;
        println(args...);
    }
};

class IOStream:public OutputStream, public InputStream{
public:
    using InputStream::read;
};

#endif