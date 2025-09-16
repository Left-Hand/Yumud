#pragma once

#include <concepts>

namespace ymd{

template<typename T>
struct FromZeroDispatcher{

};


template<typename T>
requires std::is_arithmetic_v<T>
struct FromZeroDispatcher<T>{
    static consteval T from_zero() {
        return static_cast<T>(0);
    }
}; 

struct _Zero{
    template<typename T>
    consteval operator T() const {
        return FromZeroDispatcher<T>::from_zero();
    }
};

static constexpr inline _Zero Zero = _Zero{};
}