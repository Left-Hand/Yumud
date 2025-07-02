#pragma once

#include <algorithm>
#include <array>
#include <tuple>


#include "core/math/real.hpp"
#include "core/utils/Result.hpp"
#include "config.hpp"

namespace ymd{

struct LapPosition{
    real_t position;
};

struct RawLapPosition{
    real_t position;
};

struct Elecrad{
    real_t elecrad;
};




//储存了压缩保存的期望数据和实际数据 用于校准点分析
struct PackedCalibratePoint{
public:
    static constexpr size_t IGNORE_BITS = 5;
    // static constexpr auto MAX_ERR = q31(q30(1.0 / (1 << IGNORE_BITS)));
    static constexpr auto MAX_ERR = q31(1.0 / 5);
    // static constexpr q24 MAX_ERR = q24(1.0 / (1 << IGNORE_BITS));

    constexpr PackedCalibratePoint():
        targ_packed_data_(0),
        meas_packed_data_(0){;}

    static constexpr Option<PackedCalibratePoint> 
    from_targ_and_meas(const q31 targ, const q31 meas){
        const auto targ_packed_data = ({
            const auto opt = real_to_packed(targ);
            if(opt.is_none()) return None;
            opt.unwrap();
        });

        const auto meas_packed_data = ({
            const auto opt = real_to_packed(meas);
            if(opt.is_none()) return None;
            opt.unwrap();
        });

        return Some(PackedCalibratePoint(
            targ_packed_data,
            meas_packed_data
        ));
    }

    constexpr q31 get_targ() const {
        return packed_to_real(targ_packed_data_);
    }

    constexpr q31 get_meas() const {
        return packed_to_real(meas_packed_data_);
    }

    constexpr q31 to_inaccuracy() const {
        return fposmodp(q20(get_targ() - get_meas()), 0.02_q20);
    }

    template <size_t N>
    constexpr auto get() const {
        if constexpr (N == 0) return get_targ();
        else if constexpr (N == 1) return get_meas();
    }
private:
    constexpr PackedCalibratePoint(
        const uint16_t targ_packed, 
        const uint16_t meas_packed
    ):
        targ_packed_data_(targ_packed),
        meas_packed_data_(meas_packed){;}

    uint16_t targ_packed_data_;
    uint16_t meas_packed_data_;

    static constexpr Option<uint16_t> real_to_packed(const q16 unpacked){
        return Some(uint16_t(unpacked.to_i32()));
    }

    static constexpr q31 packed_to_real(const uint16_t packed){
        return q16::from_i32(packed);
    }
};

// Specialize tuple traits

//储存了完整的校准数据
using CalibrateDataBlock = std::array<PackedCalibratePoint, MOTOR_POLE_PAIRS>;


//校准数据的视图
struct CalibrateDataBlockView{
    using Block = CalibrateDataBlock;

    struct Iterator {
        using iterator_category = std::contiguous_iterator_tag;
        using value_type = PackedCalibratePoint;
        using difference_type = std::ptrdiff_t;
        using pointer = const PackedCalibratePoint*;
        using reference = const PackedCalibratePoint&;

        constexpr explicit Iterator(
            const PackedCalibratePoint* data,
            size_t index
        ) : 
            data_(data), 
            index_(index){}

        constexpr reference operator*() const { 
            return data_[index_];
        }

        constexpr Iterator& operator++() {
            ++index_;
            return *this;
        }

        constexpr bool operator!=(const Iterator& other) const {
            return index_ != other.index_;
        }

    private:
        const PackedCalibratePoint* data_;
        size_t index_;
    };

    constexpr explicit CalibrateDataBlockView(const Block & block):
        block_(block){;}
    
    constexpr Iterator begin() const noexcept {
        return Iterator(block_.data(), 0);
    }

    constexpr Iterator end() const noexcept {
        return Iterator(block_.data(), block_.size());
    }

private:
    const Block & block_;
};

//储存了校准过程中不断提交的新数据
struct CalibrateDataVector{
    constexpr CalibrateDataVector(const size_t capacity):
        capacity_(capacity){
            reset();
        }
    constexpr Result<void, void> push_back(const q31 targ, const q31 meas){
        //确定原始数据的扇区
        const auto index = position_to_index(meas);
        // PANIC(index);
        if(index >= capacity_) return Err();
        if(cnts_[index] != 0) return Err(); 

        block_[index] = ({
            const auto opt = PackedCalibratePoint::from_targ_and_meas(
                targ, meas
            );

            if(opt.is_none()) return Err();
            opt.unwrap();
        });

        return Ok();
    }

    constexpr size_t position_to_index(const q31 position) const{
        return size_t(q24(position) * capacity_);
    }

    constexpr PackedCalibratePoint operator[](const size_t idx) const {
        return block_[idx];
    }

    constexpr PackedCalibratePoint operator[](const q31 raw_position) const {
        return block_[position_to_index(raw_position)];
    }

    constexpr void reset(){
        block_.fill(PackedCalibratePoint{});
        cnts_.fill(0);
    }

    constexpr bool is_full() const {
        // return cnt_ == capacity_;
        return std::accumulate(cnts_.begin(), cnts_.end(), 0u) == capacity_;
    }

    constexpr CalibrateDataBlockView
    as_view() const {
        return CalibrateDataBlockView(block_);
    }


private:
    using Block = CalibrateDataBlock;
    using Cnts = std::array<uint8_t, MOTOR_POLE_PAIRS>;

    Block block_;
    Cnts cnts_;
    size_t capacity_ = 0;
};
}

namespace std {
    template <>
    struct tuple_size<::ymd::PackedCalibratePoint> 
        : integral_constant<size_t, 2> {};

    template <size_t N>
    struct tuple_element<N, ::ymd::PackedCalibratePoint> {
        using type = ymd::q31;
    };
}
