#pragma once

#include "Encoder.hpp"
#include "concept/analog_channel.hpp"
#include "types/range/range.hpp"

namespace ymd::drivers{
class AnalogEncoder final:public EncoderIntf{
public:
    using Error = EncoderError;
    struct Config{
        Range2_t<real_t> volt_range;
        Range2_t<real_t> pos_range;
    };

    AnalogEncoder(const Config & cfg, hal::AnalogInIntf & ain):
        ain_(ain),
        volt_range_(cfg.volt_range),
        pos_range_(cfg.pos_range)
    {}

    [[nodiscard]] Result<real_t, EncoderError> get_lap_position(){
        return Ok(last_pos_);
    }

    [[nodiscard]] Result<void, EncoderError> update(){
        const auto volt = ain_.get_voltage();
        if(volt > volt_range_.max()) return Err(Error::ValueOverflow);
        if(volt < volt_range_.min()) return Err(Error::ValueUnderflow);
        last_pos_ = pos_range_.lerp(volt_range_.invlerp(volt));
        return Ok();
    }

    [[nodiscard]] Result<bool, EncoderError> is_stable(){
        return Ok(true);
    }

private:
    hal::AnalogInIntf & ain_;
    Range2_t<real_t> volt_range_;
    Range2_t<real_t> pos_range_;
    real_t last_pos_;
};

};
