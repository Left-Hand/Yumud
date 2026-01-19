#pragma once

#include "core/int/uint24_t.hpp"
#include "core/constants/enums.hpp"
#include <span>

namespace ymd{

template<typename T> 
struct [[nodiscard]] RegCopy final:public T{
public:
    using value_type = typename T::value_type;

	constexpr RegCopy(T & owner)
		:owner_(owner){
        T & self = *this;
        self.as_bits_mut() = owner_.to_bits();
    }

    constexpr void apply() const {
        const T & self = *this;
        owner_.as_bits_mut() = self.to_bits();
    }

    // constexpr ~RegCopy(){
    //     apply();
    // }
private:
	T & owner_;
};

template<typename T>
RegCopy(T) -> RegCopy<T>;


template<typename T, typename D = T>
struct [[nodiscard]] RegBase{
public:
    using underly_type = T;
    using value_type = T;
    
    [[nodiscard]] constexpr std::span<uint8_t> as_bytes_mut() {
        return std::span<uint8_t>(reinterpret_cast<uint8_t *>(this), sizeof(T));}
    [[nodiscard]] constexpr std::span<const uint8_t> as_bytes() const {
        return std::span<const uint8_t>(reinterpret_cast<const uint8_t *>(this), sizeof(T));}
    
    constexpr RegBase<T> & set_bits(T bits) {
        this->as_bits_mut() = static_cast<T>(this->to_bits() | static_cast<T>(bits)); 
        return *this;
    }
    constexpr RegBase<T> & unset_bits(T bits) {
        this->as_bits_mut() = static_cast<T>(this->to_bits() & ~static_cast<T>(bits)); 
        return *this;
    }
    constexpr RegBase<T> & write_bits(T bits) {
        this->as_bits_mut() = static_cast<T>(static_cast<T>(bits)); 
        return *this;
    }
    
    [[nodiscard]] constexpr T & as_bits_mut()
    requires (std::is_const_v<T> == false)
    {
        return (reinterpret_cast<T &>(*this));
    }

    [[nodiscard]] constexpr T to_bits() const 
    {return (reinterpret_cast<const T &>(*this));}

    template<typename TOther>
    friend class RegCopy;
};

#define DEF_REG_TEMPLATE(name, T, as_fn)\
template<typename D = T>\
struct [[nodiscard]] name:public RegBase<T, D>{\
constexpr T as_fn() const {return std::bit_cast<T>(this->to_bits());}\
};\

#define DEF_REGC_TEMPLATE(name, T, as_fn)\
template<typename D = T>\
struct [[nodiscard]] name:public RegBase<const T, D>{\
constexpr T as_fn() const {return T(*this);}\
};\


DEF_REG_TEMPLATE(Reg8, uint8_t, to_u8)
DEF_REG_TEMPLATE(Reg16, uint16_t, to_u16)
DEF_REG_TEMPLATE(Reg24, math::uint24_t, as_u24)
DEF_REG_TEMPLATE(Reg32, uint32_t, to_u32)
DEF_REG_TEMPLATE(Reg64, uint64_t, to_u64)

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

#define REG16_QUICK_DEF(addr, type, name)\
struct type :public Reg16<>{static constexpr RegAddr ADDRESS = addr; uint16_t bits;} DEF_R16(name)

#define REG8_QUICK_DEF(addr, type, name)\
struct type :public Reg8<>{static constexpr RegAddr ADDRESS = addr; uint8_t bits;} DEF_R8(name)


#if 0

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
#endif
}
