#pragma once

namespace ymd{

class GBKIterator {
private:
	const char* gbkString;
	int currentIndex;
	
public:
	GBKIterator(const char* _gbkString) : gbkString(_gbkString), currentIndex(0) {}
	
	operator bool() const {
		return gbkString[currentIndex] != '\0';
	}
	
	int next() {
		if (this->operator bool() == false) {
			return -1;
		}
		
		auto ch = gbkString[unsigned(currentIndex)];
		int unicodeValue;
		
		if (ch < 0x80) {
			unicodeValue = ch;
		} else {
			unicodeValue = ((gbkString[currentIndex] & 0xFF) << 8) | (gbkString[currentIndex + 1] & 0xFF);
			++currentIndex;
		}
		
		++currentIndex;
		return unicodeValue;
	}
};

}