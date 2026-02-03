#pragma once

#include <atomic>
#include <algorithm>
#include <array>

#include "core/tmp/bits/width.hpp"
#include "core/utils/enum/enum_rearranger.hpp"

namespace ymd{

template<typename Policy>
struct [[nodiscard]] BitFlag final{
    static constexpr size_t N = Policy::length();
    static constexpr size_t NUM_BITS = N;  // 每个标志使用1位
    
    using Enum = typename Policy::Enum;
    using D = typename tmp::width_to_least_uint_t<NUM_BITS>;

    static constexpr D rank_to_mask(const size_t rank) {
        return D(1) << rank;
    }

    static constexpr D enum_to_mask(const Enum e) {
        return rank_to_mask(Policy::enum_to_rank(e));
    }

    static constexpr BitFlag from_enum(const Enum e) {
        return BitFlag::from_bits(enum_to_mask(e));
    }

    static constexpr BitFlag from_enums(const std::initializer_list<Enum> es){
        D bits = 0;
        for(const auto e : es){
            bits |= enum_to_mask(e);
        }
        return BitFlag::from_bits(bits);
    }

    static constexpr BitFlag from_bits(const D bits){
        return BitFlag(bits);
    }

    constexpr void set(const Enum e) {
        set_by_mask(enum_to_mask(e));
    }


    constexpr void clear(const Enum e) {
        clear_by_mask(enum_to_mask(e));
    }

    [[nodiscard]] bool test(const Enum e) const {
        return test(enum_to_mask(e));
    }

    constexpr void toggle(const Enum e) {
        toggle(enum_to_mask(e));
    }

    [[nodiscard]] constexpr D load() const {
        return bits_;
    }

    constexpr void store(D value) {
        bits_ = value;
    }

    struct [[nodiscard]] Sentinel {};

    struct [[nodiscard]] Iterator {
        const D remaining_;
        size_t rank_;
        
    public:
        using iterator_category = std::input_iterator_tag;
        using value_type = Enum;
        using difference_type = std::ptrdiff_t;
        using pointer = const Enum*;
        using reference = const Enum&;

        explicit constexpr Iterator(D remaining) : 
            remaining_(remaining), rank_(0) {}

        constexpr Enum operator*() const {
            return Policy::rank_to_enum(rank_);
        }

        constexpr Iterator& operator++() {
            advance();
            return *this;
        }

        constexpr Iterator operator++(int) {
            Iterator tmp = *this;
            ++(*this);
            return tmp;
        }

        constexpr bool operator==(const Iterator& other) const = default;
        
        constexpr bool operator==(Sentinel) const {
            return remaining_ == 0;
        }

        constexpr size_t count_ones() const {
            return __builtin_popcount(remaining_);
        }

    private:
        constexpr void advance() {
            const D mask = (D(1) << rank_);
            if (remaining_ & mask) {
                return;
            }
            ++rank_;
        }
    };


    [[nodiscard]] size_t count_ones() const {
        return __builtin_popcount(bits_);
    }

    Iterator begin() const {
        D value = bits_;
        Iterator it(value);
        return it;
    }

    constexpr Sentinel end() const {
        return Sentinel{};
    }

private:
    D bits_{0};

    constexpr BitFlag(const D bits):
        bits_(bits){;}

    constexpr void set_by_mask(const D mask) {
        bits_ |= (mask);
    }

    constexpr void clear_by_mask(const D mask) {
        bits_ &= (~mask);
    }

};



}