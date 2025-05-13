#pragma once

#include <type_traits>

#include "core/utils/typetraits/enum_traits.hpp"
#include "core/utils/Option.hpp"

namespace ymd{



template<typename E, typename T>
requires std::is_enum_v<E>
class EnumDict{
public:
    using Dump = magic::enum_dump<E>;
    static constexpr size_t COUNT = Dump::COUNT;

    constexpr EnumDict() = default;
    constexpr EnumDict(std::array<T, COUNT> arr):
        arr_(arr){;}


    template<typename... Args>
    requires (sizeof...(Args) == COUNT)
    constexpr EnumDict(Args&&... args):
        arr_{static_cast<T>(args)...} {}
    constexpr T & operator [](const E e){
        const auto rank_opt = Dump::enum_to_rank(e);
        if(rank_opt.has_value()) return arr_[rank_opt.value()];
        else __builtin_abort();
    }

    constexpr const T & operator [](const E e) const {
        const auto rank_opt = Dump::enum_to_rank(e);
        if(rank_opt.has_value()) return arr_[rank_opt.value()];
        else __builtin_abort();
    }

    constexpr Option<const T &> at(const E e) const {
        const auto rank_opt = Dump::enum_to_rank(e);
        if(rank_opt.has_value()) return Some(&arr_[rank_opt.value()]);
        else return None;
    }


    using Pair = std::pair<E, T>;

    // Pair begin(){return {Dump::index_to_enum(1), arr_.front();}}
    // Pair end(){return {Dump::index_to_enum(COUNT - 1), arr_.back()};}
    constexpr Pair begin() const {return {Dump::index_to_enum(1), arr_.front()};}
    constexpr Pair end() const {return {Dump::index_to_enum(COUNT - 1), arr_.back()};}

private:
    std::array<T, COUNT> arr_;
};



}