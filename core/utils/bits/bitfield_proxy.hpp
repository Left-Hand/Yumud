#pragma once

#include "core/tmp/bits/width.hpp"
#include <atomic>


namespace ymd{

//指向静态确定起始比特位和结束比特位的代理
template<typename D,
	size_t BEGIN_OFFSET, size_t END_OFFSET,
    typename T = D>
struct [[nodiscard]] BitFieldProxy final{
private:    
    D * p_bits_;

    // static constexpr D lower_mask = tmp::mask_calculator::lower_mask_of<D>(BEGIN_OFFSET);
	static constexpr D MASK = tmp::mask_calculator::mask_of<D>(BEGIN_OFFSET, END_OFFSET);

    static constexpr size_t WIDTH = END_OFFSET - BEGIN_OFFSET;

    // 确保T可以安全进行bit_cast转换
    static_assert(std::is_trivially_copyable_v<T>, 
                "T must be trivially copyable for bit_cast");
    static_assert(sizeof(T) * 8 >= WIDTH, 
                "Size of T (in bits) must match the bitfield width");
    static_assert(sizeof(T) <= sizeof(D), 
                "Size of T must not exceed size of underlying type D");

public:
    using underlying_type = D;
    using bits_type = tmp::size_to_uint_t<sizeof(T)>;

    [[nodiscard]] __attribute__((always_inline)) constexpr
    explicit BitFieldProxy(D * p_bits):p_bits_(p_bits){;}

    __attribute__((always_inline)) constexpr 
    auto & operator =(const T & in){
        set(in);
        return *this;
    }
    
    __attribute__((always_inline)) constexpr 
    void set(const T & in) requires (!std::is_const_v<D>) {
        set_bits(std::bit_cast<tmp::size_to_uint_t<sizeof(in)>>(in));
    }

    __attribute__((always_inline)) constexpr 
    void atomic_set_high(
        std::memory_order order = std::memory_order_seq_cst
    ) requires (!std::is_const_v<D> && (BEGIN_OFFSET + 1 == END_OFFSET)) {
        atomic_set_all(order);
    }

    __attribute__((always_inline)) constexpr 
    void atomic_set_low(std::memory_order order = std::memory_order_seq_cst
    ) requires (!std::is_const_v<D> && (BEGIN_OFFSET + 1 == END_OFFSET)) {
        atomic_clear_all(order);
    }

    __attribute__((always_inline)) constexpr 
    void atomic_set_all(std::memory_order order = std::memory_order_seq_cst) requires (!std::is_const_v<D>) {
        auto aref = std::atomic_ref<D>(*p_bits_);
        aref.fetch_or(MASK, order);
    }

    __attribute__((always_inline)) constexpr 
    void atomic_clear_all(std::memory_order order = std::memory_order_seq_cst) requires (!std::is_const_v<D>) {
        auto aref = std::atomic_ref<D>(*p_bits_);
        aref.fetch_and(~MASK, order);
    }

    __attribute__((always_inline)) constexpr 
    void set_bits(const auto in_bits) requires (!std::is_const_v<D>) {
        *p_bits_ = (static_cast<D>(in_bits) << BEGIN_OFFSET) | (*p_bits_ & ~MASK);
    }

    [[nodiscard]] __attribute__((always_inline)) constexpr 
    T get() const{
        return std::bit_cast<T>(crop_bits());
    }

    [[nodiscard]] __attribute__((always_inline)) constexpr 
    bits_type crop_bits() const{
        return static_cast<bits_type>((*p_bits_ & MASK) >> (BEGIN_OFFSET));
    }


    [[nodiscard]] __attribute__((always_inline)) consteval
    size_t width() const{
        return WIDTH;
    }
};


//指向动态确定起始比特位和结束比特位的代理
template<typename D, typename T = D>
struct [[nodiscard]] DynBitFieldProxy final{
private:    
    D * p_bits_;
    const size_t beign_offset;
    const D mask_;

public:
    using underlying_type = D;

    [[nodiscard]] __attribute__((always_inline)) constexpr
    explicit DynBitFieldProxy(D * p_bits, const size_t begin_offset, const size_t END_OFFSET):
        p_bits_(p_bits), 
        beign_offset(begin_offset), 
        mask_(tmp::mask_calculator::mask_of<D>(begin_offset, END_OFFSET)){;}

    [[nodiscard]] __attribute__((always_inline)) constexpr 
    auto & operator =(T && in){
        *p_bits_ = (static_cast<D>(in) << beign_offset) | (*p_bits_ & ~mask_);
        return *this;
    }

    [[nodiscard]] __attribute__((always_inline)) constexpr 
    T get() const{
        return static_cast<T>((*p_bits_ & mask_) >> (beign_offset));
    }

};



//指向一段连续的比特位数组
template<typename D, size_t BEGIN_OFFSET, size_t END_OFFSET, size_t LEN>
struct [[nodiscard]] BitFieldArrayProxy final{
private:    
    static constexpr size_t WIDTH = END_OFFSET - BEGIN_OFFSET;
    static constexpr size_t ELEMENT_WIDTH = (WIDTH / LEN);
    static_assert(WIDTH % LEN == 0, "bitfield array is not aligned with obj");
    // static constexpr size_t WIDTH = ()

    static_assert((BEGIN_OFFSET + WIDTH) <= (sizeof(D) * 8), "bitfield array is longer than obj");
    static_assert(ELEMENT_WIDTH * LEN == WIDTH, "bitfield array is not aligned with obj");

    D * p_bits_;
public:
    constexpr explicit BitFieldArrayProxy(D * p_bits):p_bits_(p_bits){;}

    [[nodiscard]] __attribute__((always_inline)) constexpr 
    DynBitFieldProxy<D> operator [](const size_t idx) const {
        const size_t start = BEGIN_OFFSET + ELEMENT_WIDTH * idx;
        return DynBitFieldProxy<D>(
            p_bits_, 
            start, 
            start + ELEMENT_WIDTH
        );
    }

    [[nodiscard]] __attribute__((always_inline)) constexpr 
    auto get_element(size_t idx) const {
        return (*this)[idx];
    }

    template<size_t idx>
    [[nodiscard]] __attribute__((always_inline)) constexpr 
    auto get_element() const {
        static_assert(idx < LEN, "index out of range");
        return BitFieldProxy<D, 
            BEGIN_OFFSET + ELEMENT_WIDTH * idx, 
            BEGIN_OFFSET + ELEMENT_WIDTH * (idx + 1)
        >(p_bits_);
    }
};


namespace details{
template<size_t BEGIN_OFFSET, size_t END_OFFSET, typename T, typename D>
[[nodiscard]] __attribute__((always_inline)) static constexpr
auto _make_bitfield_proxy(D * p_obj){
    return BitFieldProxy<D, BEGIN_OFFSET, END_OFFSET, T>(p_obj);
}
}


template<size_t BEGIN_OFFSET, size_t END_OFFSET, typename T, typename D>
[[nodiscard]] static constexpr 
auto make_bitfield_proxy(D * p_obj){
    return details::_make_bitfield_proxy<BEGIN_OFFSET, END_OFFSET, T>(p_obj);
}

template<size_t BEGIN_OFFSET, size_t ELEMENT_WIDTH, size_t LEN, typename D>
[[nodiscard]] static constexpr
BitFieldArrayProxy<D, BEGIN_OFFSET, ELEMENT_WIDTH, LEN> make_bfarray_proxy(D * p_obj){
    return BitFieldArrayProxy<D, BEGIN_OFFSET, ELEMENT_WIDTH, LEN>(p_obj);
}

}