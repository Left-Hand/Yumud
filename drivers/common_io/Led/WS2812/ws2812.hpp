#pragma once

#include "../RgbLed.hpp"
#include "core/clock/clock.hpp"
#include <array>

namespace ymd::hal{
    struct GpioIntf;
    
}
namespace ymd::drivers{

struct W8212_BurstPwmSer{
    uint16_t low_cvr;
    uint16_t high_cvr;

    constexpr void apply_gs8_to_buf(
        const std::span<uint16_t, 8> buf, 
        uint8_t gs8
    ) const noexcept {

        for(size_t i = 0; i < 8; i++){
            buf[i] = (gs8 & 0x80) ? high_cvr : low_cvr;
            gs8 = gs8 << 1;
        }
    }

    constexpr void apply_rgb888_to_buf(
        std::span<uint16_t, 24> buf, 
        std::array<uint8_t, 3> rgb888
    ) const noexcept {
        apply_gs8_to_buf(std::span<uint16_t, 8>(buf.begin() + 0, 8), rgb888[0]);
        apply_gs8_to_buf(std::span<uint16_t, 8>(buf.begin() + 8, 8), rgb888[1]);
        apply_gs8_to_buf(std::span<uint16_t, 8>(buf.begin() + 16, 8), rgb888[2]);
    }

};

struct WS2812_Transport{
public:

    hal::GpioIntf & pin;


    void send_bit(const bool b);
    void send_byte(const uint8_t b);
    void send_reset();

    void send_rgb888(const uint8_t r, const uint8_t g, const uint8_t b);

private:

    __no_inline static void delay_long();
    __no_inline static void delay_short();
};


};
