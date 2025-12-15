#pragma once


#include <cstddef>
#include <type_traits>

namespace ymd::bithack{

// Individually, the code snippets here are in the public domain (unless otherwise noted) 
// https://graphics.stanford.edu/~seander/bithacks.html#OperationCounting

static constexpr size_t NUM_CHAR_BITS = 8;


template<typename  T>
requires (std::is_integral_v<T>)
static constexpr T min(T x, T y){
    return y + ((x - y) & ((x - y) >> (sizeof(T) * NUM_CHAR_BITS - 1))); // min(x, y)
}

template<typename  T>
requires (std::is_integral_v<T>)
static constexpr T max(T x, T y){
    return x - ((x - y) & ((x - y) >> (sizeof(T) * NUM_CHAR_BITS - 1))); // max(x, y)
}

template<typename  T>
requires (std::is_integral_v<T>)
static constexpr T sign(T v){
    return (v != 0) | (v >> (sizeof(T) * NUM_CHAR_BITS - 1));
}

template<typename T, typename U = std::make_unsigned_t<T>>
requires (std::is_integral_v<T> and std::is_signed_v<T>)
static constexpr U abs(T v){
    const T mask = v >> sizeof(T) * NUM_CHAR_BITS - 1;

    // r = (v + mask) ^ mask;
    // 专利变体：某些 CPU 没有整数绝对值指令（或 编译器无法使用它们）。在分支成本高昂的机器上， 上面的表达式可以比明显的方法更快， r = （v < 0） ？-（unsigned）v ： v，即使作数 是相同的。
    return (v ^ mask) - mask;
}

template<typename T>
requires (std::is_integral_v<T>)
static constexpr bool has_single_bit(T v){ 
    return v && !(v & (v - 1));
}

template<typename T>
requires (std::is_integral_v<T>)
static constexpr T modify_bit(const T word, const T mask, const bool b){ 

    // w ^= (-b ^ w) & m;

    // OR, for superscalar CPUs:
    return (word & ~mask) | (-b & mask);
}


// template<typename T>
// requires (std::is_integral_v<T>)
// static constexpr T log_base_2(const T v){ 
//     T v;	         // 32-bit value to find the log2 of 
//     register T shift;

//     r =     (v > 0xFFFF) << 4; v >>= r;
//     shift = (v > 0xFF  ) << 3; v >>= shift; r |= shift;
//     shift = (v > 0xF   ) << 2; v >>= shift; r |= shift;
//     shift = (v > 0x3   ) << 1; v >>= shift; r |= shift;
//                                             r |= (v >> 1);

// }
// template<typename T>
// requires (std::is_integral_v<T>)
// static constexpr T log_base_10(const T v){ 
//     T v; // non-zero 32-bit integer value to compute the log base 10 of 
//     int r;          // result goes here
//     int t;          // temporary

//     constexpr uint32_t PowersOf10[] = 
//         {1, 10, 100, 1000, 10000, 100000,
//         1000000, 10000000, 100000000, 1000000000};

//     t = (IntegerLogBase2(v) + 1) * 1233 >> 12; // (use a lg2 method from above)
//     r = t - (v < PowersOf10[t]);
// }s
}