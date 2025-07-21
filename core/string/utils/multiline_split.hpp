#pragma once

#include "core/string/string_view.hpp"

namespace ymd::strconv2{


static constexpr StringView trim_str(const StringView str){
    auto is_whitespace = [](char c) {
        return c == ' ' || c == '\t' || c == '\n' || c == '\r';
    };

    if (str.is_empty()) {
        return str;
    }

    // Find first non-whitespace character
    size_t start = 0;
    while (start < str.size() && is_whitespace(str[start])) {
        ++start;
    }

    // Find last non-whitespace character
    size_t end = str.size();
    while (end > start && is_whitespace(str[end - 1])) {
        --end;
    }

    return str.substr_by_len(start, end - start);

}


struct MultiLineIter {
    constexpr explicit MultiLineIter(const StringView text)
        : text_(text), 
            start_(0),
            end_(0) 
    {
        seek_next_line();
    }

    constexpr bool has_next() const {
        return start_ < text_.size();
    }

    constexpr StringView next() {
        if (!has_next()) {
            return StringView(nullptr, 0); // or throw/return empty view
        }

        const StringView line = text_.substr_by_len(start_, end_ - start_);
        start_ = end_ + 1; // Skip the newline character
        seek_next_line();
        return trim_str(line);
    }

private:
    constexpr void seek_next_line() {
        end_ = start_;
        while (end_ < text_.size() && text_[end_] != '\n') {
            ++end_;
        }
    }



    const StringView text_;
    size_t start_;  // Start of current line
    size_t end_;    // End of current line (points to '\n' or text_.size())
};

[[maybe_unused]] static void static_test_multi_line_iter(){
// Test helper function
    auto test_string_view_equal = [](StringView a, StringView b) -> bool{
        if (a.size() != b.size()) return false;
        for (size_t i = 0; i < a.size(); ++i) {
            if (a[i] != b[i]) return false;
        }
        return true;
    };

    // Test cases
    static_assert([&]()  {
        // Test empty string
        StringView empty = StringView("");
        MultiLineIter empty_iter(empty);
        if (empty_iter.has_next()) return false;
        return true;
    }(), "Empty string test failed");

    static_assert([&]()  {
        // Test single line
        StringView single = "hello";
        MultiLineIter single_iter(single);
        if (!single_iter.has_next()) return false;
        if (!test_string_view_equal(single_iter.next(), "hello")) return false;
        if (single_iter.has_next()) return false;
        return true;
    }(), "Single line test failed");

    static_assert([&]()  {
        // Test multi-line string
        StringView multi = "line1\nline2\nline3";
        MultiLineIter multi_iter(multi);
        
        if (!multi_iter.has_next()) return false;
        if (!test_string_view_equal(multi_iter.next(), "line1")) return false;
        
        if (!multi_iter.has_next()) return false;
        if (!test_string_view_equal(multi_iter.next(), "line2")) return false;
        
        if (!multi_iter.has_next()) return false;
        if (!test_string_view_equal(multi_iter.next(), "line3")) return false;
        
        if (multi_iter.has_next()) return false;
        return true;
    }(), "Multi-line test failed");

    static_assert([&]()  {
        // Test empty lines
        StringView with_empty = "\nline\n\n";
        MultiLineIter iter(with_empty);
        
        if (!iter.has_next()) return false;
        if (!test_string_view_equal(iter.next(), "")) return false;
        
        if (!iter.has_next()) return false;
        if (!test_string_view_equal(iter.next(), "line")) return false;
        
        if (!iter.has_next()) return false;
        if (!test_string_view_equal(iter.next(), "")) return false;
        
        if (iter.has_next()) return false;
        return true;
    }(), "Empty lines test failed");

    static_assert([&]()  {
        // Test trimming
        StringView to_trim = "  hello \t\n";
        StringView trimmed = trim_str(to_trim);
        if (!test_string_view_equal(trimmed, "hello")) return false;
        
        StringView all_ws = " \t\r\n ";
        trimmed = trim_str(all_ws);
        if (!trimmed.is_empty()) return false;
        return true;
    }(), "Trim test failed");

    static_assert([&]()  {
        // Test line iterator with trimming
        StringView text = "  line1  \n  line2  \n";
        MultiLineIter iter(text);
        
        if (!iter.has_next()) return false;
        StringView line1 = trim_str(iter.next());
        if (!test_string_view_equal(line1, "line1")) return false;
        
        if (!iter.has_next()) return false;
        StringView line2 = trim_str(iter.next());
        if (!test_string_view_equal(line2, "line2")) return false;
        
        if (iter.has_next()) return false;
        return true;
    }(), "Iterator with trimming test failed");
}

}