/*
  WString.cpp - String library for Wiring & Arduino
  ...mostly rewritten by Paul Stoffregen...
  Copyright (c) 2009-10 Hernando Barragan.  All rights reserved.
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

#include "String.hpp"


using namespace ymd;

String::String(const char *cstr)
{
	init();
	if (cstr) copy(cstr, strlen(cstr));
}

String::String(const String &value)
{
	init();
	*this = value;
}

#ifdef __GXX_EXPERIMENTAL_CXX0X__
String::String(String &&rval)
{
	init();
	move(rval);
}
String::String(StringSumHelper &&rval)
{
	init();
	move(rval);
}
#endif

String::String(char c)
{
	init();
		char buf[2];
		buf[0] = c;
		buf[1] = 0;
		*this = buf;
	}

String::String(char * c){
    init();
    *this = c;
}

String::String(char * c, const size_t size){
	init();
    char * cop = new char[size + 1];
    cop[size] = '\0';
    memcpy(cop, c, size);
    *this = cop;
    delete cop;
}

String::String(const char * c, const size_t size){
    init();
    char * cop = new char[size + 1];
    cop[size] = '\0';
    memcpy(cop, c, size);
    *this = cop;
    delete cop;
}

String::String(uint8_t value, uint8_t base)
{
	init();
	char buf[4] = {0};
	StringUtils::itoa(value, buf, base);
	*this = buf;
}

String::String(int value, uint8_t base)
{
	init();
	char buf[12] = {0};
	StringUtils::itoa(value, buf, base);
	*this = buf;
}

String::String(size_t value, uint8_t base)
{
	init();
	char buf[12] = {0};
	StringUtils::itoa(value, buf, base);
	*this = buf;
}

String::String(long value, uint8_t base)
{
	init();
	char buf[12] = {0};
	StringUtils::itoa(value, buf, base);
	*this = buf;
}

String::String(unsigned long value, uint8_t base)
{
	init();
	char buf[12] = {0};
	StringUtils::itoa(value, buf, base);
	*this = buf;
}

String::String(long long value, uint8_t base)
{
	init();
	char buf[24] = {0};
	StringUtils::iltoa(value, buf, base);
	*this = buf;
}

String::String(unsigned long long value, uint8_t base)
{
	init();
	char buf[24] = {0};
	StringUtils::iutoa(value, buf, base);
	*this = buf;
}

String::String(float value, uint8_t decimalPlaces)
{
	init();
	char buf[12] = {0};
	StringUtils::ftoa(value, buf, decimalPlaces);
    *this = buf;
}

String::String(double value, uint8_t decimalPlaces)
{
	init();
	char buf[12] = {0};
	StringUtils::ftoa(value, buf, decimalPlaces);
    *this = buf;
}



String::~String()
{
	free(buffer);
}

/*********************************************/
/*  Memory Management                        */
/*********************************************/

inline void String::init(void)
{
	buffer = NULL;
	capacity_ = 0;
	len = 0;
}

void String::invalidate(void)
{
	if (buffer) free(buffer);
	buffer = NULL;
	capacity_ = len = 0;
}

uint8_t String::reserve(size_t size)
{
	if (buffer && capacity_ >= size) return 1;
	if (changeBuffer(size)) {
		if (len == 0) buffer[0] = 0;
		return 1;
	}
	return 0;
}

uint8_t String::changeBuffer(size_t maxStrLen)
{
	char *newbuffer = (char *)realloc(buffer, maxStrLen + 1);
	if (newbuffer) {
		buffer = newbuffer;
		capacity_ = maxStrLen;
		return 1;
	}
	return 0;
}

/*********************************************/
/*  Copy and Move                            */
/*********************************************/

String & String::copy(const char *cstr, size_t length)
{
	if (!reserve(length)) {
		invalidate();
		return *this;
	}
	len = length;
	strcpy(buffer, cstr);
	return *this;
}

#ifdef __GXX_EXPERIMENTAL_CXX0X__
void String::move(String &rhs)
{
	if (buffer) {
		if (capacity_ >= rhs.len) {
			strcpy(buffer, rhs.buffer);
			len = rhs.len;
			rhs.len = 0;
			return;
		} else {
			free(buffer);
		}
	}
	buffer = rhs.buffer;
	capacity_ = rhs.capacity_;
	len = rhs.len;
	rhs.buffer = NULL;
	rhs.capacity_ = 0;
	rhs.len = 0;
}
#endif

