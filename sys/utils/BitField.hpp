#pragma once

#include "typetraits/size_traits.hpp"
#include "sys/utils/Reg.hpp"

namespace ymd{
template<typename D,
	size_t b_bits, size_t e_bits,
    typename I = D>

struct BitField{
private:    
    using T = std::decay_t<D>;
    D & data_;

    static constexpr D lower_mask = lower_mask_of<T>(b_bits);
	static constexpr D mask = mask_of<T>(b_bits, e_bits);

    static constexpr size_t bits_len = e_bits - b_bits;

public:
    using data_type = D;

    __inline constexpr 
    explicit BitField(D & data):data_(data){;}

    __inline constexpr 
    auto & operator =(const I & in){
        static_assert(!std::is_const_v<D>, "cannot assign to const");
        data_ = (static_cast<D>(
            std::bit_cast<type_to_uint_t<decltype(in)>>(in)) << b_bits) | (data_ & ~mask);
        return *this;
    }

    // __inline constexpr 
    // I operator =(I && in){
    //     static_assert(!std::is_const_v<D>, "cannot assign to const");
    //     data_ = (static_cast<D>(in) << b_bits) | (data_ & ~mask);
    //     return *this;
    //     // return in;
    // }

    template<typename T>
    requires std::is_constructible_v<T, I>
    explicit operator T() const{
        return T(this->as_val());
    }

    __inline constexpr 
    I as_val() const{
        return std::bit_cast<I>(static_cast<type_to_uint_t<I>>((data_ & mask) >> (b_bits)));
    }

    __inline consteval
    size_t len() const{
        return bits_len;
    }
};

template<typename D, typename I = D>

struct BitFieldDyn{
private:    
    using T = std::decay_t<D>;
    D & data_;
    const size_t b_bits_;
    const T mask_;

public:
    using data_type = D;

    __inline constexpr 
    explicit BitFieldDyn(D & data, const size_t b_bits, const size_t e_bits):
        data_(data), b_bits_(b_bits), mask_(mask_of<T>(b_bits, e_bits)){;}

    __inline constexpr 
    auto & operator =(I && in){
        data_ = (static_cast<D>(in) << b_bits_) | (data_ & ~mask_);
        return *this;
    }

    __inline constexpr 
    I as_val() const{
        return static_cast<I>((data_ & mask_) >> (b_bits_));
    }
};


namespace details{
template<size_t b_bits, size_t e_bits, typename I>
__inline static constexpr
auto _make_bitfield(auto && data){
    using D = typename std::remove_reference_t<decltype(data)>;
    return BitField<D, b_bits, e_bits, I>(data);
}

template<typename T>
struct _bitfield_data_type{};
}
template<size_t b_bits, size_t e_bits>
__inline static constexpr
auto make_bitfield(auto & data){
    using D = typename std::remove_reference_t<decltype(data)>;
    return details::_make_bitfield<b_bits, e_bits, D>(data);
}


template<size_t b_bits, size_t e_bits, typename I>
__inline static constexpr
auto make_bitfield(auto & data){
    // using D = typename std::decay_t<decltype(data)>;
    return details::_make_bitfield<b_bits, e_bits, I>(data);
}




// template<typename T>
// struct _bitfield_data_type<BitField<T, auto, auto, auto>>{using type = T};

// template<typename T>
// struct _bitfield_data_type<const BitField<T, auto, auto>>{using type = T};

template<typename T>
using bitfield_data_type_t = typename T::data_type;

// template<typename T>
// struct BitFieldArray{};

template<typename T, size_t b_bits, size_t e_bits, size_t cnt>
struct BitFieldArray{
private:    
    // using T = std::decay_t<D>;
    static constexpr size_t data_len = type_to_bits_v<T>;
    static constexpr size_t len = e_bits - b_bits;
    static constexpr size_t per_len = (len / cnt);
    // static constexpr size_t len = ()

    static_assert(b_bits + len <= data_len, "bitfield array is longer than reg data");
    static_assert(per_len * cnt == len, "bitfield array is not aligned with reg data");

    T & data_;
public:
    constexpr 
    explicit BitFieldArray(T & data):data_(data){;}

    __inline constexpr 
    BitFieldDyn<T> operator [](const size_t idx) const {
        return BitFieldDyn<T>(data_, b_bits + per_len * idx, b_bits + per_len * (idx + 1));
    }

    template<size_t idx>
    __inline constexpr 
    auto get() const {
        static_assert(idx < cnt, "index out of range");
        return BitField<T, b_bits + per_len * idx, b_bits + per_len * (idx + 1)>(data_);
    }
};



template<size_t b_bits, size_t per_len, size_t cnt, typename D>
__inline static constexpr
BitFieldArray<D, b_bits, per_len, cnt> make_bfarray(D & data){
    return BitFieldArray<D, b_bits, per_len, cnt>(data);
}

}