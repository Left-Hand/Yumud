#pragma once

namespace ymd{

template<typename T>
struct As{
    template<typename U>
    constexpr T operator |(const U & u) const {
        return static_cast<T>(u);
    }

    template<typename U>
    friend constexpr T operator |(const U & u, const As<T>){
        return static_cast<T>(u);
    }
};

template<typename T>
static constexpr As<T> as(){return As<T>();};

}