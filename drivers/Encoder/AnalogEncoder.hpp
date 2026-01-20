#pragma once

#include "Encoder.hpp"

#include "algebra/regions/range2.hpp"

#if 0
namespace ymd::drivers{
class AnalogEncoder final:public EncoderIntf{
public:
    using Error = EncoderError;
    struct Config{
        Range2<real_t> volt_range;
        Range2<real_t> pos_range;
    };

    AnalogEncoder(const Config & cfg, hal::AnalogInIntf & ain):
        ain_(ain),
        volt_range_(cfg.volt_range),
        pos_range_(cfg.pos_range)
    {}

    [[nodiscard]] Result<Angular<uq32>, EncoderError> read_lap_angle(){
        return Ok(Angular<uq32>::from_turns(last_pos_));
    }

    [[nodiscard]] Result<void, EncoderError> update(){
        const auto volt = ain_.get_voltage();
        if(volt > volt_range_.max()) return Err(Error::ValueOverflow);
        if(volt < volt_range_.min()) return Err(Error::ValueUnderflow);
        last_pos_ = pos_range_.lerp(volt_range_.invlerp(volt));
        return Ok();
    }

private:
    hal::AnalogInIntf & ain_;
    Range2<real_t> volt_range_;
    Range2<real_t> pos_range_;
    real_t last_pos_;
};

};
#endif
