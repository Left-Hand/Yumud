#pragma once

#include "calibrate_sample.hpp"
#include "core/utils/Result.hpp"

namespace ymd::dsp{


static constexpr size_t MAX_MOTOR_POLE_PAIRS_CNT = 50;

//储存了校准过程中不断提交的新数据
struct CalibrateTable{
    constexpr CalibrateTable(const size_t capacity):
        capacity_(capacity){
            reset();
        }

    [[nodiscard]] constexpr Result<void, void> push_back(const q31 expected, const q31 measured){
        //确定原始数据的扇区
        const auto index = position_to_index(measured);
        // PANIC(index);
        if(index >= capacity_) return Err();
        if(sector_nums_[index] != 0) return Err(); 

        block_[index] = ({
            const auto may_calipoint = PackedCalibrateSample::from_expected_and_measure(
                expected, measured
            );

            if(may_calipoint.is_none()) return Err();
            may_calipoint.unwrap();
        });

        return Ok();
    }

    [[nodiscard]] constexpr size_t position_to_index(const q31 position) const{
        return size_t(q24(position) * capacity_);
    }

    [[nodiscard]] constexpr PackedCalibrateSample operator[](const size_t idx) const {
        return block_[idx];
    }

    [[nodiscard]] constexpr PackedCalibrateSample operator[](const q31 raw_position) const {
        return block_[position_to_index(raw_position)];
    }

    constexpr void reset(){
        block_.fill(PackedCalibrateSample{});
        sector_nums_.fill(0);
    }

    [[nodiscard]] constexpr bool is_full() const {
        return std::accumulate(sector_nums_.begin(), sector_nums_.end(), 0u) == capacity_;
    }

    [[nodiscard]] constexpr std::span<const PackedCalibrateSample>
    iter() const {
        return std::span(block_);
    }


private:
    using Block = std::array<PackedCalibrateSample, MAX_MOTOR_POLE_PAIRS_CNT>;
    using Cnts = std::array<uint8_t, MAX_MOTOR_POLE_PAIRS_CNT>;

    Block block_;
    Cnts sector_nums_;
    size_t capacity_ = 0;
};


}