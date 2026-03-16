#pragma once

#include "kth7823_prelude.hpp"

namespace ymd::drivers::kth7823{
class KTH7823 final{
public:
    explicit KTH7823(Some<hal::Spi *> spi, const hal::SpiSlaveRank rank):
        transport_(Transport{spi, rank}){;}

    IResult<> init();
    IResult<> validate();

    IResult<Angular<uq32>> read_lap_angle(){
        return Ok(Angular<uq32>::from_turns(lap_turns_));
    }

    IResult<> update();

    IResult<> burn_zero_angle(const Angular<uq32> angle);

    IResult<> set_trim(const uq16 x, const uq16 y);

    IResult<> set_mag_threshold(const MagThreshold low, const MagThreshold high);

    IResult<> set_direction(const RotateDirection direction);

    IResult<> set_abz_freq_limit(const AbzFreqLimit freq_lim);


    IResult<> set_zero_parameters(const ZeroPulseWidth width, const ZeroPulsePhase phase);

    IResult<> set_pulse_per_turn(const uint16_t num_ppt);

private:
    Regset regset_ = {};
    Transport transport_;
    uq32 lap_turns_ = 0;

    IResult<> read_reg(const RegAddr reg_addr, uint8_t & reg_val){
        if(const auto res = transport_.read_reg(reg_addr, reg_val);
            res.is_err()) return res;
        return Ok();
    }

    template<typename T>
    IResult<> burn_reg(const RegCopy<T> & reg){
        if(const auto res = transport_.burn_reg(T::REG_ADDR, reg.to_bits()); 
            res.is_err()) return res;
        reg.apply();
        return Ok();
    }
};

};