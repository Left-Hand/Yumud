#pragma once

#include "enum_array.hpp"

namespace ymd{
template<typename E, typename T>
class EnumScaler{
public:
    constexpr EnumScaler(const E init_e, const EnumArray<E, T> & mapping):
        enum_(init_e),
        mapping_(mapping)
        {;}

    constexpr EnumScaler & operator =(const E e){
        enum_ = e;
    }

    constexpr auto to_fullscale() const {
        return mapping_[enum_];
    }

private:
    E enum_;
    // size_t scale_;
    const EnumArray<E, T> & mapping_;
};

}