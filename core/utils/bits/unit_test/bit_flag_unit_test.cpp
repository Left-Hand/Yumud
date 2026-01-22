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

using MyEnumList = EnumList<MyFlagBit, MyFlagBit::A, MyFlagBit::B, MyFlagBit::C>;

static_assert(MyEnumList::length() == 3);
static_assert(MyEnumList::enum_to_rank_v<MyFlagBit::B> == 1);
static_assert(MyEnumList::rank_to_enum_v<1> == MyFlagBit::B);

using MyBitFlag = BitFlag<MyEnumList>;

static_assert(MyBitFlag::NUM_BITS == 3);
// static_assert(MyBitFlag::from_enums({MyFlagBit::A, MyFlagBit::B}).begin().width() == 2);
static_assert(MyBitFlag::enum_to_mask(MyFlagBit::A)  == 1 << 0);
static_assert(MyBitFlag::enum_to_mask(MyFlagBit::B)  == 1 << 1);
static_assert(MyBitFlag::Iterator(
    MyBitFlag::enum_to_mask(MyFlagBit::A) |
    MyBitFlag::enum_to_mask(MyFlagBit::B)
).ones()  == 2);

}