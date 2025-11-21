#pragma once

#include <cstddef>
#include "core/tmp/bits/width.hpp"
#include "core/utils/Option.hpp"
#include <bit>

namespace ymd{




template<size_t N>
requires (N <= 32 and N > 0)
struct [[nodiscard]] BitsSet {
    using Self = BitsSet<N>;
    using D = tmp::bits_to_uint_t<N>;

    static constexpr D bits_mask = tmp::mask_calculator::lower_mask_of<D>(N);

    template<typename D2 = D>
    static constexpr D2 invalid_bits_mask = static_cast<D2>(~bits_mask);

    // 从短片段构造
    template<size_t N2>
    requires (N2 <= N)
    constexpr explicit BitsSet(const BitsSet<N2> & other):
        bits_(static_cast<D>(other.bits_)){}

    template<typename D2>
    requires (std::is_unsigned_v<D2> && std::is_same_v<D, D2>)
    constexpr explicit BitsSet(const D2 bits):
        bits_(static_cast<D>(bits)){}


    template<typename D2 = D>
    static constexpr Self from_bits(const D2 bits){
        // using D2 = std::decay_t<decltype(bits)>;
        if constexpr(N != tmp::type_to_bitswidth_v<D2>)
            if(bits & invalid_bits_mask<D2>) [[unlikely]]
                __builtin_trap();
        return from_bits_unchecked(bits);
    }

    static consteval Self zero(){
        return Self::from_bits(0);
    }

    static consteval Self full(){
        return Self::from_bits(Self::bits_mask);
    }

    template<typename D2 = D>
    static constexpr Option<Self> try_from_bits(const D2 bits){
        // using D2 = std::decay_t<decltype(bits)>;
        if constexpr(N != tmp::type_to_bitswidth_v<D2>)
            if(bits & invalid_bits_mask<D2>) [[unlikely]]
                return None;
        return Some(from_bits_unchecked(bits));
    }

    template<typename D2 = D>
    static constexpr Self from_bits_bounded(const D2 bits){
        if constexpr (std::is_same_v<D, D2>)
            return from_bits_unchecked(bits);
        else 
            return from_bits_unchecked(bits & bits_mask);
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

    [[nodiscard]] static consteval size_t size(){
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
        return from_bits_unchecked(~bits_ & bits_mask);
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


namespace literals { 
using Bs1 = BitsSet<1>;
using Bs2 = BitsSet<2>;
using Bs3 = BitsSet<3>;
using Bs4 = BitsSet<4>;
using Bs5 = BitsSet<5>;
using Bs6 = BitsSet<6>;
using Bs7 = BitsSet<7>;
using Bs8 = BitsSet<8>;
using Bs9 = BitsSet<9>;

using Bs10 = BitsSet<10>;
using Bs11 = BitsSet<11>;
using Bs12 = BitsSet<12>;
using Bs13 = BitsSet<13>;
using Bs14 = BitsSet<14>;
using Bs15 = BitsSet<15>;
using Bs16 = BitsSet<16>;
using Bs17 = BitsSet<17>;
using Bs18 = BitsSet<18>;
using Bs19 = BitsSet<19>;

using Bs20 = BitsSet<20>;
using Bs21 = BitsSet<21>;
using Bs22 = BitsSet<22>;
using Bs23 = BitsSet<23>;
using Bs24 = BitsSet<24>;
using Bs25 = BitsSet<25>;
using Bs26 = BitsSet<26>;
using Bs27 = BitsSet<27>;
using Bs28 = BitsSet<28>;
using Bs29 = BitsSet<29>;

using Bs30 = BitsSet<30>;
using Bs31 = BitsSet<31>;
using Bs32 = BitsSet<32>;
}
}