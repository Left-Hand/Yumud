/*
  WString.h - String library for Wiring & Arduino
  ...mostly rewritten by Paul Stoffregen...
  Copyright (c) 2009-10 Hernando Barragan.  All right reserved.
  Copyright 2011, Paul Stoffregen, paul@pjrc.com

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#pragma once


#include <string.h>
#include <ctype.h>
#include "sys/core/platform.h"


#include "StringView.hpp"
#include "sys/string/utils/StringUtils.hpp"

namespace ymd{
	
// An inherited class for holding the result of a concatenation.  These
// result objects are assumed to be writable by subsequent concatenations.
class StringSumHelper;


// The string class
class String
{
	// use a function pointer to allow for "if (s)" without the
	// complications of an operator bool(). for more information, see:
	// http://www.artima.com/cppsource/safebool.html
	typedef void (String::*StringIfHelperType)() const;
	void StringIfHelper() const {}

	friend class StringSumHelper;
public:
	// constructors
	// creates a copy of the initial value.
	// if the initial value is null or invalid, or if memory allocation
	// fails, the string will be marked as invalid (i.e. "if (s)" will
	// be false).
	String(const char *cstr = "");
	String(const String &str);
	#ifdef __GXX_EXPERIMENTAL_CXX0X__
	String(String &&rval);
	String(StringSumHelper &&rval);
	#endif
	explicit String(char c);
    explicit String(char * c);
    explicit String(char * c, const size_t size);
    explicit String(const char * c, const size_t size);
	String(const ::std::string & str):String(str.c_str(), str.length()){};
	String(const ::std::string_view & str):String(str.data(), str.length()){};
	String(const StringView & str):String(str.data(), str.length()){};

	explicit String(uint8_t value, uint8_t base=10);
	explicit String(int value, uint8_t base=10);
	explicit String(size_t value, uint8_t base=10);
	explicit String(long value, uint8_t base=10);
	explicit String(unsigned long value, uint8_t base=10);
	explicit String(unsigned long long value, uint8_t base=10);
	explicit String(long long value, uint8_t base=10);


	explicit String(float value, uint8_t decimalPlaces = 3);
	explicit String(double value, uint8_t decimalPlaces = 3);
	explicit String(const iq_t value, uint8_t decimalPlaces = 3);
	~String(void);

    uint32_t hash() const {return StringView(*this).hash();}
	
	// memory management
	// return true on success, false on failure (in which case, the string
	// is left unchanged).  reserve(0), if successful, will validate an
	// invalid string (i.e., "if (s)" will be true afterwards)
	uint8_t reserve(size_t size);
	inline size_t length(void) const {return len;}

	// creates a copy of the assigned value.  if the value is null or
	// invalid, or if the memory allocation fails, the string will be 
	// marked as invalid ("if (s)" will be false).
	String & operator = (const String &rhs);
	String & operator = (const char *cstr);
	#ifdef __GXX_EXPERIMENTAL_CXX0X__
	String & operator = (String &&rval);
	String & operator = (StringSumHelper &&rval);
	#endif

	// concatenate (works w/ built-in types)
	
	// returns true on success, false on failure (in which case, the string
	// is left unchanged).  if the argument is null or invalid, the 
	// concatenation is considered unsucessful.  
	uint8_t concat(const String &str);
	uint8_t concat(const char *cstr);
	uint8_t concat(const char *cstr, size_t length);
	uint8_t concat(char c);
	uint8_t concat(uint8_t c);
	uint8_t concat(int num);
	uint8_t concat(size_t num);
	uint8_t concat(long num);
	uint8_t concat(unsigned long num);
	uint8_t concat(float num);
	uint8_t concat(double num);
	
	// if there's not enough memory for the concatenated value, the string
	// will be left unchanged (but this isn't signalled in any way)
	String & operator += (const String &rhs)	{concat(rhs); return (*this);}
	String & operator += (const char *cstr)		{concat(cstr); return (*this);}
	String & operator += (char c)			{concat(c); return (*this);}
	String & operator += (uint8_t num)		{concat(num); return (*this);}
	String & operator += (int num)			{concat(num); return (*this);}
	String & operator += (unsigned int num)		{concat(num); return (*this);}
	String & operator += (long num)			{concat(num); return (*this);}
	String & operator += (unsigned long num)	{concat(num); return (*this);}
	String & operator += (float num)		{concat(num); return (*this);}
	String & operator += (double num)		{concat(num); return (*this);}

	friend StringSumHelper & operator + (const StringSumHelper & lhs, const String &rhs);
	friend StringSumHelper & operator + (const StringSumHelper & lhs, const char *cstr);
	friend StringSumHelper & operator + (const StringSumHelper & lhs, char c);
	friend StringSumHelper & operator + (const StringSumHelper & lhs, uint8_t num);
	friend StringSumHelper & operator + (const StringSumHelper & lhs, int num);
	friend StringSumHelper & operator + (const StringSumHelper & lhs, size_t num);
	friend StringSumHelper & operator + (const StringSumHelper & lhs, long num);
	friend StringSumHelper & operator + (const StringSumHelper & lhs, unsigned long num);
	friend StringSumHelper & operator + (const StringSumHelper & lhs, float num);
	friend StringSumHelper & operator + (const StringSumHelper & lhs, double num);

	// comparison (only works w/ Strings and "strings")
	operator StringIfHelperType() const { return buffer ? &String::StringIfHelper : 0; }
	int compareTo(const String &s) const;
	uint8_t equals(const String &s) const;
	uint8_t equals(const char *cstr) const;
	uint8_t operator == (const String &rhs) const {return equals(rhs);}
	uint8_t operator == (const char *cstr) const {return equals(cstr);}
	uint8_t operator != (const String &rhs) const {return !equals(rhs);}
	uint8_t operator != (const char *cstr) const {return !equals(cstr);}
	uint8_t operator <  (const String &rhs) const;
	uint8_t operator >  (const String &rhs) const;
	uint8_t operator <= (const String &rhs) const;
	uint8_t operator >= (const String &rhs) const;
	uint8_t equalsIgnoreCase(const String &s) const;
	uint8_t startsWith( const String &prefix) const;
	uint8_t startsWith(const String &prefix, size_t offset) const;
	uint8_t endsWith(const String &suffix) const;

	// character acccess
	char charAt(size_t index) const;
	void setCharAt(size_t index, char c);
	char operator [] (size_t index) const;
	char& operator [] (size_t index);
	void getBytes(uint8_t *buf, size_t bufsize, size_t index=0) const;
	void toCharArray(char *buf, size_t bufsize, size_t index=0) const
		{getBytes((uint8_t *)buf, bufsize, index);}
    char * getBuffer() const {return buffer;}
	const char * c_str() const { return buffer; }

	// search
	int indexOf( char ch ) const;
	int indexOf( char ch, size_t fromIndex ) const;
	int indexOf( const String &str ) const;
	int indexOf( const String &str, size_t fromIndex ) const;
	int lastIndexOf( char ch ) const;
	int lastIndexOf( char ch, size_t fromIndex ) const;
	int lastIndexOf( const String &str ) const;
	int lastIndexOf( const String &str, size_t fromIndex ) const;
	StringView substring( size_t beginIndex ) const { return substring(beginIndex, len); };
	StringView substring( size_t beginIndex, size_t endIndex ) const;

	std::vector<StringView> split(const char chr, const size_t times = 0) const;
    constexpr const char * begin() const {return buffer;}
    constexpr const char * end() const {return buffer + len;}
	constexpr size_t size() const {return len;}
	// modification
	void replace(char find, char replace);
	// String& replace(const String& replace);
	// String& replace(const String & find, const String & replace);
	void remove(size_t index);
	void remove(size_t index, size_t count);
	String & toLowerCase(void);
	String & toUpperCase(void);
	void trim(void);
	void alphanum(void);

    bool isNumeric(void) const;
    bool isDigit(void) const;


	template<integral T>
    explicit operator T(void) const{return T(StringView(*this));}

	template<floating T>
    explicit operator T(void) const{return T(StringView(*this));}

	explicit operator iq_t() const {return iq_t(StringView(*this));}

	explicit operator std::string(void) const {return std::string(this->c_str(), this->length());}
	explicit operator std::string_view(void) const {return std::string(this->c_str(), this->length());}

	operator StringView(void) const {return StringView(this->c_str(), this->length());}

	size_t capacity() const {return capacity_;}
// protected:
public:
	char *buffer;	        // the actual char array
	size_t capacity_;  // the array length minus one (for the '\0')
	size_t len;       // the String length (not counting the '\0')
// protected:
public:
	void init(void);
	void invalidate(void);
	uint8_t changeBuffer(size_t maxStrLen);


	// copy and move
	String & copy(const char *cstr, size_t length);
	#ifdef __GXX_EXPERIMENTAL_CXX0X__
	void move(String &rhs);
	#endif
};

class StringSumHelper : public String
{
public:
	StringSumHelper(const String &s) : String(s) {}
	StringSumHelper(const char *p) : String(p) {}
	StringSumHelper(char c) : String(c) {}
	StringSumHelper(uint8_t num) : String(num) {}
	StringSumHelper(int num) : String(num) {}
	StringSumHelper(size_t num) : String(num) {}
	StringSumHelper(long num) : String(num) {}
	StringSumHelper(unsigned long num) : String(num) {}
	StringSumHelper(float num) : String(num) {}
	StringSumHelper(double num) : String(num) {}
};


String toString(char c);
String toString(const char * c);
String toString(uint8_t value, uint8_t base = 10);
String toString(int value, uint8_t base = 10);
String toString(size_t value, uint8_t base = 10);
String toString(long value, uint8_t base = 10);
String toString(unsigned long value, uint8_t base = 10);
String toString(long long value, uint8_t base = 10);
String toString(unsigned long long value, uint8_t base = 10);
String toString(float value, uint8_t decimalPlaces = 3);
String toString(double value, uint8_t decimalPlaces = 3);

template<typename T>
concept HasToString = requires(T t, unsigned char eps) {
    { t.toString(eps) } -> std::same_as<String>; // 假设 toString 返回 String 类型
};

}
