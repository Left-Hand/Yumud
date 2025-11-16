#pragma once

#include "core/math/uint24_t.hpp"
#include "core/constants/enums.hpp"
#include <span>

namespace ymd{

template<typename T> 
struct RegCopy final:public T{
public:
    using value_type = typename T::value_type;

	constexpr RegCopy(T & owner)
		:owner_(owner){
        T & self = *this;
        self.as_mut_bits() = owner_.as_bits();
    }

    constexpr void apply() const {
        const T & self = *this;
        owner_.as_mut_bits() = self.as_bits();
    }

    // constexpr ~RegCopy(){
    //     apply();
    // }
private:
	T & owner_;
};

template<typename T>
RegCopy(T) -> RegCopy<T>;


struct __RegBase{
    template<typename T>
    friend class RegCopy;
};



template<typename T, typename D = T>
struct __Reg_t:public __RegBase{
public:
    using underly_type = T;
    using value_type = T;
    
    [[nodiscard]] constexpr std::span<uint8_t> as_mut_bytes() {
        return std::span<uint8_t>(reinterpret_cast<uint8_t *>(this), sizeof(T));}
    [[nodiscard]] constexpr std::span<const uint8_t> as_bytes() const {
        return std::span<const uint8_t>(reinterpret_cast<const uint8_t *>(this), sizeof(T));}
    
    constexpr __Reg_t<T> & set_bits(T bits) {
        this->as_mut_bits() = static_cast<T>(this->as_bits() | static_cast<T>(bits)); 
        return *this;
    }
    constexpr __Reg_t<T> & clr_bits(T bits) {
        this->as_mut_bits() = static_cast<T>(this->as_bits() & ~static_cast<T>(bits)); 
        return *this;
    }
    constexpr __Reg_t<T> & reconf_bits(T bits) {
        this->as_mut_bits() = static_cast<T>(static_cast<T>(bits)); 
        return *this;
    }
    
    [[nodiscard]] constexpr T & as_mut_bits()
    requires (std::is_const_v<T> == false)
    {
        return (reinterpret_cast<T &>(*this));
    }


    [[nodiscard]] constexpr T as_bits() const 
    {return (reinterpret_cast<const T &>(*this));}

};

#define DEF_REG_TEMPLATE(name, T, as_fn)\
template<typename D = T>\
struct name:public __Reg_t<T, D>{\
constexpr T as_fn() const {return std::bit_cast<T>(this->as_bits());}\
};\

#define DEF_REGC_TEMPLATE(name, T, as_fn)\
template<typename D = T>\
struct name:public __Reg_t<const T, D>{\
constexpr T as_fn() const {return T(*this);}\
};\


DEF_REG_TEMPLATE(Reg8, uint8_t, as_u8)
DEF_REG_TEMPLATE(Reg16, uint16_t, as_u16)
DEF_REG_TEMPLATE(Reg24, uint24_t, as_u24)
DEF_REG_TEMPLATE(Reg32, uint32_t, as_u32)
DEF_REG_TEMPLATE(Reg64, uint64_t, as_u64)

DEF_REG_TEMPLATE(Reg8i, int8_t, as_i8)
DEF_REG_TEMPLATE(Reg16i, int16_t, as_i16)
DEF_REG_TEMPLATE(Reg32i, int32_t, as_i32)
DEF_REG_TEMPLATE(Reg64i, int64_t, as_i64)

DEF_REGC_TEMPLATE(RegC8, uint8_t, as_u8)
DEF_REGC_TEMPLATE(RegC16, uint16_t, as_u16)
DEF_REGC_TEMPLATE(RegC24, uint24_t, as_u24)
DEF_REGC_TEMPLATE(RegC32, uint32_t, as_u32)
DEF_REGC_TEMPLATE(RegC64, uint64_t, as_u64)

DEF_REGC_TEMPLATE(RegC8i, int8_t, as_i8)
DEF_REGC_TEMPLATE(RegC16i, int16_t, as_i16)
DEF_REGC_TEMPLATE(RegC32i, int32_t, as_i32)
DEF_REGC_TEMPLATE(RegC64i, int64_t, as_i64)

#undef DEF_REG_TEMPLATE
#undef DEF_REGC_TEMPLATE


#define CHECK_R32(type)\
static_assert(sizeof(type) == 4, "x must be 32bit register");\
static_assert(std::has_unique_object_representations_v<type>, "x must has unique bitfield");\

#define CHECK_R16(type)\
static_assert(sizeof(type) == 2, "x must be 16bit register");\
static_assert(std::has_unique_object_representations_v<type>, "x must has unique bitfield");\

#define CHECK_R8(type)\
static_assert(sizeof(type) == 1, "x must be 8bit register");\
static_assert(std::has_unique_object_representations_v<type>, "x must has unique bitfield");\


#define DEF_R32(name)\
name{};\
CHECK_R32(std::decay_t<decltype(name)>)\


#define DEF_R16(name)\
name{};\
CHECK_R16(std::decay_t<decltype(name)>)\

#define DEF_R8(name)\
name{};\
CHECK_R8(std::decay_t<decltype(name)>)\

#define REG16I_QUICK_DEF(addr, type, name)\
struct type :public Reg16i<>{static constexpr uint8_t address = addr; int16_t data;} DEF_R16(name)

#define REG16_QUICK_DEF(addr, type, name)\
struct type :public Reg16<>{static constexpr RegAddr ADDRESS = addr; uint16_t data;} DEF_R16(name)

#define REG8_QUICK_DEF(addr, type, name)\
struct type :public Reg8<>{static constexpr RegAddr ADDRESS = addr; uint8_t data;} DEF_R8(name)




template<typename T>
struct reg_decay{
    // using type = void;
};

template<typename T>
requires std::is_base_of_v<__RegBase, T>
struct reg_decay<T>{
    using type = T::value_type;
};


template<typename T>
using reg_decay_t = typename reg_decay<T>::type;

}
