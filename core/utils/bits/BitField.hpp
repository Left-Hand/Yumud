#pragma once

#include "core/magic/size_traits.hpp"

namespace ymd{
template<typename D,
	size_t b_bits, size_t e_bits,
    typename T = D>

struct BitFieldRef{
private:    
    D & pdata_;

    static constexpr D lower_mask = magic::lower_mask_of<D>(b_bits);
	static constexpr D mask = magic::mask_of<D>(b_bits, e_bits);

    static constexpr size_t bits_len = e_bits - b_bits;

public:
    using data_type = D;

    [[nodiscard]] __inline constexpr
    explicit BitFieldRef(D & data):pdata_(data){;}

    __inline constexpr 
    auto & operator =(const T & in){
        static_assert(!std::is_const_v<D>, "cannot assign to const");
        pdata_ = (static_cast<D>(
            std::bit_cast<magic::type_to_uint_t<decltype(in)>>(in)) << b_bits) | (pdata_ & ~mask);
        return *this;
    }


    [[nodiscard]] __inline constexpr 
    T to_bits() const{
        return std::bit_cast<T>(static_cast<magic::type_to_uint_t<T>>((pdata_ & mask) >> (b_bits)));
    }

    [[nodiscard]] __inline consteval
    size_t len() const{
        return bits_len;
    }
};

template<typename D, typename T = D>

struct BitFieldDyn{
private:    
    D & pdata_;
    const size_t b_bits_;
    const D mask_;

public:
    using data_type = D;

    [[nodiscard]] __inline constexpr
    explicit BitFieldDyn(D & data, const size_t b_bits, const size_t e_bits):
        pdata_(data), b_bits_(b_bits), mask_(magic::mask_of<D>(b_bits, e_bits)){;}

    [[nodiscard]] __inline constexpr 
    auto & operator =(T && in){
        pdata_ = (static_cast<D>(in) << b_bits_) | (pdata_ & ~mask_);
        return *this;
    }

    [[nodiscard]] __inline constexpr 
    T as_val() const{
        return static_cast<T>((pdata_ & mask_) >> (b_bits_));
    }
};


namespace details{
template<size_t b_bits, size_t e_bits, typename T>
[[nodiscard]] __inline static constexpr
auto _make_bitfield(auto && data){
    using D = typename std::remove_reference_t<decltype(data)>;
    return BitFieldRef<D, b_bits, e_bits, T>(data);
}

template<typename T>
struct _bitfield_data_type{};
}

template<typename T>
using bitfield_data_type_t = typename T::data_type;


template<typename D, size_t b_bits, size_t e_bits, size_t cnt>
struct BitFieldArrayRef{
private:    
    static constexpr size_t data_len = magic::type_to_bits_v<D>;
    static constexpr size_t len = e_bits - b_bits;
    static constexpr size_t per_len = (len / cnt);
    static_assert(len % cnt == 0, "bitfield array is not aligned with reg data");
    // static constexpr size_t len = ()

    static_assert(b_bits + len <= data_len, "bitfield array is longer than reg data");
    static_assert(per_len * cnt == len, "bitfield array is not aligned with reg data");

    D & pdata_;
public:
    constexpr 
    explicit BitFieldArrayRef(D & data):pdata_(data){;}

    [[nodiscard]] __inline constexpr 
    BitFieldDyn<D> operator [](const size_t idx) const {
        return BitFieldDyn<D>(pdata_, b_bits + per_len * idx, b_bits + per_len * (idx + 1));
    }

    template<size_t idx>
    [[nodiscard]] __inline constexpr 
    auto get() const {
        static_assert(idx < cnt, "index out of range");
        return BitFieldRef<D, b_bits + per_len * idx, b_bits + per_len * (idx + 1)>(pdata_);
    }
};



template<size_t b_bits, size_t e_bits>
[[nodiscard]] __fast_inline static constexpr 
auto make_bitfield(auto & data){
    using D = typename std::remove_reference_t<decltype(data)>;
    return details::_make_bitfield<b_bits, e_bits, D>(data);
}


template<size_t b_bits, size_t e_bits, typename T>
[[nodiscard]] __fast_inline static constexpr 
auto make_bitfield(auto & data){
    // using D = typename std::decay_t<decltype(data)>;
    return details::_make_bitfield<b_bits, e_bits, T>(data);
}


template<size_t b_bits, size_t per_len, size_t cnt, typename D>
[[nodiscard]] __fast_inline static constexpr
BitFieldArrayRef<D, b_bits, per_len, cnt> make_bfarray(D & data){
    return BitFieldArrayRef<D, b_bits, per_len, cnt>(data);
}

}