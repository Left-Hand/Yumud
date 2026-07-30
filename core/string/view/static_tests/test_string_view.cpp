#include "../string_view.hpp"


using namespace ymd;

namespace {

[[maybe_unused]]static void test_stringview_trim(){
    {
        static constexpr auto str = StringView("  hello world   \n");
        static constexpr auto trimmed = str.trim();
        static_assert(trimmed.equals("hello world"));
    }

    {
        static constexpr auto str = StringView("  hello world   \n");
        static constexpr auto trimmed = str.trim_withfn([](const char c){
            switch(c){
                case ' ':
                case '\r':
                case '\n':
                case '\t':
                case 'h':
                case 'd':
                    return true;
                default:
                    return false;
            }
        });
        static_assert(trimmed.equals("ello worl"));
    }
}
}