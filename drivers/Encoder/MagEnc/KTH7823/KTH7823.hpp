#pragma once

#include "kth7823_prelude.hpp"

namespace ymd::drivers::kth7823{
class KTH7823 final{
public:
    explicit KTH7823(Some<hal::Spi *> spi, const hal::SpiSlaveRank rank):
        phy_(Phy{spi, rank}){;}

    [[nodiscard]] IResult<> init();
    [[nodiscard]] IResult<> validate();

    [[nodiscard]] IResult<Angular<uq32>> read_lap_angle(){
        return Ok(Angular<uq32>::from_turns(lap_turns_));
    }

    [[nodiscard]] IResult<> update();

    [[nodiscard]] IResult<> set_zero_angle(const Angular<uq32> angle);

    [[nodiscard]] IResult<> set_trim_x(const real_t k);

    [[nodiscard]] IResult<> set_trim_y(const real_t k);

    [[nodiscard]] IResult<> set_trim(const real_t am, const real_t e);

    [[nodiscard]] IResult<> set_mag_threshold(const MagThreshold low, const MagThreshold high);

    [[nodiscard]] IResult<> set_direction(const RotateDirection direction);


    [[nodiscard]] IResult<> set_zero_parameters(const ZeroPulseWidth width, const ZeroPulsePhase phase);

    [[nodiscard]] IResult<> set_pulse_per_turn(const uint16_t ppt);

private:
    Regset regset_ = {};
    Phy phy_;
    uq32 lap_turns_ = 0;

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