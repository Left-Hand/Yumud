#pragma once

#include "core/math/real.hpp"


namespace ymd::foc{

class LapPosAccumulator{
protected:
    bool first_ = true;
    q16 last_lap_pos_ = 0;
    q16 accu_pos_ = 0;
public:
    LapPosAccumulator(){reset();}
    q16 update(q16 lap_pos){
        if(unlikely(first_)){
            last_lap_pos_ = lap_pos;
            first_ = false;
            return 0;
        }

        q16 delta_lap_pos = lap_pos - last_lap_pos_;
        last_lap_pos_ = lap_pos;

        if(delta_lap_pos > q16(0.5)){
            accu_pos_ += (delta_lap_pos - 1);
        }else if (delta_lap_pos < -q16(0.5)){
            accu_pos_ += (delta_lap_pos + 1);
        }else{
            accu_pos_ += delta_lap_pos;
        }

        return accu_pos_;
    }

    void reset(){
        accu_pos_ = 0;
        first_ = true;
    }
};

// class PiController{
// public:
//     struct Config{
//         q16 kp;
//         q16 ki;
//         uint fc;
//     };
// private:
//     q16 kp_;
//     q16 ki_;
//     q16 inte_;
// public:
//     PiController(const Config & config = {})
//         {reconfig(config); reset();}

//     q16 update(q16 x){
//         inte_ += x;
//         inte_= CLAMP(inte_, -10000, 10000);
//         const auto res = CLAMP(x * kp_ + inte_ * ki_, -100, 100);
//         return res;
//     }

//     void reconfig(const Config & config){
//         kp_ = config.kp;
//         ki_ = config.ki / config.fc;
//     }
//     void reset(){
//         inte_ = 0;
//     }

// };


class LapPosPll{
public:
    struct Config{
        q16 kp;
        q16 ki;
        uint fc;
    };
    
// protected:
public:
    static constexpr int shift_bits = 5;

    q16 kp_ = 0;
    q16 ki_ = 0;
    q16 ko_ = 0;

    LapPosAccumulator pos_accumulator_;
    // PiController spd_pi_ctrl_;
    q16 pos_err_int_ = 0;
    q16 spd_est_ = 0;
    // PiController pos_i_ctrl_;
    q16 pos_int_ = 0;
    q16 pos_est_ = 0;

public:

    LapPosPll(const Config & config){
        reconfig(config);
        reset();
    };

    // LapPosPll(const q16 kp, const q16 ki, const uint fc):
    //     LapPosPll(Config{.kp = kp, .ki = ki, .fc = fc}){;}

    q16 update(const q16 lap_pos);

    void reset(){
        pos_accumulator_.reset();
        // spd_pi_ctrl_.reset();

        spd_est_ = 0;
        // pos_i_ctrl_.reset();
        pos_est_ = 0;
    }

    void reconfig(const Config & config){
        kp_ = config.kp;
        ki_ = config.ki;
        ko_ = q16(1 << shift_bits) / config.fc;
        // spd_pi_ctrl_.reconfig({.kp = config.kp, .ki = config.ki, .fc = config.fc});
        // pos_i_ctrl_.reconfig({.kp = 0, .ki = (1 << shift_bits), .fc = config.fc});
    }

    q16 theta() const {return frac(pos_est_) * real_t(TAU);}

    q16 accpos() const {return pos_est_;}
    q16 speed() const {return spd_est_;}

};


class SimplePll{
    protected:
        q16 last_lap_pos = 0;
        q16 err_int_ = 0;
        q16 accu_pos_ = 0;
        q16 pll_pos_ = 0;
    
    public:
        void update(const q16 lap_pos);
        q16 theta() const {return frac(pll_pos_) * real_t(TAU);}
    };
    
}