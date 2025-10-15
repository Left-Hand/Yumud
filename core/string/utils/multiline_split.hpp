#pragma once

#include "core/string/string_view.hpp"
#include "core/utils/scope_guard.hpp"


namespace ymd::strconv2{


struct StringSplitIter {
    constexpr explicit StringSplitIter(const StringView text, const char delimiter)
        : str_(text), 
        delimiter_(delimiter)
    {
        seek_next();
    }

    constexpr bool has_next() const {
        return pos_ < str_.size();
    }

    constexpr Option<StringView> next() {
        auto guard = make_scope_guard([&]{
            pos_ = end_ + 1; // Skip the newline character
            seek_next();
        });
        return str_.substr_by_len(pos_, end_ - pos_);
    }

private:
    constexpr void seek_next() {
        end_ = pos_;
        while (end_ < str_.size() && str_[end_] != delimiter_) {
            ++end_;
        }
    }

    const StringView str_;
    const char delimiter_;
    size_t pos_   = 0;  // Start of current line
    size_t end_     = 0;    // End of current line (points to '\n' or str_.size())
};


#if 0
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
        if (!test_string_view_equal(single_iter.next().unwrap(), "hello")) return false;
        if (single_iter.has_next()) return false;
        return true;
    }(), "Single line test failed");

    static_assert([&]()  {
        // Test multi-line string
        StringView multi = "line1\nline2\nline3";
        StringSplitIter multi_iter(multi, '\n');
        
        if (!multi_iter.has_next()) return false;
        if (!test_string_view_equal(multi_iter.next().unwrap(), "line1")) return false;
        
        if (!multi_iter.has_next()) return false;
        if (!test_string_view_equal(multi_iter.next().unwrap(), "line2")) return false;
        
        if (!multi_iter.has_next()) return false;
        if (!test_string_view_equal(multi_iter.next().unwrap(), "line3")) return false;
        
        if (multi_iter.has_next()) return false;
        return true;
    }(), "Multi-line test failed");

    static_assert([&]()  {
        // Test empty lines
        StringView with_empty = "\nline\n\n";
        StringSplitIter iter(with_empty, '\n');
        
        if (!iter.has_next()) return false;
        if (!test_string_view_equal(iter.next().unwrap(), "")) return false;
        
        if (!iter.has_next()) return false;
        if (!test_string_view_equal(iter.next().unwrap(), "line")) return false;
        
        if (!iter.has_next()) return false;
        if (!test_string_view_equal(iter.next().unwrap(), "")) return false;
        
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
        if (trimmed.length() != 0) return false;
        return true;
    }(), "Trim test failed");

    static_assert([&]()  {
        // Test line iterator with trimming
        StringView text = "  line1  \n  line2  \n";
        StringSplitIter iter(text, '\n');
        
        if (!iter.has_next()) return false;
        StringView line1 = iter.next().unwrap().trim();
        if (!test_string_view_equal(line1, "line1")) return false;
        
        if (!iter.has_next()) return false;
        StringView line2 = iter.next().unwrap().trim();
        if (!test_string_view_equal(line2, "line2")) return false;
        
        if (iter.has_next()) return false;
        return true;
    }(), "Iterator with trimming test failed");
}
#endif

}