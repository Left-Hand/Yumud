#pragma once

#include "ma730_prelude.hpp"

namespace ymd::drivers{


class MA730 final:public MA730_Prelude{
public:
    struct Config{
        RotateDirection direction;
    };

    explicit MA730(const hal::SpiDrv & spi_drv):
        spi_drv_(spi_drv){;}
    explicit MA730(hal::SpiDrv && spi_drv):
        spi_drv_(std::move(spi_drv)){;}
    explicit MA730(Some<hal::Spi *> spi, const hal::SpiSlaveRank rank):
        spi_drv_(hal::SpiDrv(spi, rank)){;}


    [[nodiscard]] IResult<> init(const Config & cfg);
    [[nodiscard]] IResult<> update();

    [[nodiscard]] IResult<> set_zero_angle(const Angular<uq32> angle);
    [[nodiscard]] IResult<Angular<uq32>> read_lap_angle(){
        return Ok(Angular<uq32>::from_turns(lap_turns_));
    }

    [[nodiscard]] IResult<> set_trim_x(const uq16 k);

    [[nodiscard]] IResult<> set_trim_y(const uq16 k);

    [[nodiscard]] IResult<> set_trim(const uq16 am, const uq16 e);

    [[nodiscard]] IResult<> set_mag_threshold(
        const MagThreshold low, const MagThreshold high);

    [[nodiscard]] IResult<> set_direction(const RotateDirection direction);
    [[nodiscard]] IResult<EncoderFaultBitFields> get_fault();

    [[nodiscard]] IResult<> 
    set_zero_parameters(const ZeroPulseWidth width, const ZeroPulsePhase phase);

    [[nodiscard]] IResult<> 
    set_pulse_per_turn(const uint16_t ppt);
private:
    hal::SpiDrv spi_drv_;
    MA730_Regset regs_ = {};
    uq32 lap_turns_ = 0;

    template<typename T>
    [[nodiscard]] IResult<> write_reg(const RegCopy<T> & reg){
        uint16_t tx_bits = uint16_t(0x8000);
        tx_bits |= (static_cast<uint16_t>(T::ADDRESS) << 8);
        tx_bits |= reg.to_bits();

        if(const auto res = spi_drv_.write_single<uint16_t>(tx_bits);
            res.is_err()) return Err(Error(res.unwrap_err()));
        reg.apply();
        return Ok();
    }


    template<typename T>
    [[nodiscard]] IResult<> read_reg(T & reg){
        uint16_t rx_bits;

        uint16_t tx_bits = uint16_t(0x4000);
        tx_bits |= (static_cast<uint16_t>(T::ADDRESS) << 8);

        if(const auto res = spi_drv_.write_single<uint16_t>(tx_bits); 
            res.is_err()) return Err(Error(res.unwrap_err()));
        if(const auto res = spi_drv_.read_single<uint16_t>(rx_bits);
            res.is_err()) return Err(Error(res.unwrap_err()));
        if((rx_bits & 0xff) != 0x00) 
            return Err(Error(Error::Kind::InvalidRxFormat));
        reg.as_bits_mut() = (rx_bits >> 8);
        return Ok();
    }

    [[nodiscard]]
    IResult<uint16_t> direct_read();
    
    [[nodiscard]]
    IResult<uint16_t> get_raw_data();
    
};

};