String & String::operator = (const String &rhs)
{
	if (this == &rhs) return *this;
	
	if (rhs.buffer) copy(rhs.buffer, rhs.len);
	else invalidate();
	
	return *this;
}

#ifdef __GXX_EXPERIMENTAL_CXX0X__
String & String::operator = (String &&rval)
{
	if (this != &rval) move(rval);
	return *this;
}

String & String::operator = (StringSumHelper &&rval)
{
	if (this != &rval) move(rval);
	return *this;
}
#endif

String & String::operator = (const char *cstr)
{
	if (cstr) copy(cstr, strlen(cstr));
	else invalidate();

	return *this;
}


/*********************************************/
/*  concat                                   */
/*********************************************/

uint8_t String::concat(const String &s)
{
	return concat(s.buffer, s.len);
}

uint8_t String::concat(const char *cstr, size_t length)
{
	size_t newlen = len + length;
	if (!cstr) return 0;
	if (length == 0) return 1;
	if (!reserve(newlen)) return 0;
	strcpy(buffer + len, cstr);
	len = newlen;
	return 1;
}

uint8_t String::concat(const char *cstr)
{
	if (!cstr) return 0;
	return concat(cstr, strlen(cstr));
}

uint8_t String::concat(char c)
{
	char buf[2];
	buf[0] = c;
	buf[1] = 0;
	return concat(buf, 1);
}

uint8_t String::concat(uint8_t num)
{
	char buf[12];
	StringUtils::itoa(num, buf, 10);
	return concat(buf, strlen(buf));
}

uint8_t String::concat(int num)
{
	char buf[12];
	StringUtils::itoa(num, buf, 10);
	return concat(buf, strlen(buf));
}

uint8_t String::concat(size_t num)
{
	char buf[12];
	StringUtils::itoa(num, buf, 10);
	return concat(buf, strlen(buf));
}

uint8_t String::concat(long num)
{
	char buf[12];
	StringUtils::itoa(num, buf, 10);
	return concat(buf, strlen(buf));
}

uint8_t String::concat(unsigned long num)
{
	char buf[12];
	StringUtils::itoa(num, buf, 10);
	return concat(buf, strlen(buf));
}

uint8_t String::concat(float num)
{
	char buf[12];
	StringUtils::ftoa(num, buf, 6);
	return concat(buf, strlen(buf));
}

uint8_t String::concat(double num)
{
	char buf[12];
	StringUtils::ftoa(num, buf, 6);
	return concat(buf, strlen(buf));
}

/*********************************************/
/*  Concatenate                              */
/*********************************************/

StringSumHelper & operator + (const StringSumHelper &lhs, const String &rhs)
{
	StringSumHelper &a = const_cast<StringSumHelper&>(lhs);
	if (!a.concat(rhs.buffer, rhs.len)) a.invalidate();
	return a;
}

StringSumHelper & operator + (const StringSumHelper &lhs, const char *cstr)
{
	StringSumHelper &a = const_cast<StringSumHelper&>(lhs);
	if (!cstr || !a.concat(cstr, strlen(cstr))) a.invalidate();
	return a;
}

StringSumHelper & operator + (const StringSumHelper &lhs, char c)
{
	StringSumHelper &a = const_cast<StringSumHelper&>(lhs);
	if (!a.concat(c)) a.invalidate();
	return a;
}

StringSumHelper & operator + (const StringSumHelper &lhs, uint8_t num)
{
	StringSumHelper &a = const_cast<StringSumHelper&>(lhs);
	if (!a.concat(num)) a.invalidate();
	return a;
}

StringSumHelper & operator + (const StringSumHelper &lhs, int num)
{
	StringSumHelper &a = const_cast<StringSumHelper&>(lhs);
	if (!a.concat(num)) a.invalidate();
	return a;
}

StringSumHelper & operator + (const StringSumHelper &lhs, size_t num)
{
	StringSumHelper &a = const_cast<StringSumHelper&>(lhs);
	if (!a.concat(num)) a.invalidate();
	return a;
}

