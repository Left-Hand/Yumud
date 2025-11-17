#pragma once

#include "core/magic/size_traits.hpp"

namespace ymd{
template<typename D,
	size_t b_bits, size_t e_bits,
    typename I = D>

struct BitFieldRef{
private:    
    using T = std::decay_t<D>;
    D & data_;

    static constexpr D lower_mask = magic::lower_mask_of<T>(b_bits);
	static constexpr D mask = magic::mask_of<T>(b_bits, e_bits);

    static constexpr size_t bits_len = e_bits - b_bits;

public:
    using data_type = D;

    [[nodiscard]] __inline constexpr
    explicit BitFieldRef(D & data):data_(data){;}

    __inline constexpr 
    auto & operator =(const I & in){
        static_assert(!std::is_const_v<D>, "cannot assign to const");
        data_ = (static_cast<D>(
            std::bit_cast<magic::type_to_uint_t<decltype(in)>>(in)) << b_bits) | (data_ & ~mask);
        return *this;
    }


    [[nodiscard]] __inline constexpr 
    I as_bits() const{
        return std::bit_cast<I>(static_cast<magic::type_to_uint_t<I>>((data_ & mask) >> (b_bits)));
    }

    [[nodiscard]] __inline consteval
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

    [[nodiscard]] __inline constexpr
    explicit BitFieldDyn(D & data, const size_t b_bits, const size_t e_bits):
        data_(data), b_bits_(b_bits), mask_(magic::mask_of<T>(b_bits, e_bits)){;}

    [[nodiscard]] __inline constexpr 
    auto & operator =(I && in){
        data_ = (static_cast<D>(in) << b_bits_) | (data_ & ~mask_);
        return *this;
    }

    [[nodiscard]] __inline constexpr 
    I as_val() const{
        return static_cast<I>((data_ & mask_) >> (b_bits_));
    }
};


namespace details{
template<size_t b_bits, size_t e_bits, typename I>
[[nodiscard]] __inline static constexpr
auto _make_bitfield(auto && data){
    using D = typename std::remove_reference_t<decltype(data)>;
    return BitFieldRef<D, b_bits, e_bits, I>(data);
}

template<typename T>
struct _bitfield_data_type{};
}

template<typename T>
using bitfield_data_type_t = typename T::data_type;


template<typename T, size_t b_bits, size_t e_bits, size_t cnt>
struct BitFieldArray{
private:    
    // using T = std::decay_t<D>;
    static constexpr size_t data_len = magic::type_to_bits_v<T>;
    static constexpr size_t len = e_bits - b_bits;
    static constexpr size_t per_len = (len / cnt);
    // static constexpr size_t len = ()

    static_assert(b_bits + len <= data_len, "bitfield array is longer than reg data");
    static_assert(per_len * cnt == len, "bitfield array is not aligned with reg data");

    T & data_;
public:
    constexpr 
    explicit BitFieldArray(T & data):data_(data){;}

    [[nodiscard]] __inline constexpr 
    BitFieldDyn<T> operator [](const size_t idx) const {
        return BitFieldDyn<T>(data_, b_bits + per_len * idx, b_bits + per_len * (idx + 1));
    }

    template<size_t idx>
    [[nodiscard]] __inline constexpr 
    auto get() const {
        static_assert(idx < cnt, "index out of range");
        return BitFieldRef<T, b_bits + per_len * idx, b_bits + per_len * (idx + 1)>(data_);
    }
};



template<size_t b_bits, size_t e_bits>
[[nodiscard]] __fast_inline static constexpr 
auto make_bitfield(auto & data){
    using D = typename std::remove_reference_t<decltype(data)>;
    return details::_make_bitfield<b_bits, e_bits, D>(data);
}


template<size_t b_bits, size_t e_bits, typename I>
[[nodiscard]] __fast_inline static constexpr 
auto make_bitfield(auto & data){
    // using D = typename std::decay_t<decltype(data)>;
    return details::_make_bitfield<b_bits, e_bits, I>(data);
}


template<size_t b_bits, size_t per_len, size_t cnt, typename D>
[[nodiscard]] __fast_inline static constexpr
BitFieldArray<D, b_bits, per_len, cnt> make_bfarray(D & data){
    return BitFieldArray<D, b_bits, per_len, cnt>(data);
}




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
// struct BitFieldRef{};





// template<auto ... Args>
// using BitField_u16 = BitFieldRef<const uint16_t, Args...>;

// template<auto ... Args>
// using BitField_mu16 = BitFieldRef<uint16_t, Args...>;




// static_assert(std::is_same_v<uint16_t, reg_data_type_t<uint16_t>>, "reg_data_type_t is not uint16_t");

// template<size_t b_bits, size_t e_bits, typename T>
// __inline static constexpr
// auto make_mut_bitfield(T * obj){
//     // using T = std::remove_pointer_t<decltype(obj)>;
//     using D = typename std::remove_const_t<reg_data_type_t<T>>;
    
//     return BitFieldRef<D, b_bits, e_bits>(*reinterpret_cast<D *>(obj));
// }


// template<size_t b_bits, size_t e_bits, typename I>
// BitFieldRef(auto && data) -> BitFieldRef<decltype(data), b_bits, e_bits, I>;




// template<size_t b_bits, size_t e_bits, typename D, typename I = D>
// __inline static constexpr
// auto make_mut_bitfield(D & data){
//     return BitFieldRef<D, b_bits, e_bits, I>(data);
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
    
//     return BitFieldRef<reg_data_type, b_bits, e_bits>(reinterpret_cast<reg_data_type &>(*obj));
// }
}