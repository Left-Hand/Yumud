#pragma once

#include "core/string/string_view.hpp"
namespace ymd{

class GBKIterator final{

public:
	constexpr GBKIterator(const StringView str) : 
		str_(str), 
		current_index_(0) {}
	
	constexpr bool has_next() const {
		return str_[current_index_] != '\0' and str_.size() > current_index_;
	}
	
	constexpr wchar_t next() {

		auto ch = str_[unsigned(current_index_)];
		int unicodeValue;
		
		if (ch < 0x80) {
			unicodeValue = ch;
		} else {
			unicodeValue = ((str_[current_index_] & 0xFF) << 8) | (str_[current_index_ + 1] & 0xFF);
			++current_index_;
		}
		
		++current_index_;
		return unicodeValue;
	}

private:
	// const char* str_;
	
	StringView str_;
	size_t current_index_;
};

}