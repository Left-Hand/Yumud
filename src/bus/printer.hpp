#ifndef __PRINTABLE_HPP__

#define __PRINTABLE_HPP__

#include "bus.hpp"
#include "../../types/buffer/buffer.hpp"
#include "src/platform.h"
#include "../../types/string/String.hpp"
#include <cstdint>
#include <string>
#include <string_view>

// #ifndef endl
// #define endl "\r\n";
// #endif

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

class Printer{
private:
    String space = ", ";
    uint8_t radix = 10;
    uint8_t eps = 2;
    bool skipSpec = false;

    __fast_inline void printString(const String & str){write(str.c_str(), str.length());}
    __fast_inline void printString(const std::string & str){write(str.c_str(), str.length());}
    
    __fast_inline void printString(const std::string_view & str){write(str.data(), str.length());}
    __fast_inline void printString(const char * str){write(str, strlen(str));}

protected:
    virtual void _write(const char & data) = 0;
    virtual void _write(const char * data_ptr, const size_t & len){
        for(size_t i=0;i<len;i++) _write(data_ptr[i]);
	}
    virtual void _read(char & data) = 0;
    virtual void _read(char * data_ptr, const size_t len){
        for(size_t i=0;i<len;i++) _read(data_ptr[i]);
    }


public:
    Printer(){;}
    virtual size_t available() = 0;
    void write(const char & data){_write(data);}
    void write(const char * data_ptr, const size_t & len){_write(data_ptr, len);}

    char read(){char data; _read(data); return data;};
    String readString(const size_t & len);
    String readStringUntil(const char & chr);
    String readStringAll(){return readString(available());}
    void setSpace(const String & _space){space = _space;}
    void setRadix(const uint8_t & _radix){radix = _radix;}
    void setEps(const uint8_t & _eps){eps = _eps;}

    Printer& operator<<(uint8_t val){printString(String((unsigned long long)val, radix)); return *this;}
    Printer& operator<<(uint16_t val){printString(String((unsigned long long)val, radix)); return *this;}
    Printer& operator<<(uint32_t val){printString(String((unsigned long long)val, radix)); return *this;}
    Printer& operator<<(uint64_t val){printString(String((unsigned long long)val, radix)); return *this;}

    Printer& operator<<(int8_t val){printString(String((long long)val, radix)); return *this;}
    Printer& operator<<(int16_t val){printString(String((long long)val, radix)); return *this;}
    Printer& operator<<(int32_t val){printString(String((long long)val, radix)); return *this;}
    Printer& operator<<(int64_t val){printString(String((long long)(val), radix)); return *this;}

    Printer& operator<<(bool val){write(val ? '1' : '0'); return *this;}
    Printer& operator<<(int val){printString(String(val, radix)); return *this;}
    Printer& operator<<(unsigned int val){printString(String(val, radix)); return *this;}
    Printer& operator<<(float val){printString(String(val, eps)); return *this;}
    Printer& operator<<(double val){printString(String(val, eps)); return *this;}

    Printer& operator<<(const char & chr){printString(String(chr)); return *this;}
    Printer& operator<<(char* pStr){printString(String(pStr)); return *this;}
    Printer& operator<<(const char* pStr){printString(String(pStr)); return *this;}
    Printer& operator<<(const String & str){printString(str); return *this;}
    Printer& operator<<(const std::string & str){printString(str); return *this;}
    Printer& operator<<(const std::string_view & str){printString(str); return * this;}

    Printer& operator<<(const SpecToken & spec);

    template<typename real>
    Printer& operator<<(real misc){*this << misc.toString(eps); return *this;}

    void print(){}

	template <typename real>
	void print(const real& first) {
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


    void println(){*this << "\n";}

	template <typename real>
	void println(const real& first) {
		*this << first;
        *this << "\r\n";
	}

    template <typename real, typename... Args>
    void println(real first, Args... args) {
        *this << first;
        if(!skipSpec) *this << space;
        else skipSpec = false;
        println(args...);
    }
};

#endif