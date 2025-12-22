#include "accmulation_builder.hpp"

using namespace ymd::math;
namespace{

[[maybe_unused]]void test1(){
    constexpr AccumulationBuilder<int16_t> builder(4);
    static_assert(builder.add(1).get() == 5);
    static_assert(builder.add(256).try_get<uint8_t>().unwrap_err() == std::strong_ordering::greater);
    static_assert(builder.add(-256).try_get<uint8_t>().unwrap_err() == std::strong_ordering::less);
}
}