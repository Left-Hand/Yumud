#ifndef __PRINTABLE_HPP__

#define __PRINTABLE_HPP__

#include "bus.hpp"
#include "../ringbuf/ringbuf.hpp"
#include "../defines/comm_inc.h"

#include <cstdint>

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
    char space[3] = " ";
    uint8_t radix = 10;
    uint8_t eps = 2;
    bool skipSpec = false;

protected:
    virtual void _write(const char & data) = 0;
    virtual void _write(const char * data_ptr, const size_t & len) = 0;
    virtual void _read(char & data) = 0;
    virtual void _read(char * data, const size_t len) = 0;
    virtual char * _get_read_ptr(){return nullptr;}
public:
    virtual size_t available() = 0;

    __fast_inline void write(const char & data){_write(data);}
    void write(const char * data_ptr, const size_t & len){_write(data_ptr, len);}

    __fast_inline char read(){char data; _read(data); return data;};
    String read(const size_t & len);

    void print(const char& chr){write((chr));}
    void print(const char* pStr, size_t len){write((pStr), len);}
    void print(const char* pStr){print(pStr, strlen(pStr));}
    void print(const String & str){print(str.c_str(), str.length());}

    void println(const char* pStr){print(pStr, strlen(pStr)); print("\r\n");}
    void println(const String & str){println(str.c_str());}

    void print(int val){print(String(val, radix));}
    void print(float val){print(String(val, eps));}
    void print(double val){print(String(val, eps));}

    void println(int val){print(val); print("\r\n");}
    void println(float val){print(val); print("\r\n");}
    void println(double val){print(val); print("\r\n");}

    Printer& operator<<(int val){print(val); return *this;}
    Printer& operator<<(float val){print(val); return *this;}
    Printer& operator<<(double val){print(val); return *this;}

    Printer& operator<<(const char chr){print(chr); return *this;}
    Printer& operator<<(const char* pStr){print(pStr); return *this;}
    Printer& operator<<(const String & str){print(str); return *this;}
    Printer& operator<<(const SpecToken & spec);

    template<typename T>
    Printer& operator<<(T misc){*this << String(misc); return *this;}

    void print(){
    }

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


    void println(){
        *this << "\r\n";
    }

	template <typename T>
	void println(const T& first) {
		*this << first;
		println();
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