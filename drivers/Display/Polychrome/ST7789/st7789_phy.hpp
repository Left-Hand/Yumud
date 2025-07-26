#pragma once

#include "st7789_prelude.hpp"

namespace ymd::drivers{


class ST7789_Phy final:
    ST7789_Prelude{
public:
    static constexpr auto COMMAND_LEVEL = LOW;
    static constexpr auto DATA_LEVEL = HIGH;

    template<typename T = void>
    using IResult = Result<void, drivers::DisplayerError>;

    explicit ST7789_Phy(
        Some<hal::SpiHw *> spi,
        const hal::SpiSlaveIndex index,
        Some<hal::Gpio *> dc_gpio, 
        Option<hal::Gpio &> res_gpio = None
    ):  
        spi_(spi.deref()), 
        idx_(index), 
        dc_gpio_(dc_gpio.deref()), 
        res_gpio_(res_gpio)
        {};

    [[nodiscard]] IResult<> init(){
        dc_gpio_.outpp();
        if(res_gpio_.is_some())
            res_gpio_.unwrap().outpp(HIGH);

        return reset();
    }

    [[nodiscard]] IResult<> reset(){
        if(res_gpio_.is_none()) return Ok();
        auto & res_gpio = res_gpio_.unwrap();
        clock::delay(10ms);
        res_gpio.clr();
        clock::delay(10ms);
        res_gpio.set();
        return Ok();
    }

    [[nodiscard]] IResult<> write_command(const uint8_t cmd){
        dc_gpio_.clr();
        return IResult<>(phy_write_single<uint8_t>(cmd));
    }

    [[nodiscard]] IResult<> write_data8(const uint8_t data){
        dc_gpio_.set();
        return IResult<>(phy_write_single<uint8_t>(data));
    }

    [[nodiscard]] IResult<> write_data16(const uint16_t data){
        dc_gpio_.set();
        return IResult<>(phy_write_single<uint16_t>(data));
    }

    template<typename T>
    [[nodiscard]] IResult<> write_burst_pixels(std::span<const T> pbuf){
        return IResult<>(phy_write_burst<uint16_t>(pbuf));
    }

    [[nodiscard]] IResult<> write_repeat_pixels(const auto & data, size_t len){
        dc_gpio_.set();
        return IResult<>(phy_write_repeat<uint16_t>(data, len));
    }
private:
    hal::SpiHw & spi_;
    hal::SpiSlaveIndex idx_;

    hal::Gpio & dc_gpio_;
    Option<hal::Gpio &>res_gpio_;

    template <hal::valid_spi_data T>
    [[nodiscard]] IResult<> phy_write_burst(
        const std::span<const auto> pbuf, 
        Continuous cont = DISC) {
        if (const auto res = spi_
            .begin(idx_.to_req()); res.is_err()) 
                return Err(res.unwrap_err()); 
        if constexpr (sizeof(T) != 1){
            if(const auto res = spi_.set_data_width(magic::type_to_bits_v<T>); res.is_err())
                return Err(res.unwrap_err());
        }

        const auto len = pbuf.size();
        // DEBUG_PRINTLN(len, pbuf[0], static_cast<T>(pbuf[0]));
        for (size_t i = 0; i < len; i++){
            (void)spi_.fast_write(static_cast<RGB565>(pbuf[i]).as_u16());
            // (void)spi_.write(static_cast<uint32_t>(p[i]));
        } 

        if (cont == DISC) spi_.end();

        if constexpr (sizeof(T) != 1) {
            if(const auto res = spi_.set_data_width(8); res.is_err()) return Err(res.unwrap_err());
        }

        return Ok();
    }

    template <hal::valid_spi_data T>
    [[nodiscard]] IResult<> phy_write_repeat(
        const is_stdlayout auto data, 
        const size_t len, 
        Continuous cont = DISC) {
        static_assert(sizeof(T) == sizeof(std::decay_t<decltype(data)>));
        if (const auto res = spi_.begin(idx_.to_req()); res.is_err()) return Err(res.unwrap_err()); 
        if constexpr (sizeof(T) != 1){
            if(const auto res = spi_.set_data_width(sizeof(T) * 8); res.is_err())
                return Err(res.unwrap_err());
        }
        for (size_t i = 0; i < len; i++){
            if(const auto res = spi_.write((data).as_u16());
                res.is_err()) return Err(res.unwrap_err());
        } 
        if (cont == DISC) spi_.end();
        if constexpr (sizeof(T) != 1) {
            if(const auto res = spi_.set_data_width(8); res.is_err()) return Err(res.unwrap_err());
        }
        return Ok();
    }

    template<hal::valid_spi_data T>
    [[nodiscard]] IResult<> phy_write_single(
        const is_stdlayout auto data, 
        Continuous cont = DISC) {
        static_assert(sizeof(T) == sizeof(std::decay_t<decltype(data)>));

        if(const auto res = spi_.begin(idx_.to_req()); res.is_err()) return Err(res.unwrap_err());
        if constexpr (sizeof(T) != 1){
            if(const auto res = spi_.set_data_width(sizeof(T) * 8); res.is_err())
                return Err(res.unwrap_err());
        }

        if constexpr (sizeof(T) == 1) {
            if(const auto res = spi_.write(uint8_t(data)); res.is_err()) return Err(res.unwrap_err());
        } else if constexpr (sizeof(T) == 2) {
            if(const auto res = spi_.write(uint16_t(data)); res.is_err()) return Err(res.unwrap_err());
        }

        if (cont == DISC) spi_.end();
        if constexpr (sizeof(T) != 1) {
            if(const auto res = spi_.set_data_width(8); res.is_err()) return Err(res.unwrap_err());
        }

        return Ok();
    }

};
}