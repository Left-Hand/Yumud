#pragma once

#include "core/string/string_view.hpp"

namespace ymd::strconv2{


struct StringSplitIter {
    constexpr explicit StringSplitIter(const StringView text, const char delimiter)
        : text_(text), 
        delimiter_(delimiter){
        seek_next_line();
    }

    constexpr bool has_next() const {
        return start_ < text_.size();
    }

    constexpr StringView next() {
        const StringView line = text_.substr_by_len(start_, end_ - start_);
        start_ = end_ + 1; // Skip the newline character
        seek_next_line();
        return line;
    }

private:
    constexpr void seek_next_line() {
        end_ = start_;
        while (end_ < text_.size() && text_[end_] != '\n') {
            ++end_;
        }
    }



    const StringView text_;
    const char delimiter_;
    size_t start_   = 0;  // Start of current line
    size_t end_     = 0;    // End of current line (points to '\n' or text_.size())
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
        StringSplitIter empty_iter(empty, '\n');
        if (empty_iter.has_next()) return false;
        return true;
    }(), "Empty string test failed");

    static_assert([&]()  {
        // Test single line
        StringView single = "hello";
        StringSplitIter single_iter(single, '\n');
        if (!single_iter.has_next()) return false;
        if (!test_string_view_equal(single_iter.next(), "hello")) return false;
        if (single_iter.has_next()) return false;
        return true;
    }(), "Single line test failed");

    static_assert([&]()  {
        // Test multi-line string
        StringView multi = "line1\nline2\nline3";
        StringSplitIter multi_iter(multi, '\n');
        
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
        StringSplitIter iter(with_empty, '\n');
        
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
        StringView trimmed = to_trim.trim();
        if (!test_string_view_equal(trimmed, "hello")) return false;
        
        StringView all_ws = " \t\r\n ";
        trimmed = all_ws.trim();
        if (!trimmed.is_empty()) return false;
        return true;
    }(), "Trim test failed");

    static_assert([&]()  {
        // Test line iterator with trimming
        StringView text = "  line1  \n  line2  \n";
        StringSplitIter iter(text, '\n');
        
        if (!iter.has_next()) return false;
        StringView line1 = iter.next().trim();
        if (!test_string_view_equal(line1, "line1")) return false;
        
        if (!iter.has_next()) return false;
        StringView line2 = iter.next().trim();
        if (!test_string_view_equal(line2, "line2")) return false;
        
        if (iter.has_next()) return false;
        return true;
    }(), "Iterator with trimming test failed");
}

struct LineText{
    explicit LineText(const StringView text):text_(text){}
    
    constexpr StringView text(){
        return text_;
    };
private:
    StringView text_;

};

struct MultiLineText{
    Option<StringView> get_line(const size_t line_nth) const {
        //TODO
        return None;
    }
private:
};

}