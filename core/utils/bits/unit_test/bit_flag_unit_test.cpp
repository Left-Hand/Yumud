#include "../bitflag.hpp"
#include <type_traits>
#include "../bits_caster.hpp"

using namespace ymd;
// enum class Fault:uint8_t{
//     OverCurrent,
//     OverVoltage,
//     OverTemperature,
//     UnderTemperature
// };


namespace {
enum class MyFlagBit { A, B, C };

using MyEnumRearranger = EnumRearranger<MyFlagBit, MyFlagBit::A, MyFlagBit::B, MyFlagBit::C>;

static_assert(MyEnumRearranger::length() == 3);
static_assert(MyEnumRearranger::enum_to_rank_v<MyFlagBit::B> == 1);
static_assert(MyEnumRearranger::rank_to_enum_v<1> == MyFlagBit::B);

using MyBitFlag = BitFlag<MyEnumRearranger>;

static_assert(MyBitFlag::NUM_BITS == 3);
static_assert(MyBitFlag::enum_to_mask(MyFlagBit::A)  == 1 << 0);
static_assert(MyBitFlag::enum_to_mask(MyFlagBit::B)  == 1 << 1);
static_assert(MyBitFlag::Iterator(
    MyBitFlag::enum_to_mask(MyFlagBit::A) |
    MyBitFlag::enum_to_mask(MyFlagBit::B)
).count_ones()  == 2);

}