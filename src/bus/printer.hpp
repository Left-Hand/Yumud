#ifndef __PRINTABLE_HPP__

#define __PRINTABLE_HPP__

#include "bus.hpp"
#include "../../types/buffer/buffer.hpp"
#include "../defines/comm_inc.h"
#include "../../types/string/String.hpp"
#include <cstdint>

#ifndef endl
#define endl "\r\n";
#endif

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
    Buffer & buffer;

    String space = ", ";
    uint8_t radix = 10;
    uint8_t eps = 2;
    bool skipSpec = false;

    __fast_inline void printString(const String & str){write(str.c_str(), str.length());}

protected:
    virtual void _write(const char & data) = 0;
    virtual void _write(const char * data_ptr, const size_t & len) = 0;
    virtual void _read(char & data){buffer.getData((uint8_t &)data);}
    virtual void _read(char * data_ptr, const size_t len){buffer.getDatas((uint8_t *)data_ptr, len);}

public:
    Printer(Buffer & _buffer):buffer(_buffer){;}

    virtual size_t available(){return buffer.available();}

    __fast_inline void write(const char & data){_write(data);}
    void write(const char * data_ptr, const size_t & len){_write(data_ptr, len);}

    __fast_inline char read(){char data; _read(data); return data;};
    String read(const size_t & len);
    String readAll(){return read(available());}
    void setSpace(const String & _space){space = _space;}

    Printer& operator<<(uint8_t val){printString(String((unsigned long long)val, radix)); return *this;}
    Printer& operator<<(uint16_t val){printString(String((unsigned long long)val, radix)); return *this;}
    Printer& operator<<(uint32_t val){printString(String((unsigned long long)val, radix)); return *this;}
    Printer& operator<<(uint64_t val){printString(String((unsigned long long)val, radix)); return *this;}

    Printer& operator<<(int8_t val){printString(String((long long)val, radix)); return *this;}
    Printer& operator<<(int16_t val){printString(String((long long)val, radix)); return *this;}
    Printer& operator<<(int32_t val){printString(String((long long)val, radix)); return *this;}
    Printer& operator<<(int64_t val){printString(String((long long)(val), radix)); return *this;}

    Printer& operator<<(bool val){printString(String(val, radix)); return *this;}
    Printer& operator<<(int val){printString(String(val, radix)); return *this;}
    Printer& operator<<(unsigned int val){printString(String(val, radix)); return *this;}
    Printer& operator<<(float val){printString(String(val, eps)); return *this;}
    Printer& operator<<(double val){printString(String(val, eps)); return *this;}

    Printer& operator<<(const char & chr){printString(String(chr)); return *this;}
    Printer& operator<<(char* pStr){printString(String(pStr)); return *this;}
    Printer& operator<<(const char* pStr){printString(String(pStr)); return *this;}
    Printer& operator<<(const String & str){printString(str); return *this;}
    Printer& operator<<(const SpecToken & spec);

    template<typename T>
    Printer& operator<<(T misc){*this << misc.toString(eps); return *this;}

    void print(){}

	template <typename T>
	void print(const T& first) {
		*this << first;
		print();
	}

    template <typename T, typename... Args>
    void print(T first, Args... args) {
        *this << first;
        if(!skipSpec) *this << space;
        else skipSpec = false;
        print(args...);
    }


    void println(){*this << "\r\n";}

	template <typename T>
	void println(const T& first) {
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
};

#endif