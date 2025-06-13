#pragma once

#include <atomic>
#include <algorithm>
#include <array>
#include "core/utils/typetraits/size_traits.hpp"

namespace ymd{

template<typename E, E ... EList> 
struct EnumList final{

public:
    using Enum = E;
    static constexpr size_t N = sizeof...(EList);
private:
    static constexpr std::array<Enum, N> VALUES = std::to_array({EList...});

    template<size_t ... Is>
    static consteval size_t enum_to_rank_tmp_impl(const E e, std::index_sequence<Is...>) {
        size_t result = N; // 初始化为无效值
        // 使用折叠表达式查找匹配的枚举值
        ((VALUES[Is] == e ? (result = Is, false) : false), ...);
        return result;
    }

public:

    static consteval size_t size(){
        return N;
    }

    static constexpr E rank_to_enum(size_t rank) {
        return VALUES[rank];
    }

    template<E e>
    static constexpr size_t enum_to_rank_v = enum_to_rank_tmp_impl(e, std::make_index_sequence<N>{});

    template<size_t rank>
    static constexpr E rank_to_enum_v = rank_to_enum(rank);

    static constexpr size_t enum_to_rank(const E e){
        return std::distance(VALUES.begin(),
            std::find(VALUES.begin(), VALUES.end(), e));
    }
};


template<typename Layout>
struct BitFlag final{
    static constexpr size_t N = Layout::size();
    static constexpr size_t BITS = N;  // 每个标志使用1位
    
    using Enum = typename Layout::Enum;
    using Raw = typename magic::bits_to_uint_t<BITS>;
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

enum class Fault:uint8_t{
    OverCurrent,
    OverVoltage,
    OverTemperature,
    UnderTemperature
};

enum class MyEnum { A, B, C };

using MyEnumList = EnumList<MyEnum, MyEnum::A, MyEnum::B, MyEnum::C>;

static_assert(MyEnumList::size() == 3);
static_assert(MyEnumList::enum_to_rank_v<MyEnum::B> == 1);
static_assert(MyEnumList::rank_to_enum_v<1> == MyEnum::B);

using MyBitFlag = BitFlag<MyEnumList>;

static_assert(MyBitFlag::BITS == 3);
// static_assert(MyBitFlag::from_enums({MyEnum::A, MyEnum::B}).begin().size() == 2);
static_assert(MyBitFlag::enum_to_mask(MyEnum::A)  == 1 << 0);
static_assert(MyBitFlag::enum_to_mask(MyEnum::B)  == 1 << 1);
static_assert(MyBitFlag::Iterator(
    MyBitFlag::enum_to_mask(MyEnum::A) |
    MyBitFlag::enum_to_mask(MyEnum::B)
).size()  == 2);
}