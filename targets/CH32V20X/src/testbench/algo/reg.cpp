#include "../tb.h"

#include "core/magic/size_traits.hpp"
#include "core/utils/bits/BitField.hpp"

using namespace ymd;

#if 0

enum class Num:uint8_t{
    _0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16
};

struct R16_Temp{
    // using Super = RegBase<uint16_t>;
    // constexpr R16_Temp(uint16_t val) : Super(val) {} 
    __inline constexpr
    auto data1() const { return make_bitfield<8, 10>(data); };

    __inline constexpr
    auto data2() { return make_bitfield<8, 10>(data); };

    __inline constexpr
    auto enum1() const { return make_bitfield<8, 12, Num>(data); };

    __inline constexpr
    auto enum2() { return make_bitfield<8, 12, Num>(data); };

    __inline constexpr
    auto arr1() const { return make_bfarray<0,16,4>(data); };
};


void test() {

    static_assert(magic::mask_of<uint16_t>(8, 16) == 0xFF00);

    {
        // static constexpr uint16_t temp = 0xFF00;
        // static constexpr BitField_u16<8, 10> bf1(temp);

        // static_assert(bf1.len() == 2, "bitfield length error");
        // static_assert(bf1.to_bits() == 0b11, "bitfield data error");

    }
    


    {

        static constexpr R16_Temp temp = R16_Temp{0x1234};
        static constexpr auto bf1 = temp.data1();
        // static constexpr auto bf2 = temp.data2();
        static constexpr auto arr1 = temp.arr1();
        static constexpr auto en1 = temp.enum1();

        static_assert(bf1.len() == 2, "bitfield length error");
        static_assert(bf1.to_bits() == 0x02, "bitfield data error");
        static_assert(sizeof(temp) == 0x02, "reg size error");

        static_assert(en1.to_bits() == Num::_2, "reg size error");

        static_assert(std::is_const_v<bitfield_data_type_t<decltype(bf1)>>, "const bitfield data is mutable");
        // static_assert(!std::is_const_v<bitfield_data_type_t<decltype(bf2)>>, "mutable bitfield data is const");
        
        static_assert(arr1.get<0>().to_bits() == 0x04, "bitfield data error");
        static_assert(arr1.get<1>().to_bits() == 0x03, "bitfield data error");
        static_assert(arr1.get<2>().to_bits() == 0x02, "bitfield data error");
        static_assert(arr1.get<3>().to_bits() == 0x01, "bitfield data error");

        static_assert(arr1[0].to_bits() == 0x04, "bitfield data error");
        static_assert(arr1[1].to_bits() == 0x03, "bitfield data error");
        static_assert(arr1[2].to_bits() == 0x02, "bitfield data error");
        static_assert(arr1[3].to_bits() == 0x01, "bitfield data error");
    }

    {

            R16_Temp temp = R16_Temp{0x1234};
            auto en2 = temp.enum2();

            en2 = Num::_0;
            // en2 = 0;
    }
}

#endif