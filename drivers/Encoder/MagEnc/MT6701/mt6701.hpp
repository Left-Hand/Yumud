#pragma once

#include "mt6701_prelude.hpp"

namespace ymd::drivers{

class MT6701 final:
    public MagEncoderIntf,
    public MT6701_Regs
{
public:
    explicit MT6701(
        Some<hal::I2cBase *> i2c, 
        hal::I2cSlaveAddr<7> addr = DEFAULT_I2C_ADDR
    ):
        transport_(MT6701_Transport(i2c, addr)){;}

    explicit MT6701(MT6701_Transport && phy):
        transport_(std::move(phy)){;}

    ~MT6701(){};

    [[nodiscard]] IResult<> init();

    [[nodiscard]] IResult<> update();

    [[nodiscard]] IResult<Angular<uq32>> read_lap_angle();
    
    [[nodiscard]] IResult<MagStatus> get_mag_status();

    [[nodiscard]] IResult<> enable_uvwmux(const Enable en);

    [[nodiscard]] IResult<> enable_abzmux(const Enable en);

    [[nodiscard]] IResult<> set_direction(const RotateDirection dir);

    [[nodiscard]] IResult<> set_pole_pairs(const uint8_t pole_pairs);

    [[nodiscard]] IResult<> set_abz_resolution(const uint16_t abz_resolution);

    [[nodiscard]] IResult<> set_zero_angle(const Angular<uq32> angle);

    [[nodiscard]] IResult<> set_zero_pulse_width(const ZeroPulseWidth zero_pulse_width);

    [[nodiscard]] IResult<> set_hysteresis(const Hysteresis hysteresis);

    [[nodiscard]] IResult<> enable_fast_mode(const Enable en);

    [[nodiscard]] IResult<> enable_pwm(const Enable en);

    [[nodiscard]] IResult<> set_pwm_polarity(const bool polarity);

    [[nodiscard]] IResult<> set_pwm_freq(const PwmFreq pwm_freq);

    [[nodiscard]] IResult<> set_start_angle(const Angular<uq32> start);

    [[nodiscard]] IResult<> set_stop_angle(const Angular<uq32> stop);
private:

    MT6701_Transport transport_;
    Packet packet_ = {0, 0};
    uq32 lap_position_ = 0;
    bool fast_mode_ = true;


    template<typename T>
    IResult<> read_reg(T & reg){
        return transport_.read_reg(reg);
    }


    template<typename T>
    IResult<> write_reg(const RegCopy<T> & reg){
        return transport_.write_reg(reg);
    }
};

}
