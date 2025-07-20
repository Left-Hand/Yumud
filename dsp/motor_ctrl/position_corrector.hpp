#pragma once

#include "core/math/iq/iq_t.hpp"
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

    constexpr q16 correct_raw_position(const q16 raw_lap_position) const {
        const auto corr1 = forward_cali_table_[raw_lap_position].to_inaccuracy();
        const auto corr2 = backward_cali_table_[raw_lap_position].to_inaccuracy();

        return raw_lap_position + ((corr1 + corr2) >> 1);
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