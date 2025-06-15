#pragma once

#include <type_traits>

#include "core/magic/enum_traits.hpp"
#include "core/utils/Option.hpp"

namespace ymd{
template<typename E, typename T>
requires std::is_enum_v<E>
class EnumArray{
public:
    using Dump = magic::enum_dump<E>;
    static constexpr size_t COUNT = Dump::COUNT;

    constexpr EnumArray() = default;
    constexpr EnumArray(std::array<T, COUNT> arr):
        arr_(arr){;}


    template<typename... Args>
    requires (sizeof...(Args) == COUNT)
    constexpr EnumArray(Args&&... args):
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

    template<typename U>
    constexpr Option<E> lower_bound(U&& val) const {
        // 使用二分查找找到第一个大于等于 val 的枚举项
        auto it = std::lower_bound(arr_.begin(), arr_.end(), std::forward<U>(val));
        if (it != arr_.end()) {
            // 找到对应的枚举项
            size_t idx = std::distance(arr_.begin(), it);
            return Some(Dump::index_to_enum(idx));
        }
        return None; // 未找到
    }

    template<typename U>
    constexpr Option<E> upper_bound(U&& val) const {
        // 使用二分查找找到第一个大于 val 的枚举项
        auto it = std::upper_bound(arr_.begin(), arr_.end(), std::forward<U>(val));
        if (it != arr_.end()) {
            // 找到对应的枚举项
            size_t idx = std::distance(arr_.begin(), it);
            return Some(Dump::index_to_enum(idx));
        }
        return None; // 未找到
    }

    const T * begin() const { return arr_.begin(); }
    const T * end() const { return arr_.end(); }

    T * begin() { return arr_.begin(); }
    T * end() { return arr_.end(); }

private:
    std::array<T, COUNT> arr_;
};

template<typename E, typename T>
class EnumScaler{
public:
    constexpr EnumScaler(const E init_e, const EnumArray<E, T> & mapping):
        enum_(init_e),
        // scale_(scale),
        mapping_(mapping)
        {;}

    constexpr EnumScaler & operator =(const E e){
        enum_ = e;
    }

    // constexpr auto to_lsb(){
    //     return mapping_[enum_] / scale_;
    // }

    constexpr auto to_fullscale(){
        return mapping_[enum_];
    }

private:
    E enum_;
    // size_t scale_;
    const EnumArray<E, T> & mapping_;
};


}