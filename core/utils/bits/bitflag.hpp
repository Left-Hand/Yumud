#pragma once

#include <atomic>
#include <algorithm>
#include <array>

#include "core/tmp/bits/width.hpp"
#include "core/utils/enum/enum_list.hpp"

namespace ymd{

template<typename Layout>
struct BitFlag final{
    static constexpr size_t N = Layout::size();
    static constexpr size_t BITS = N;  // 每个标志使用1位
    
    using Enum = typename Layout::Enum;
    using Raw = typename tmp::bits_to_uint_t<BITS>;
    using Storage = std::atomic<Raw>;

    static constexpr Raw rank_to_mask(const size_t rank) {
        return Raw(1) << rank;
    }

    static constexpr Raw enum_to_mask(const Enum e) {
        return rank_to_mask(Layout::enum_to_rank(e));
    }

    static constexpr BitFlag from_enum(const Enum e) {
        return BitFlag::from_raw(enum_to_mask(e));
    }

    static constexpr BitFlag from_enums(const std::initializer_list<Enum> es){
        Raw raw = 0;
        for(const auto e : es){
            raw |= enum_to_mask(e);
        }
        return BitFlag::from_raw(raw);
    }

    static constexpr BitFlag from_raw(const Raw raw){
        return BitFlag(raw);
    }

    constexpr void set(const Enum e) {
        set_by_mask(enum_to_mask(e));
    }


    constexpr void clear(const Enum e) {
        clear_by_mask(enum_to_mask(e));
    }

    bool test(const Enum e) const {
        return test(enum_to_mask(e));
    }

    constexpr void toggle(const Enum e) {
        toggle(enum_to_mask(e));
    }

    constexpr Raw load() const {
        return storage_.load(std::memory_order_relaxed);
    }

    constexpr void store(Raw value) {
        storage_.store(value, std::memory_order_relaxed);
    }

    struct Sentinel {};

    class Iterator {
        const Raw remaining_;
        size_t rank_;
        
    public:
        using iterator_category = std::input_iterator_tag;
        using value_type = Enum;
        using difference_type = std::ptrdiff_t;
        using pointer = const Enum*;
        using reference = const Enum&;

        explicit constexpr Iterator(Raw remaining) : 
            remaining_(remaining), rank_(0) {}

        constexpr Enum operator*() const {
            return Layout::rank_to_enum(rank_);
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

        constexpr size_t size() const {
            return __builtin_popcount(remaining_);
        }

    private:
        constexpr void advance() {
            const Raw mask = (Raw(1) << rank_);
            if (remaining_ & mask) {
                return;
            }
            ++rank_;
        }
    };


    Iterator begin() const {
        Raw value = storage_.load(std::memory_order_relaxed);
        Iterator it(value);
        return it;
    }

    size_t size() const {
        return __builtin_popcount(storage_.load(std::memory_order_relaxed));
    }

    constexpr Sentinel end() const {
        return Sentinel{};
    }

private:
    Storage storage_{0};

    constexpr BitFlag(const Raw raw):
        storage_(raw){;}

    constexpr void set_by_mask(const Raw mask) {
        storage_.fetch_or(mask, std::memory_order_relaxed);
    }

    constexpr void clear_by_mask(const Raw mask) {
        storage_.fetch_and(~mask, std::memory_order_relaxed);
    }

};



}