#pragma once

#include "core/tmp/bits/width.hpp"

namespace ymd{

//指向静态确定起始比特位和结束比特位的代理
template<typename D,
	size_t BEGIN_OFFSET, size_t END_OFFSET,
    typename T = D>
struct [[nodiscard]] BitFieldProxy final{
private:    
    D & p_obj_;

    static constexpr D lower_mask = tmp::mask_calculator::lower_mask_of<D>(BEGIN_OFFSET);
	static constexpr D mask = tmp::mask_calculator::mask_of<D>(BEGIN_OFFSET, END_OFFSET);

    static constexpr size_t WIDTH = END_OFFSET - BEGIN_OFFSET;

public:
    using underlying_type = D;

    [[nodiscard]] __attribute__((always_inline)) constexpr
    explicit BitFieldProxy(D & obj):p_obj_(obj){;}

    __attribute__((always_inline)) constexpr 
    auto & operator =(const T & in){
        set(in);
        return *this;
    }

    __attribute__((always_inline)) constexpr 
    void set(const T & in){
        static_assert(!std::is_const_v<D>, "cannot assign to const");
        p_obj_ = (static_cast<D>(
            std::bit_cast<tmp::size_to_uint_t<sizeof(in)>>(in)) << BEGIN_OFFSET) | (p_obj_ & ~mask);
    }
    [[nodiscard]] __attribute__((always_inline)) constexpr 
    T get() const{
        return std::bit_cast<T>(static_cast<tmp::size_to_uint_t<sizeof(T)>>((p_obj_ & mask) >> (BEGIN_OFFSET)));
    }

    [[nodiscard]] __attribute__((always_inline)) constexpr 
    T operator *() const{
        return get();
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
    D & p_obj_;
    const size_t beign_offset;
    const D mask_;

public:
    using underlying_type = D;

    [[nodiscard]] __attribute__((always_inline)) constexpr
    explicit DynBitFieldProxy(D & obj, const size_t begin_offset, const size_t END_OFFSET):
        p_obj_(obj), 
        beign_offset(begin_offset), 
        mask_(tmp::mask_calculator::mask_of<D>(begin_offset, END_OFFSET)){;}

    [[nodiscard]] __attribute__((always_inline)) constexpr 
    auto & operator =(T && in){
        p_obj_ = (static_cast<D>(in) << beign_offset) | (p_obj_ & ~mask_);
        return *this;
    }

    [[nodiscard]] __attribute__((always_inline)) constexpr 
    T get() const{
        return static_cast<T>((p_obj_ & mask_) >> (beign_offset));
    }

    [[nodiscard]] __attribute__((always_inline)) constexpr 
    T operator *() const{
        return get();
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

    static_assert(BEGIN_OFFSET + WIDTH <= sizeof(D) * 8, "bitfield array is longer than obj");
    static_assert(ELEMENT_WIDTH * LEN == WIDTH, "bitfield array is not aligned with obj");

    D & p_obj_;
public:
    constexpr explicit BitFieldArrayProxy(D & obj):p_obj_(obj){;}

    [[nodiscard]] __attribute__((always_inline)) constexpr 
    DynBitFieldProxy<D> operator [](const size_t idx) const {
        return DynBitFieldProxy<D>(p_obj_, BEGIN_OFFSET + ELEMENT_WIDTH * idx, BEGIN_OFFSET + ELEMENT_WIDTH * (idx + 1));
    }

    [[nodiscard]] __attribute__((always_inline)) constexpr 
    auto get_element(size_t idx) const {
        return (*this)[idx];
    }

    template<size_t idx>
    [[nodiscard]] __attribute__((always_inline)) constexpr 
    auto get_element() const {
        static_assert(idx < LEN, "index out of range");
        return BitFieldProxy<D, BEGIN_OFFSET + ELEMENT_WIDTH * idx, BEGIN_OFFSET + ELEMENT_WIDTH * (idx + 1)>(p_obj_);
    }
};


namespace details{
template<size_t BEGIN_OFFSET, size_t END_OFFSET, typename T>
[[nodiscard]] __attribute__((always_inline)) static constexpr
auto _make_bitfield_proxy(auto && obj){
    using D = typename std::remove_reference_t<decltype(obj)>;
    return BitFieldProxy<D, BEGIN_OFFSET, END_OFFSET, T>(obj);
}


}


template<size_t BEGIN_OFFSET, size_t END_OFFSET>
[[nodiscard]] static constexpr 
auto make_bitfield_proxy(auto & obj){
    using D = typename std::remove_reference_t<decltype(obj)>;
    return details::_make_bitfield_proxy<BEGIN_OFFSET, END_OFFSET, D>(obj);
}

template<size_t BEGIN_OFFSET, size_t END_OFFSET, typename T>
[[nodiscard]] static constexpr 
auto make_bitfield_proxy(auto & obj){
    return details::_make_bitfield_proxy<BEGIN_OFFSET, END_OFFSET, T>(obj);
}

template<size_t BEGIN_OFFSET, size_t ELEMENT_WIDTH, size_t LEN, typename D>
[[nodiscard]] static constexpr
BitFieldArrayProxy<D, BEGIN_OFFSET, ELEMENT_WIDTH, LEN> make_bfarray_proxy(D & obj){
    return BitFieldArrayProxy<D, BEGIN_OFFSET, ELEMENT_WIDTH, LEN>(obj);
}

}