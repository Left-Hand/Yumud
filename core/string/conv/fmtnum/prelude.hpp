#pragma once

#include <cstdint>

namespace ymd::str{

struct [[nodiscard]] alignas(4) IntTypeErased final{
    bool is_signed;
    uint8_t width;

    template<typename T>
    static consteval IntTypeErased from(){
        return IntTypeErased{
            .is_signed = std::is_signed_v<T>,
            .width = sizeof(T) * 8
        };
    }
};
}