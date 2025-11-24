#pragma once

#include "../tmp_primitive.hpp"
#include <bit>

namespace ymd::tmp{

namespace details{
    template <size_t N_bytes>
    struct _bytes_to_sint;
    
    template <>
    struct _bytes_to_sint<1> {
        using type = int8_t;
    };
    
    template <>
    struct _bytes_to_sint<2> {
        using type = int16_t;
    };
    
    template <>
    struct _bytes_to_sint<4> {
        using type = int32_t;
    };

    template <>
    struct _bytes_to_sint<8> {
        using type = int64_t;
    };

    template <size_t N_bytes>
    struct _bytes_to_uint;
    
    template <>
    struct _bytes_to_uint<1> {
        using type = uint8_t;
    };
    
    template <>
    struct _bytes_to_uint<2> {
        using type = uint16_t;
    };
    
    template <>
    struct _bytes_to_uint<4> {
        using type = uint32_t;
    };

    template <>
    struct _bytes_to_uint<8> {
        using type = uint64_t;
    };
}


template<size_t N>
using bytes_to_uint_t = typename details::_bytes_to_uint<std::bit_ceil(N)>::type;

template<size_t N>
using bits_to_uint_t = typename details::_bytes_to_uint<std::bit_ceil((N + 7) / 8)>::type;

template<typename T>
using type_to_uint_t = typename details::_bytes_to_uint<sizeof(std::decay_t<T>)>::type;

template<typename T>
using type_to_sint_t = typename details::_bytes_to_sint<sizeof(std::decay_t<T>)>::type;

template<typename T>
static constexpr size_t type_to_bitswidth_v = sizeof(T) * 8;

template<typename T>
static constexpr size_t type_to_bytes_v = sizeof(T);


namespace details{
    template<std::size_t N_bytes>
    struct size_to_sint;

    template<>
    struct size_to_sint<1> {
        using type = int8_t;
    };

    template<>
    struct size_to_sint<2> {
        using type = int16_t;
    };

    template<>
    struct size_to_sint<4> {
        using type = int32_t;
    };

    template<>
    struct size_to_sint<8> {
        using type = int64_t;
    };

    template<std::size_t N_bytes>
    struct size_to_uint;

    template<>
    struct size_to_uint<1> {
        using type = uint8_t;
    };

    template<>
    struct size_to_uint<2> {
        using type = uint16_t;
    };

    template<>
    struct size_to_uint<4> {
        using type = uint32_t;
    };

    template<>
    struct size_to_uint<8> {
        using type = uint64_t;
    };
}

template<size_t N_bytes>
using size_to_sint_t = typename details::size_to_sint<N_bytes>::type;

template<size_t N_bytes>
using size_to_uint_t = typename details::size_to_uint<N_bytes>::type;


template<typename D>
requires (std::is_unsigned_v<D> and std::is_integral_v<D>)
static constexpr D int_to_zero_v = static_cast<D>(0);

template<typename D>
requires (std::is_unsigned_v<D> and std::is_integral_v<D>)
static constexpr D int_to_full_v = static_cast<D>(~int_to_zero_v<D>);



struct mask_calculator{
    template<typename T>
    __inline static constexpr 
    T lower_mask_of(const size_t b){
        if(b == 64) return static_cast<T>(~uint64_t(0));
        return static_cast<T>(uint64_t(uint64_t(1) << b) - 1);
    }

    template<typename T>
    __inline static constexpr 
    T mask_of(const size_t b_bits, const size_t e_bits){
        const T lower_mask = lower_mask_of<T>(b_bits);
        const T upper_mask = lower_mask_of<T>(e_bits);
        return upper_mask & ~lower_mask;
    }

    template<typename D>
    __inline static constexpr 
    bool is_under_bits(const size_t bits, const D && val){
        using T = std::decay_t<D>;

        const T lower_mask = lower_mask_of<T>(bits);
        return val <= lower_mask;
    }
};

namespace details{
template<typename T>
struct _pure_sizeof{
private:
    [[no_unique_address]] T _1;
    uint8_t _2;
};
}

template<typename T>
static constexpr size_t pure_sizeof_v = (sizeof(details::_pure_sizeof<T>) - 1);
}