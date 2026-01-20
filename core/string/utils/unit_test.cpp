#include "c_style/strnlen.hpp"
#include "split_iter.hpp"

using namespace ymd;


namespace {
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

static_assert(strnlen_from_right("", 0) == 0);
static_assert(strnlen_from_right("abc", 4) == 3);
static_assert(strnlen_from_right("abc", 3) == 3);
static_assert(strnlen_from_right("abc", 2) == 2);

}
