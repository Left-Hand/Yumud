#pragma once

#include <cstddef>
#include "core/tmp/bits/width.hpp"
#include "core/utils/Option.hpp"
#include <bit>

namespace ymd{

template<size_t N>
requires (N <= 32 and N > 0)
struct [[nodiscard]] BitsSet final{
    using Self = BitsSet<N>;
    using D = tmp::width_to_least_uint_t<N>;

    static constexpr D BITS_MASK = tmp::mask_calculator::lower_mask_of<D>(N);

    template<typename D2 = D>
    static constexpr D2 INVALID_BITS_MASK = static_cast<D2>(~BITS_MASK);

    // 从短片段构造
    template<size_t N2>
    requires (N2 <= N)
    constexpr explicit BitsSet(const BitsSet<N2> & other):
        bits_(static_cast<D>(other.bits_)){}

    template<typename D2 = D>
    static constexpr Self from_bits(const D2 bits){
        // using D2 = std::decay_t<decltype(bits)>;
        if constexpr(N != sizeof(D2) * 8)
            if(bits & INVALID_BITS_MASK<D2>) [[unlikely]]
                __builtin_trap();
        return from_bits_unchecked(bits);
    }

    static consteval Self zero(){
        return Self::from_bits(0);
    }

    static consteval Self full(){
        return Self::from_bits(Self::BITS_MASK);
    }

    template<typename D2 = D>
    static constexpr Option<Self> try_from_bits(const D2 bits){
        if constexpr(N != sizeof(D2) * 8)
            if(bits & INVALID_BITS_MASK<D2>) [[unlikely]]
                return None;
        return Some(from_bits_unchecked(bits));
    }

    template<typename D2 = D>
    static constexpr Self from_bits_bounded(const D2 bits){
        if constexpr (std::is_same_v<D, D2>)
            return from_bits_unchecked(bits);
        else 
            return from_bits_unchecked(bits & BITS_MASK);
    }

    static constexpr Self from_bits_unchecked(const D bits){
        return Self{bits};
    }

    template<size_t N2>
    constexpr BitsSet<N + N2> connect(const BitsSet<N2> & other) const {
        using D_RET = typename BitsSet<N + N2>::D;
        return BitsSet<N + N2>::from_bits_unchecked(
            static_cast<D_RET>(bits_ << N2) | 
            static_cast<D_RET>(other.bits_)
        );
    }

    template<size_t I>
    requires (I < N and I > 0)
    constexpr std::tuple<BitsSet<I>, BitsSet<N - I>> split() const {
        return std::tuple<BitsSet<I>, BitsSet<N - I>>{
            BitsSet<I>::from_bits_unchecked(bits_ >> (N - I)),
            BitsSet<N - I>::from_bits_unchecked(bits_ & tmp::mask_calculator::lower_mask_of<typename BitsSet<N - I>::D>(N - I))
        };
    }


    [[nodiscard]] constexpr bool highest_bit() const {
        return bool(bits_ & (static_cast<D>(1) << (N - 1)));
    }

    // [[nodiscard]] constexpr std::tuple<bool, BitsSet<N - 1>> split_highest() const requires(N > 1){
    //     static_assert(N > 1);
    //     return std::make_tuple(highest_bit(), BitsSet<N - 1>::from_bits_unchecked(bits_ & (static_cast<D>(~0) << (N - 1))));
    // }


    #if 0
    // 主接口：支持可变数量的分割点
    template<size_t... Sizes>
    requires (sizeof...(Sizes) > 1 && ((Sizes > 0) && ...)) && ((Sizes + ...) <= N)
    constexpr auto split() const {
        return split_impl(std::index_sequence<Sizes...>{});
    }
    #endif

    constexpr D to_bits() const {
        return bits_;
    }

    [[nodiscard]] constexpr bool operator[](const size_t i) const {
        return bool(bits_ & (static_cast<D>(1) << i));
    }

    [[nodiscard]] constexpr bool test(const size_t i) const {
        if (i >= N) [[unlikely]]
            return false;
        return bool(bits_ & (static_cast<D>(1) << i));
    }

    [[nodiscard]] constexpr size_t count_ones() const {
        return __builtin_popcount(bits_);
    }

    [[nodiscard]] static consteval size_t width(){
        return N;
    }



    [[nodiscard]] constexpr bool is_zero() const {
        return bits_ == 0;
    }
    
    // 添加一些有用的操作符
    constexpr bool operator==(const Self& other) const {
        return bits_ == other.bits_;
    }
    
    constexpr bool operator!=(const Self& other) const {
        return bits_ != other.bits_;
    }
    
