#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <span>
#include <bit>


namespace ymd::vofa{


template<typename Receiver, typename ... Args>
constexpr void ser_just_float(Receiver & receiver, Args && ... args){
    constexpr std::array<uint8_t, 4> TAIL_BYTES = {0x00, 0x00, 0x80, 0x7f} ;
    auto push_bytes = [&](std::span<const uint8_t> bytes) constexpr {
        receiver.push_bytes(bytes);
    };

    auto push_element = [&]<typename T>(T && element) constexpr {
        std::array<uint8_t, 4> bytes = std::bit_cast<std::array<uint8_t, 4>>(
            static_cast<float>(std::forward<T>(element))
        );
        push_bytes(std::span(bytes));
    };

    (push_element(std::forward<Args>(args)), ...);



    push_bytes(std::span(TAIL_BYTES));
};


}