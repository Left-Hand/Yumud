#pragma once

#include "core/int/uint24_t.hpp"
#include "core/constants/enums.hpp"
#include <bit>
#include <span>

namespace ymd{

template<typename T> 
struct [[nodiscard]] RegCopy final:public T{
public:
    using bits_type = typename T::bits_type;

	constexpr RegCopy(T & owner)
		:owner_(owner)
    {
        T & self = *this;
        self.as_bits_mut() = owner_.to_bits();
    }

    constexpr void apply() const noexcept {
        const T & self = *this;
        owner_.as_bits_mut() = self.to_bits();
    }

private:
	T & owner_;
};

template<typename T>
RegCopy(T) -> RegCopy<T>;


template<typename T, typename D = T>
struct [[nodiscard]] RegBase{
public:
    using underly_type = T;
    using bits_type = T;
    
    [[nodiscard]] constexpr std::span<uint8_t, sizeof(T)> as_bytes_mut() {
        return std::span<uint8_t, sizeof(T)>(reinterpret_cast<uint8_t *>(this), sizeof(T));
    }

    [[nodiscard]] constexpr std::span<const uint8_t, sizeof(T)> as_bytes() const noexcept {
        return std::span<const uint8_t, sizeof(T)>(reinterpret_cast<const uint8_t *>(this), sizeof(T));
    }
    

    /**
     * @brief 获取底层位字段的非 volatile 可变引用。
     * 
     * 此函数用于普通内存访问。若对象表示硬件寄存器，
     * 应确保其底层存储为 non-volatile 或由用户自行管理同步。
     * 
     * @return T& 对底层位字段的引用
     */
    [[nodiscard]] T & as_bits_mut()
    requires (std::is_const_v<T> == false)
    {
        return reinterpret_cast<T &>(*this);
    }

    /**
     * @brief 获取底层位字段的 volatile 可变引用。
     * 
     * 此重载用于当对象被声明为 volatile 时（如映射到 MMIO 地址），
     * 确保对寄存器的写入不会被编译器优化掉。
     * 
     * @return volatile T& 对底层 volatile 位字段的引用
     */
    [[nodiscard]] volatile T & as_bits_mut()
    volatile requires (std::is_const_v<T> == false) 
    {
        return reinterpret_cast<volatile T &>(*this);
    }

    /**
     * @brief 以普通方式写入整个寄存器值。
     * 
     * 直接覆盖底层位字段。适用于普通内存或已知安全的寄存器访问。
     * 
     * @param bits 要写入的位值
     * @return RegBase<T>& 返回自身以支持链式调用
     */
    RegBase<T> & write_bits(T bits) 
    requires (std::is_const_v<T> == false)
    {
        this->as_bits_mut() = static_cast<T>(bits); 
        return *this;
    }
    
    /**
     * @brief 以 volatile 方式写入整个寄存器值。
     * 
     * 确保写入操作不会被编译器优化，并实际写入内存（如硬件寄存器）。
     * 仅在对象为 volatile 时调用此重载。
     * 
     * @param bits 要写入的位值
     * @return RegBase<T>& 返回自身以支持链式调用
     */
    RegBase<T> & write_bits(T bits) 
    volatile requires (std::is_const_v<T> == false)
    {
        this->as_bits_mut() = static_cast<T>(bits); 
        return *this;
    }

    /**
     * @brief 以普通方式读取整个寄存器值。
     * 
     * 直接返回底层位字段的副本。适用于普通内存。
     * 
     * @return T 寄存器当前的位值
     */
    [[nodiscard]] T to_bits() const noexcept {
        return reinterpret_cast<const T &>(*this);
    }

    /**
     * @brief 以 volatile 方式读取整个寄存器值。
     * 
     * 强制从内存中读取（如从硬件寄存器），防止编译器缓存或优化。
     * 仅在对象为 volatile 时调用此重载。
     * 
     * @return T 寄存器当前的位值（从 volatile 内存加载）
     */
    [[nodiscard]] T to_bits() const volatile
    {
        // 正确保留 volatile 语义：直接 reinterpret 为 const volatile T&
        return reinterpret_cast<const volatile T&>(*this);
    }

    template<typename TOther>
    friend class RegCopy;
};

#define DEF_REG_TEMPLATE(name, T, as_fn)\
template<typename D = T>\
struct [[nodiscard]] name:public RegBase<T, D>{\
constexpr T as_fn() const noexcept {return std::bit_cast<T>(this->to_bits());}\
};\

DEF_REG_TEMPLATE(Reg8,  uint8_t,        to_u8)
DEF_REG_TEMPLATE(Reg16, uint16_t,       to_u16)
DEF_REG_TEMPLATE(Reg24, math::uint24_t, to_u24)
DEF_REG_TEMPLATE(Reg32, uint32_t,       to_u32)
DEF_REG_TEMPLATE(Reg64, uint64_t,       to_u64)

#undef DEF_REG_TEMPLATE




#define VALIDATE_R8(type)\
static_assert(sizeof(type) == 1, "x must be 8bit register");\
static_assert(std::has_unique_object_representations_v<type>, "x must has unique bitfield");\

#define VALIDATE_R16(type)\
static_assert(sizeof(type) == 2, "x must be 16bit register");\
static_assert(std::has_unique_object_representations_v<type>, "x must has unique bitfield");\

#define VALIDATE_R32(type)\
static_assert(sizeof(type) == 4, "x must be 32bit register");\
static_assert(std::has_unique_object_representations_v<type>, "x must has unique bitfield");\


#define DEF_R8(name)\
name{};\
VALIDATE_R8(std::decay_t<decltype(name)>)\

#define DEF_R16(name)\
name{};\
VALIDATE_R16(std::decay_t<decltype(name)>)\

#define DEF_R32(name)\
name{};\
VALIDATE_R32(std::decay_t<decltype(name)>)\


#define REG8_QUICK_DEF(addr, type, name)\
struct type :public Reg8<>{\
    static constexpr RegAddr REG_ADDR = RegAddr{addr}; \
    uint8_t bits;} DEF_R8(name)\


#define REG16_QUICK_DEF(addr, type, name)\
struct type :public Reg16<>{\
    static constexpr RegAddr REG_ADDR = RegAddr{addr}; \
    uint16_t bits;} DEF_R16(name)\


#define REG32_QUICK_DEF(addr, type, name)\
struct type :public Reg32<>{\
    static constexpr RegAddr REG_ADDR = RegAddr{addr}; \
    uint32_t bits;} DEF_R32(name)\


}
