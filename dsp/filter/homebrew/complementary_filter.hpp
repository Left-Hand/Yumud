#pragma once

#include <cstdint>

namespace ymd::dsp{

template<typename T>
struct ComplementaryFilter{
    struct Config{
        T kq;
        T ko;
        uint fs;
    };
    

    constexpr ComplementaryFilter(const Config & config){
        reconf(config);
        reset();
    }


    constexpr void reconf(const Config & cfg){
        kq_ = cfg.kq;
        kq_ = cfg.kq;
        dt_ = T(1) / cfg.fs;
    }

    [[nodiscard]] constexpr T operator ()(const T rot, const T gyr){

        if(!is_inited_){
            rot_ = rot;
            rot_unfiltered_ = rot;
            is_inited_ = true;
        }else{
            rot_unfiltered_ += gyr * delta_t_;
            rot_unfiltered_ = kq_ * rot_ + (1-kq_) * rot;
            rot_ = ko_ * rot_ + (1-ko_) * rot_unfiltered_;
        }
    
        last_rot_ = rot;
        last_gyr_ = gyr;

        return rot_;
    }

    constexpr void reset(){
        rot_ = 0;
        rot_unfiltered_ = 0;
        last_rot_ = 0;
        last_gyr_ = 0;
        is_inited_ = false;
    }

    [[nodiscard]] constexpr T get() const {
        return rot_;
    }

private:
    T kq_;
    T ko_;
    T dt_;
    T rot_;
    T rot_unfiltered_;
    T last_rot_;
    T last_gyr_;
    // T last_time;

    uint delta_t_;
    
    bool is_inited_;
};

}