    constexpr Self operator|(const Self& other) const {
        return from_bits_unchecked(bits_ | other.bits_);
    }
    
    constexpr Self operator&(const Self& other) const {
        return from_bits_unchecked(bits_ & other.bits_);
    }
    
    constexpr Self operator^(const Self& other) const {
        return from_bits_unchecked(bits_ ^ other.bits_);
    }
    
    constexpr Self operator~() const {
        return from_bits_unchecked(~bits_ & BITS_MASK);
    }

    constexpr Self bitwise_not() const {
        return ~*this;
    }

    constexpr Self bitwise_and(const Self & other) const {
        return (*this) & other;
    }

    constexpr Self bitwise_or(const Self & other) const {
        return (*this) | other;
    }

    constexpr Self bitwise_xor(const Self & other) const {
        return (*this) ^ other;
    }

private:
    D bits_ = 0;

    template<size_t N2>
    friend class BitsSet;

    constexpr explicit BitsSet(const D bits):
        bits_(bits){}

    // 修正后的 split_impl 实现
    template<size_t... Sizes>
    constexpr auto split_impl(std::index_sequence<Sizes...>) const {
        constexpr size_t total_specified_bits = (Sizes + ...);
        static_assert(total_specified_bits <= N, "Total specified bits exceed BitsSet size");
        
        constexpr size_t last_part_size = N - total_specified_bits;
        
        // Create helper arrays at compile time
        constexpr size_t part_sizes[] = {Sizes..., (last_part_size > 0 ? last_part_size : 1)};
        constexpr size_t total_parts = sizeof...(Sizes) + (last_part_size > 0 ? 1 : 0);
        
        // Precompute shifts
        constexpr auto compute_shifts = []() constexpr {
            std::array<size_t, sizeof...(Sizes) + 1> shifts = {};
            size_t shift_acc = 0;
            for (int i = int(sizeof...(Sizes)); i >= 0; --i) {
                shifts[i] = shift_acc;
                if (i > 0) shift_acc += part_sizes[i-1];
            }
            return shifts;
        };
        
        constexpr auto shifts = compute_shifts();
        
        return [&]<size_t... Idx>(std::index_sequence<Idx...>) {
            return std::make_tuple(
                [&]() {
                    constexpr size_t current_size = (Idx < sizeof...(Sizes)) ? 
                        part_sizes[Idx] : last_part_size;
                    constexpr size_t current_shift = shifts[Idx];
                    
                    if constexpr (current_size > 0) {
                        constexpr size_t mask = tmp::mask_calculator::lower_mask_of<D>(current_size);
                        return BitsSet<current_size>::from_bits_unchecked((bits_ >> current_shift) & mask);
                    } else {
                        return BitsSet<1>::zero(); // Placeholder - shouldn't be reached when last_part_size=0
                    }
                }()...
            );
        }(std::make_index_sequence<total_parts>{});
    }
};

}

namespace ymd::literals { 
// using bs1 = ymd::BitsSet<1>;
using bs2 = ymd::BitsSet<2>;
using bs3 = ymd::BitsSet<3>;
using bs4 = ymd::BitsSet<4>;
using bs5 = ymd::BitsSet<5>;
using bs6 = ymd::BitsSet<6>;
using bs7 = ymd::BitsSet<7>;
using bs8 = ymd::BitsSet<8>;
using bs9 = ymd::BitsSet<9>;

using bs10 = ymd::BitsSet<10>;
using bs11 = ymd::BitsSet<11>;
using bs12 = ymd::BitsSet<12>;
using bs13 = ymd::BitsSet<13>;
using bs14 = ymd::BitsSet<14>;
using bs15 = ymd::BitsSet<15>;
using bs16 = ymd::BitsSet<16>;
using bs17 = ymd::BitsSet<17>;
using bs18 = ymd::BitsSet<18>;
using bs19 = ymd::BitsSet<19>;

using bs20 = ymd::BitsSet<20>;
using bs21 = ymd::BitsSet<21>;
using bs22 = ymd::BitsSet<22>;
using bs23 = ymd::BitsSet<23>;
using bs24 = ymd::BitsSet<24>;
using bs25 = ymd::BitsSet<25>;
using bs26 = ymd::BitsSet<26>;
using bs27 = ymd::BitsSet<27>;
using bs28 = ymd::BitsSet<28>;
using bs29 = ymd::BitsSet<29>;

using bs30 = ymd::BitsSet<30>;
using bs31 = ymd::BitsSet<31>;
using bs32 = ymd::BitsSet<32>;
}