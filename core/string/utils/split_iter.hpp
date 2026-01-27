#pragma once

#include "core/string/view/string_view.hpp"
#include "core/utils/scope_guard.hpp"
#include <ranges>


namespace ymd{

struct StringSplitIter {
    constexpr explicit StringSplitIter(
        const StringView str, 
        const char delimiter
    ): 
        str_(str), 
        delimiter_(delimiter)
    {
        seek_next();
    }

    [[nodiscard]] constexpr bool has_next() const {
        return pos_ < str_.size();
    }

    constexpr StringView next() {
        auto guard = make_scope_guard([&]{
            pos_ = end_ + 1; // Skip the delimiter character
            seek_next();
        });
        return StringView(str_.data() + pos_, end_ - pos_);
    }

private:
    constexpr void seek_next() {
        // Skip consecutive delimiters
        while (pos_ < str_.size() && str_[pos_] == delimiter_) {
            ++pos_;
        }
        
        if (pos_ >= str_.size()) {
            // If we've reached the end after skipping delimiters
            end_ = str_.size();
            return;
        }
        
        // Find the next delimiter
        end_ = pos_;
        while (end_ < str_.size() && str_[end_] != delimiter_) {
            ++end_;
        }
    }

    const StringView str_;
    const char delimiter_;
    size_t pos_ = 0;  // Start of current token
    size_t end_ = 0;  // End of current token (points to delimiter or str_.size())
};
}