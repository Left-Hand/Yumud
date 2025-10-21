#pragma once

#include "ma730_prelude.hpp"

namespace ymd::drivers{


class MA730 final:
    public MA730_Prelude,
    public MagEncoderIntf{
public:
    struct Config{
        ClockDirection direction;
    };

    explicit MA730(const hal::SpiDrv & spi_drv):
        spi_drv_(spi_drv){;}
    explicit MA730(hal::SpiDrv && spi_drv):
        spi_drv_(std::move(spi_drv)){;}
    explicit MA730(Some<hal::Spi *> spi, const hal::SpiSlaveRank rank):
        spi_drv_(hal::SpiDrv(spi, rank)){;}


    [[nodiscard]] IResult<> init(const Config & cfg);
    [[nodiscard]] IResult<> update();

    [[nodiscard]] IResult<> set_zero_angle(const Angle<q31> angle);
    [[nodiscard]] IResult<Angle<q31>> read_lap_angle(){
        return Ok(Angle<q31>::from_turns(lap_angle_));
    }

    [[nodiscard]] IResult<> set_trim_x(const real_t k);

    [[nodiscard]] IResult<> set_trim_y(const real_t k);

    [[nodiscard]] IResult<> set_trim(const real_t am, const real_t e);

    [[nodiscard]] IResult<> set_mag_threshold(
        const MagThreshold low, const MagThreshold high);

    [[nodiscard]] IResult<> set_direction(const ClockDirection direction);
    [[nodiscard]] IResult<MagStatus> get_mag_status();

    [[nodiscard]]
    IResult<> set_zparameters(const PulseWidth width, const PulsePhase phase);

    [[nodiscard]]
    IResult<> set_pulse_per_turn(const uint16_t ppt);
private:
    hal::SpiDrv spi_drv_;
    MA730_Regset regs_ = {};
    q31 lap_angle_ = 0;

    template<typename T>
    [[nodiscard]] IResult<> write_reg(const RegCopy<T> & reg){
        const auto address = T::ADDRESS;
        const uint8_t data = reg.as_val();
        const auto tx = uint16_t(
            0x8000 | (std::bit_cast<uint8_t>(address) << 8) | data);
        if(const auto res = spi_drv_.write_single<uint16_t>(tx);
            res.is_err()) return Err(Error(res.unwrap_err()));
        reg.apply();
        return Ok();
    }


    template<typename T>
    [[nodiscard]] IResult<> read_reg(T & reg){
        uint16_t dummy;
        const auto addr = std::bit_cast<uint8_t>(T::ADDRESS);
        const auto tx = uint16_t(0x4000 | ((uint8_t)addr << 8));
        if(const auto res = spi_drv_.write_single<uint16_t>(tx); 
            res.is_err()) return Err(Error(res.unwrap_err()));
        if(const auto res = spi_drv_.read_single<uint16_t>(dummy);
            res.is_err()) return Err(Error(res.unwrap_err()));
        if((dummy & 0xff) != 0x00) 
            return Err(Error(Error::Kind::InvalidRxFormat));
        reg.as_ref() = (dummy >> 8);
        return Ok();
    }

    [[nodiscard]]
    IResult<uint16_t> direct_read();
    
    [[nodiscard]]
    IResult<uint16_t> get_raw_data();
    
    [[nodiscard]]
    IResult<> set_zero_data(const uint16_t data);
};

};