StringSumHelper & operator + (const StringSumHelper &lhs, long num)
{
	StringSumHelper &a = const_cast<StringSumHelper&>(lhs);
	if (!a.concat(num)) a.invalidate();
	return a;
}

StringSumHelper & operator + (const StringSumHelper &lhs, unsigned long num)
{
	StringSumHelper &a = const_cast<StringSumHelper&>(lhs);
	if (!a.concat(num)) a.invalidate();
	return a;
}

StringSumHelper & operator + (const StringSumHelper &lhs, float num)
{
	StringSumHelper &a = const_cast<StringSumHelper&>(lhs);
	if (!a.concat(num)) a.invalidate();
	return a;
}

StringSumHelper & operator + (const StringSumHelper &lhs, double num)
{
	StringSumHelper &a = const_cast<StringSumHelper&>(lhs);
	if (!a.concat(num)) a.invalidate();
	return a;
}

/*********************************************/
/*  Comparison                               */
/*********************************************/

int String::compareTo(const String &s) const
{
	if (!buffer || !s.buffer) {
		if (s.buffer && s.len > 0) return 0 - *(uint8_t *)s.buffer;
		if (buffer && len > 0) return *(uint8_t *)buffer;
		return 0;
	}
	return strcmp(buffer, s.buffer);
}

uint8_t String::equals(const String &s2) const
{
	return (len == s2.len && compareTo(s2) == 0);
}

uint8_t String::equals(const char *cstr) const
{
	if (len == 0) return (cstr == NULL || *cstr == 0);
	if (cstr == NULL) return buffer[0] == 0;
	return strcmp(buffer, cstr) == 0;
}

uint8_t String::operator<(const String &rhs) const
{
	return compareTo(rhs) < 0;
}

uint8_t String::operator>(const String &rhs) const
{
	return compareTo(rhs) > 0;
}

uint8_t String::operator<=(const String &rhs) const
{
	return compareTo(rhs) <= 0;
}

uint8_t String::operator>=(const String &rhs) const
{
	return compareTo(rhs) >= 0;
}

uint8_t String::equalsIgnoreCase( const String &s2 ) const
{
	if (this == &s2) return 1;
	if (len != s2.len) return 0;
	if (len == 0) return 1;
	const char *p1 = buffer;
	const char *p2 = s2.buffer;
	while (*p1) {
		if (tolower(*p1++) != tolower(*p2++)) return 0;
	} 
	return 1;
}

uint8_t String::startsWith( const String &s2 ) const
{
	if (len < s2.len) return 0;
	return startsWith(s2, 0);
}

uint8_t String::startsWith( const String &s2, size_t offset ) const
{
	if (offset > len - s2.len || !buffer || !s2.buffer) return 0;
	return strncmp( &buffer[offset], s2.buffer, s2.len ) == 0;
}

uint8_t String::endsWith( const String &s2 ) const
{
	if ( len < s2.len || !buffer || !s2.buffer) return 0;
	return strcmp(&buffer[len - s2.len], s2.buffer) == 0;
}

/*********************************************/
/*  Character Access                         */
/*********************************************/

char String::charAt(size_t loc) const
{
	return operator[](loc);
}

void String::setCharAt(size_t loc, char c) 
{
	if (loc < len) buffer[loc] = c;
}

char & String::operator[](size_t index)
{
	static char dummy_writable_char;
	if (index >= len || !buffer) {
		dummy_writable_char = 0;
		return dummy_writable_char;
	}
	return buffer[index];
}

char String::operator[]( size_t index ) const
{
	if (index >= len || !buffer) return 0;
	return buffer[index];
}

void String::getBytes(uint8_t *buf, size_t bufsize, size_t index) const
{
	if (!bufsize || !buf) return;
	if (index >= len) {
		buf[0] = 0;
		return;
	}
	size_t n = bufsize - 1;
	if (n > len - index) n = len - index;
	strncpy((char *)buf, buffer + index, n);
	buf[n] = 0;
}

/*********************************************/
/*  Search                                   */
/*********************************************/

int String::indexOf(char c) const
{
	return indexOf(c, 0);
}

int String::indexOf( char ch, size_t fromIndex ) const
{
	if (fromIndex >= len) return -1;
	const char* temp = strchr(buffer + fromIndex, ch);
	if (temp == NULL) return -1;
	return temp - buffer;
}

