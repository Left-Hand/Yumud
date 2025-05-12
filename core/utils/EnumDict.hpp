#pragma once

#include <type_traits>

#include "core/utils/typetraits/size_traits.hpp"
#include "core/utils/typetraits/function_traits.hpp"
// #include "core/utils/typetraits/typetraits_details.hpp"
#include "core/utils/typetraits/serialize_traits.hpp"
#include "core/utils/typetraits/enum_traits.hpp"

namespace ymd{

template<typename E, typename T>
requires std::is_enum_v<E>
class EnumDict{
public:
    using Dump = magic::enum_dump<E>;
    static constexpr size_t COUNT = Dump::COUNT;

    T & operator [](const E e){
        const auto idx_opt = Dump::enum_to_index(e);
        if(idx_opt.has_value()) return dict[idx_opt.value()];
        else __builtin_abort();
    }

    const T & operator [](const E e) const {
        const auto idx_opt = Dump::enum_to_index(e);
        if(idx_opt.has_value()) return dict[idx_opt.value()];
        else __builtin_abort();
    }
private:
    std::array<T, COUNT> dict;
};
}