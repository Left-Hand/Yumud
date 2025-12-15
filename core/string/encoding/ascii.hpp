#pragma once

#include "core/string/string_view.hpp"
namespace ymd{

class [[nodiscard]] AsciiIterator final{

public:
	constexpr AsciiIterator(const StringView str) : 
		str_(str), 
		idx_(0) {}
	
	[[nodiscard]] constexpr bool has_next() const {
		return str_[idx_] != '\0' and str_.size() > idx_;
	}
	
	[[nodiscard]] constexpr wchar_t next() {
        return str_[idx_++];
	}

private:
	StringView str_;
	size_t idx_;
};

}