int String::indexOf(const String &s2) const
{
	return indexOf(s2, 0);
}

int String::indexOf(const String &s2, size_t fromIndex) const
{
	if (fromIndex >= len) return -1;
	const char *found = strstr(buffer + fromIndex, s2.buffer);
	if (found == NULL) return -1;
	return found - buffer;
}

int String::lastIndexOf( char theChar ) const
{
	return lastIndexOf(theChar, len - 1);
}

int String::lastIndexOf(char ch, size_t fromIndex) const
{
	if (fromIndex >= len) return -1;
	char tempchar = buffer[fromIndex + 1];
	buffer[fromIndex + 1] = '\0';
	char* temp = strrchr( buffer, ch );
	buffer[fromIndex + 1] = tempchar;
	if (temp == NULL) return -1;
	return temp - buffer;
}

int String::lastIndexOf(const String &s2) const
{
	return lastIndexOf(s2, len - s2.len);
}

int String::lastIndexOf(const String &s2, size_t fromIndex) const
{
  	if (s2.len == 0 || len == 0 || s2.len > len) return -1;
	if (fromIndex >= len) fromIndex = len - 1;
	int found = -1;
	for (char *p = buffer; p <= buffer + fromIndex; p++) {
		p = strstr(p, s2.buffer);
		if (!p) break;
		if ((size_t)(p - buffer) <= fromIndex) found = p - buffer;
	}
	return found;
}

StringView String::substring(size_t left, size_t right) const{
	return StringView(*this).substring(left, right);

}

/*********************************************/
/*  Modification                             */
/*********************************************/

void String::replace(char find, char replace)
{
	if (!buffer) return;
	for (char *p = buffer; *p; p++) {
		if (*p == find) *p = replace;
	}
}

// String& String::replace(const String& match, const String& replace)
// {
//     int dst_len = match.length();
//     if(dst_len != (int)replace.length()) return *this;
// 	StringUtils::str_replace(
//             this->c_str(),this -> length(),
//             match.c_str(),replace.c_str(),dst_len);
//     return *this;
// }

// void String::replace(const String& find, const String& replace){

// }

void String::remove(size_t index){
	// Pass the biggest integer as the count. The remove method
	// below will take care of truncating it at the end of the
	// string.
	remove(index, (size_t)-1);
}

void String::remove(size_t index, size_t count) {
    if (index >= len) { return; }
    if (count <= 0) { return; }
    if (count > len - index) { count = len - index; }
    char *writeTo = buffer + index;
    len = len - count;

    memmove(writeTo, buffer + index + count, len - index);
    buffer[len] = '\0';
}

String & String::toLowerCase(void)
{
	if (!buffer) return*this;
	for (char *p = buffer; *p; p++) {
		*p = tolower(*p);
	}
	return *this;
}

String & String::toUpperCase(void)
{
	if (!buffer) return*this;
	for (char *p = buffer; *p; p++) {
		*p = toupper(*p);
	}
	return *this;
}

std::vector<StringView> String::split(const char delimiter, const size_t times) const{
	return StringView(*this).split(delimiter, times);
}
	
void String::trim(void)
{
	if (!buffer || len == 0) return;
	char *begin = buffer;
	while (isspace(*begin)) begin++;
	char *end = buffer + len - 1;
	while (isspace(*end) && end >= begin) end--;
	len = end + 1 - begin;
	if (begin > buffer) memcpy(buffer, begin, len);
	buffer[len] = 0;
}

void String::alphanum(void){
	char * new_buf = (char *)malloc(len);

	size_t len0 = len;
	len = 0;
	for(size_t i = 0; i < len0; i++){
		char chr = buffer[i];
		if(StringUtils::is_alpha(chr) || StringUtils::is_digit(chr) || chr == ' ' || chr == '+' || chr == '-' || chr == '.'){
			new_buf[len++] = chr;
		}
	}

	memcpy(buffer, new_buf, len);
	buffer[len] = 0;
	free(new_buf);
}

bool String::isNumeric(void) const {return StringUtils::is_numeric(c_str(), len);}
bool String::isDigit(void) const {return StringUtils::is_digit(c_str(), len);}
