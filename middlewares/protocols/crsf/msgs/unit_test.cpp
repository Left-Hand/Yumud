#include "crsf_extended_msgs.hpp"

using namespace ymd;
using namespace ymd::crsf;

// https://www.zhihu.com/question/4436519767/answer/34255755305

namespace {

// [[maybe_unused]] void test_ser_msg(){
//     {
//         [[maybe_unused]] static constexpr auto msg = msgs::ParameterSettingsRead{
//             .parameter_number = 0x11,
//             .parameter_chunk_number = 0x45
//         };

//         auto fn = []{
//             std::array<uint8_t, 5> bytes{0xff, 0xff, 0xff, 0xff, 0xff};
//             auto receiver = SerializeReceiver{std::span(bytes), 0};
//             msg.sink_to(receiver).unwrap();
//             return std::make_tuple(bytes, receiver);
//         };

//         static constexpr auto bytes = std::get<0>(fn());
//         static_assert(bytes[0] == 0x11);
//         static_assert(bytes[1] == 0x45);
//         static_assert(bytes[2] == 0xff);
//         static_assert(bytes[3] == 0xff);
//         static_assert(bytes[4] == 0xff);

//     }
// }


template <typename T>
constexpr std::size_t const_context() {
    return T{}.size();
}

template <typename T, std::size_t = const_context<T>()>
constexpr bool is_constexpr_impl(T&&) {
    return true;
}
constexpr bool is_constexpr_impl(...) { return false; }

template <typename T>
concept is_constexpr_size = is_constexpr_impl(T{});

struct A {
    size_t value;
    // constexpr std::size_t size() { return 10; }
    constexpr std::size_t size() { return value; }
};

struct B { std::size_t size() { return 10; }};
struct C { static constexpr std::size_t size() { return 10; }};
// static constexpr size_t a_size = std::declval<A>().size();
// static constexpr size_t b_size = std::declval<B>().size();
// static constexpr size_t c_size = std::declval<C>().size();

static_assert(is_constexpr_size<A> == true); // true
static_assert(is_constexpr_size<B> == false); // false

}