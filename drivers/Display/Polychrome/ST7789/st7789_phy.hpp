#pragma once

#include "st7789_prelude.hpp"
#include "hal/bus/spi/spihw.hpp"

namespace ymd::drivers{


class ST7789_Transport final:
    ST7789_Prelude{
public:
    static constexpr auto COMMAND_LEVEL = LOW;
    static constexpr auto DATA_LEVEL = HIGH;

    template<typename T = void>
    using IResult = Result<void, drivers::DisplayerError>;

    explicit ST7789_Transport(
        Some<hal::Spi *> spi,
        const hal::SpiSlaveRank rank,
        Some<hal::Gpio *> dc_gpio, 
        Option<hal::Gpio &> res_gpio = None
    ):  
        spi_(spi.deref()), 
        rank_(rank), 
        dc_pin_(dc_gpio.deref()), 
        res_pin_(res_gpio)
        {};

    [[nodiscard]] IResult<> init(){
        dc_pin_.outpp();
        if(res_pin_.is_some())
            res_pin_.unwrap().outpp(HIGH);

        return reset();
    }

    [[nodiscard]] IResult<> reset(){
        if(res_pin_.is_none()) return Ok();
        auto & res_gpio = res_pin_.unwrap();
        clock::delay(10ms);
        res_gpio.set_low();
        clock::delay(10ms);
        res_gpio.set_high();
        return Ok();
    }

    [[nodiscard]] IResult<> write_command(const uint8_t cmd){
        dc_pin_.set_low();
        return transport_write_single<uint8_t>(cmd);
    }

    [[nodiscard]] IResult<> write_data8(const uint8_t data){
        dc_pin_.set_high();
        return transport_write_single<uint8_t>(data);
    }

    [[nodiscard]] IResult<> write_data16(const uint16_t data){
        dc_pin_.set_high();
        return transport_write_single<uint16_t>(data);
    }

    template<typename T>
    [[nodiscard]] IResult<> write_burst_pixels(std::span<const T> pbuf){
        dc_pin_.set_high();
        return spi_fast_write_burst<uint16_t>(pbuf);
    }

    [[nodiscard]] IResult<> write_repeat_pixels(const auto & data, size_t len){
        dc_pin_.set_high();
        return spi_fast_write_repeat<uint16_t>(data, len);
    }
private:
    hal::Spi & spi_;
    hal::SpiSlaveRank rank_;

    hal::Gpio & dc_pin_;
    Option<hal::Gpio &>res_pin_;

    template <hal::valid_spi_data T>
    [[nodiscard]] IResult<> spi_fast_write_burst(
        const std::span<const auto> pbuf, 
        Continuous cont = DISC) {
        if (const auto res = spi_
            .borrow(rank_); res.is_err()) 
                
            return Err(res.unwrap_err()); 
        if constexpr (sizeof(T) != 1){
            if(const auto res = spi_.set_wordsize(hal::SpiWordSize::TwoBytes); res.is_err())
                
            return Err(res.unwrap_err());
        }

        const auto len = pbuf.size();
        // DEBUG_PRINTLN(len, pbuf[0], static_cast<T>(pbuf[0]));
        for (size_t i = 0; i < len; i++){
            (void)spi_.fast_blocking_write(color_cast<RGB565>(pbuf[i]).to_u16());
            // (void)spi_.blocking_write(static_cast<uint32_t>(p[i]));
        } 

        if (cont == DISC) spi_.lend();

        if constexpr (sizeof(T) != 1) {
            if(const auto res = spi_.set_wordsize(hal::SpiWordSize::OneByte); res.is_err()) 
            return Err(res.unwrap_err());
        }

        return Ok();
    }

    template <hal::valid_spi_data T>
    [[nodiscard]] IResult<> spi_fast_write_repeat(
        const is_stdlayout auto data, 
        const size_t len, 
        Continuous cont = DISC) {
        static_assert(sizeof(T) == sizeof(std::decay_t<decltype(data)>));
        if (const auto res = spi_.borrow(rank_); res.is_err()) 
            return Err(res.unwrap_err()); 
        if constexpr (sizeof(T) != 1){
            if(const auto res = spi_.set_wordsize(hal::SpiWordSize::TwoBytes); res.is_err())
                
            return Err(res.unwrap_err());
        }
        for (size_t i = 0; i < len; i++){
            spi_.blocking_write((data).to_u16());
        } 
        if (cont == DISC) spi_.lend();
        if constexpr (sizeof(T) != 1) {
            if(const auto res = spi_.set_wordsize(hal::SpiWordSize::OneByte); res.is_err()) 
            return Err(res.unwrap_err());
        }
        return Ok();
    }

    template<hal::valid_spi_data T>
    [[nodiscard]] IResult<> transport_write_single(
        const is_stdlayout auto data, 
        Continuous cont = DISC) {
        static_assert(sizeof(T) == sizeof(std::decay_t<decltype(data)>));

        if(const auto res = spi_.borrow(rank_); res.is_err()) 
            return Err(res.unwrap_err());
        if constexpr (sizeof(T) != 1){
            if(const auto res = spi_.set_wordsize(hal::SpiWordSize::TwoBytes); res.is_err())
                
            return Err(res.unwrap_err());
        }

        if constexpr (sizeof(T) == 1) {
            spi_.blocking_write(uint8_t(data));
        } else if constexpr (sizeof(T) == 2) {
            spi_.blocking_write(uint16_t(data));
        }

        if (cont == DISC) spi_.lend();
        if constexpr (sizeof(T) != 1) {
            if(const auto res = spi_.set_wordsize(hal::SpiWordSize::OneByte); res.is_err()) 
            return Err(res.unwrap_err());
        }

        return Ok();
    }

};
}