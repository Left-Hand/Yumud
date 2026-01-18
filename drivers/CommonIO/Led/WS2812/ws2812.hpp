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
    ) const{

        for(size_t i = 0; i < 8; i++){
            buf[i] = (gs8 & 0x80) ? high_cvr : low_cvr;
            gs8 = gs8 << 1;
        }
    }

    constexpr void apply_rgb888_to_buf(
        std::span<uint16_t, 24> buf, 
        std::array<uint8_t, 3> rgb888
    ) const{
        apply_gs8_to_buf(std::span<uint16_t, 8>(buf.begin() + 0, 8), rgb888[0]);
        apply_gs8_to_buf(std::span<uint16_t, 8>(buf.begin() + 8, 8), rgb888[1]);
        apply_gs8_to_buf(std::span<uint16_t, 8>(buf.begin() + 16, 8), rgb888[2]);
    }

};

struct WS2812_Transport{
public:
    WS2812_Transport(hal::GpioIntf & gpio):gpio_(gpio){;}

    __no_inline static void delay_long();
    __no_inline static void delay_short();

    void send_bit(const bool state);
    void send_byte(const uint8_t data);
    void send_reset();
    void init();
private:
    hal::GpioIntf & gpio_;
};


struct WS2812: public RgbLedIntf{
public:
    explicit WS2812(hal::GpioIntf & gpio):transport_(gpio){;}
    void init();
    void set_rgb(const RGB<iq16> &color);
private:
    WS2812_Transport transport_;
};

struct WS2812Single: public RgbLedIntf{
public:
    RGB<iq16> color;
    WS2812Single() = default;
};


template<size_t N>
struct WS2812Chain{
protected:
    WS2812_Transport transport_;
    std::array<WS2812Single, N> leds;

public:
    WS2812Chain(hal::GpioIntf & gpio):transport_(gpio){;}
    void init(){
        for(auto & led : leds) led = RGB(0,0,0);
        transport_.init();
    }

    WS2812Single & operator[](const int index){
        if(index < 0) return leds.at(N + index);
        else return leds.at(index);
    }

    void refresh(){

        transport_.send_reset();

        for(auto & led : leds){
            uint16_t r,g,b;

            uni_to_u16(led.color.r, r);
            uni_to_u16(led.color.g, g);
            uni_to_u16(led.color.b, b);

            transport_.send_byte(CLAMP(uint8_t(r * 255), 0, 255));
            transport_.send_byte(CLAMP(uint8_t(g * 255), 0, 255));
            transport_.send_byte(CLAMP(uint8_t(b * 255), 0, 255));

        }

    }
};

};
