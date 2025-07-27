#pragma once

#include "core/string/string_view.hpp"
namespace ymd{

class AsciiIterator final{

public:
	constexpr AsciiIterator(const StringView str) : 
		str_(str), 
		current_index_(0) {}
	
	constexpr bool has_next() const {
		return str_[current_index_] != '\0' and str_.size() > current_index_;
	}
	
	constexpr wchar_t next() {
        return str_[current_index_++];
	}

private:
	// const char* str_;
	
	StringView str_;
	size_t current_index_;
};

}