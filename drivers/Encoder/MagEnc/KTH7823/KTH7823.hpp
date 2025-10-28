#pragma once

#include "kth7823_prelude.hpp"

namespace ymd::drivers{
class KTH7823 final:
    public MagEncoderIntf, 
    public KTH7823_Regs{
public:
    using Phy = KTH7823_Phy;
    explicit KTH7823(Some<hal::Spi *> spi, const hal::SpiSlaveRank rank):
        phy_(Phy{spi, rank}){;}

    [[nodiscard]] IResult<> init();
    [[nodiscard]] IResult<> validate();

    [[nodiscard]] IResult<Angle<q31>> read_lap_angle(){
        return Ok(Angle<q31>::from_turns(lap_turns_));
    }

    [[nodiscard]] IResult<> update();

    [[nodiscard]] IResult<> set_zero_angle(const Angle<q31> angle);

    [[nodiscard]] IResult<> set_trim_x(const real_t k);

    [[nodiscard]] IResult<> set_trim_y(const real_t k);

    [[nodiscard]] IResult<> set_trim(const real_t am, const real_t e);

    [[nodiscard]] IResult<> set_mag_threshold(const MagThreshold low, const MagThreshold high);

    [[nodiscard]] IResult<> set_direction(const ClockDirection direction);

    [[nodiscard]] IResult<MagStatus> get_mag_status();

    [[nodiscard]] IResult<> set_zero_parameters(const ZeroPulseWidth width, const ZeroPulsePhase phase);

    [[nodiscard]] IResult<> set_pulse_per_turn(const uint16_t ppt);

private:

    Phy phy_;
    real_t lap_turns_ = 0;

    [[nodiscard]]
    IResult<> write_reg(const RegAddr addr, uint8_t data);

    [[nodiscard]]
    IResult<> read_reg(const RegAddr addr, uint8_t & reg);

    [[nodiscard]]
    IResult<> direct_read(uint16_t & data);
    
    [[nodiscard]]
    IResult<uint16_t> get_raw_data();
    
    [[nodiscard]]
    IResult<> set_zero_data(const uint16_t data);
};

};