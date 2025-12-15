#pragma once

#include "core/math/iq/fixed_t.hpp"
#include "calibrate_table.hpp"

namespace ymd::dsp{

struct PositionCorrector{
    struct Config{
        const CalibrateTable & forward_cali_table;
        const CalibrateTable & backward_cali_table;
    };

    explicit PositionCorrector(const Config & cfg):
        forward_cali_table_(cfg.forward_cali_table),
        backward_cali_table_(cfg.backward_cali_table)
    {}

    constexpr Angular<uq32> correct_raw_angle(const Angular<uq32> raw_angle) const {
        const auto corr1 = forward_cali_table_[raw_angle].to_inaccuracy();
        const auto corr2 = backward_cali_table_[raw_angle].to_inaccuracy();

        return Angular<uq32>::from_turns(
            raw_angle.to_turns() + uq32::from_bits((corr1 + corr2).to_bits())
        );
    }

    constexpr auto & forward(){
        return forward_cali_table_;
    }

    constexpr auto & backward(){
        return backward_cali_table_;
    }
private:
    const CalibrateTable & forward_cali_table_;
    const CalibrateTable & backward_cali_table_;
};

}