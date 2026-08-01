#pragma once

#include <cstdint>

namespace ymd::str{

struct [[nodiscard]] alignas(4) IntTypeErased final{
    uint8_t width;
    bool is_signed;

    template<typename T>
    static consteval IntTypeErased from(){
        return IntTypeErased{
            .width = sizeof(T) * 8,
            .is_signed = std::is_signed_v<T>,
        };
    }
};

struct [[nodiscard]] alignas(4) FixedTypeTag final{
    uint8_t q_num;
    bool is_signed;
};

}