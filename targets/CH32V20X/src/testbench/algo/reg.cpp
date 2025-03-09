#include "../tb.h"

#include "sys/utils/typetraits/size_traits.hpp"
#include "sys/utils/BitField.hpp"
#include "sys/utils/Reg.hpp"

using namespace ymd;



// template<typename T>
// struct reg_data_type<Reg>{using type = T;};


// template<typename T>
// struct BitFieldCrtp{
//     __inline constexpr
//     T as_mask() const{
//         return static_cast<T *>(this)->as_mask();
//     }

//     __inline constexpr 
//     T as_unshifted() const{
//         return static_cast<T *>(this)->as_unshifted();
//     }
//     __inline constexpr 
//     T as_val() const{
//         return static_cast<T *>(this)->as_unshifted();
//     }
// };

// template<typename TDummy>
// struct BitField{};





// template<auto ... Args>
// using BitField_u16 = BitField<const uint16_t, Args...>;

// template<auto ... Args>
// using BitField_mu16 = BitField<uint16_t, Args...>;




// static_assert(std::is_same_v<uint16_t, reg_data_type_t<uint16_t>>, "reg_data_type_t is not uint16_t");

// template<size_t b_bits, size_t e_bits, typename T>
// __inline static constexpr
// auto make_mut_bitfield(T * obj){
//     // using T = std::remove_pointer_t<decltype(obj)>;
//     using D = typename std::remove_const_t<reg_data_type_t<T>>;
    
//     return BitField<D, b_bits, e_bits>(*reinterpret_cast<D *>(obj));
// }


// template<size_t b_bits, size_t e_bits, typename I>
// BitField(auto && data) -> BitField<decltype(data), b_bits, e_bits, I>;




// template<size_t b_bits, size_t e_bits, typename D, typename I = D>
// __inline static constexpr
// auto make_mut_bitfield(D & data){
//     return BitField<D, b_bits, e_bits, I>(data);
// }



// template<size_t b_bits, size_t e_bits>
// __inline static constexpr
// auto make_bitfield(auto * obj){
//     using T = std::remove_pointer_t<decltype(obj)>;
//     using reg_data_type = std::conditional_t<
//         std::is_const_v<T>,
//         const reg_data_type_t<T>,
//         reg_data_type_t<T>
//     >;
    
//     return BitField<reg_data_type, b_bits, e_bits>(reinterpret_cast<reg_data_type &>(*obj));
// }

enum class Num:uint8_t{
    _0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16
};

struct R16_Temp:public RegBase<uint16_t>{
    using Super = RegBase<uint16_t>;
    constexpr R16_Temp(uint16_t val) : Super(val) {} 
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

    static_assert(mask_of<uint16_t>(8, 16) == 0xFF00);

    {
        // static constexpr uint16_t temp = 0xFF00;
        // static constexpr BitField_u16<8, 10> bf1(temp);

        // static_assert(bf1.len() == 2, "bitfield length error");
        // static_assert(bf1.as_val() == 0b11, "bitfield data error");

    }
    


    {

        static constexpr R16_Temp temp = R16_Temp{0x1234};
        static constexpr auto bf1 = temp.data1();
        // static constexpr auto bf2 = temp.data2();
        static constexpr auto arr1 = temp.arr1();
        static constexpr auto en1 = temp.enum1();

        static_assert(bf1.len() == 2, "bitfield length error");
        static_assert(bf1.as_val() == 0x02, "bitfield data error");
        static_assert(sizeof(temp) == 0x02, "reg size error");

        static_assert(en1.as_val() == Num::_2, "reg size error");

        static_assert(std::is_const_v<bitfield_data_type_t<decltype(bf1)>>, "const bitfield data is mutable");
        // static_assert(!std::is_const_v<bitfield_data_type_t<decltype(bf2)>>, "mutable bitfield data is const");
        
        static_assert(arr1.get<0>().as_val() == 0x04, "bitfield data error");
        static_assert(arr1.get<1>().as_val() == 0x03, "bitfield data error");
        static_assert(arr1.get<2>().as_val() == 0x02, "bitfield data error");
        static_assert(arr1.get<3>().as_val() == 0x01, "bitfield data error");

        static_assert(arr1[0].as_val() == 0x04, "bitfield data error");
        static_assert(arr1[1].as_val() == 0x03, "bitfield data error");
        static_assert(arr1[2].as_val() == 0x02, "bitfield data error");
        static_assert(arr1[3].as_val() == 0x01, "bitfield data error");
    }

    {

            R16_Temp temp = R16_Temp{0x1234};
            auto en2 = temp.enum2();

            en2 = Num::_0;
            // en2 = 0;
    }
}

