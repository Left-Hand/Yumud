#include "../thrifty_string.hpp"


using namespace ymd;

namespace {
using ThriftyString8 = ThriftyInlineString<8>;
// static constexpr auto CSTRING_HELLO = "hel";
// static constexpr auto STRING_HELLO = ThriftyString8::try_from_cstr(CSTRING_HELLO).unwrap();
static constexpr auto STRING_HELLO = ThriftyString8::try_from_sv("hello").unwrap();
static_assert(STRING_HELLO.view()[0] == 'h');
static_assert(STRING_HELLO.view()[1] == 'e');
static_assert(STRING_HELLO.view()[2] == 'l');
static_assert(STRING_HELLO.view()[3] == 'l');
static_assert(STRING_HELLO.view()[4] == 'o');
}

