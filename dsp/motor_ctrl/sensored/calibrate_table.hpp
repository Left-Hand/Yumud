#pragma once

#include "calibrate_sample.hpp"
#include "core/utils/Result.hpp"

namespace ymd::dsp{


static constexpr size_t MAX_MOTOR_POLE_PAIRS_CNT = 50;



//储存了校准过程中不断提交的新数据
struct CalibrateTable{
    using Error = CalibrateError;
    constexpr explicit CalibrateTable(const size_t capacity):
        capacity_(capacity){
            reset();
        }

    [[nodiscard]] constexpr Result<void, Error> push_back(
        const Angular<uq32> expected, 
        const Angular<uq32> measured
    ){
        //确定原始数据的扇区
        const auto ind = angle_to_index(measured);
        if(ind >= capacity_) 
            return Err(Error::SampleIndexOutOfRange);//unreachable

        if(sector_collected_nums_[ind] != 0) 
            return Err(Error::DuplicatedSectorSampleDetected); 

        block_[ind] = ({
            const auto may_calipoint = PackedCalibrateSample::from_expected_and_measure(
                expected, measured
            );

            if(may_calipoint.is_none()) 
                return Err(Error::ExpectedAndMeasuredExceedLimit);
            may_calipoint.unwrap();
        });

        return Ok();
    }



    [[nodiscard]] constexpr PackedCalibrateSample operator[](const size_t idx) const {
        return block_[idx];
    }

    [[nodiscard]] constexpr PackedCalibrateSample operator[](const Angular<uq32> raw_angle) const {
        return block_[angle_to_index(raw_angle)];
    }

    constexpr void reset(){
        block_.fill(PackedCalibrateSample{});
        sector_collected_nums_.fill(0);
    }

    [[nodiscard]] constexpr bool is_full() const {
        return std::accumulate(sector_collected_nums_.begin(), sector_collected_nums_.end(), 0u) == capacity_;
    }

    [[nodiscard]] constexpr std::span<const PackedCalibrateSample>
    view() const {return std::span(block_);}

    [[nodiscard]] constexpr size_t angle_to_index(const Angular<uq32> angle) const{
        return size_t(uq24(angle.to_turns()) * capacity_);
    }
private:
    using Block = std::array<PackedCalibrateSample, MAX_MOTOR_POLE_PAIRS_CNT>;
    using Cnts = std::array<uint8_t, MAX_MOTOR_POLE_PAIRS_CNT>;

    Block block_;
    Cnts sector_collected_nums_;
    size_t capacity_ = 0;
};


}