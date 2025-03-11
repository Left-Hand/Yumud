#pragma once

#include "sys/math/real.hpp"


namespace ymd::foc{

class LapPosAccumulator{
protected:
    bool first_;
    iq_t<16> last_lap_pos_;
    iq_t<16> accu_pos_;
public:
    LapPosAccumulator(){reset();}
    iq_t<16> update(iq_t<16> lap_pos){
        if(unlikely(first_)){
            last_lap_pos_ = lap_pos;
            first_ = false;
            return 0;
        }

        iq_t<16> delta_lap_pos = lap_pos - last_lap_pos_;
        last_lap_pos_ = lap_pos;

        if(delta_lap_pos > iq_t<16>(0.5)){
            accu_pos_ += (delta_lap_pos - 1);
        }else if (delta_lap_pos < -iq_t<16>(0.5)){
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
//         iq_t<16> kp;
//         iq_t<16> ki;
//         uint fc;
//     };
// private:
//     iq_t<16> kp_;
//     iq_t<16> ki_;
//     iq_t<16> inte_;
// public:
//     PiController(const Config & config = {})
//         {reconfig(config); reset();}

//     iq_t<16> update(iq_t<16> x){
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
        iq_t<16> kp;
        iq_t<16> ki;
        uint fc;
    };
    
// protected:
public:
    scexpr int shift_bits = 5;

    iq_t<16> kp_;
    iq_t<16> ki_;
    iq_t<16> ko_;

    LapPosAccumulator pos_accumulator_;
    // PiController spd_pi_ctrl_;
    iq_t<16> pos_err_int_;
    iq_t<16> spd_est_;
    // PiController pos_i_ctrl_;
    iq_t<16> pos_int_;
    iq_t<16> pos_est_;

public:

    LapPosPll(const Config & config){
        reconfig(config);
        reset();
    };

    // LapPosPll(const iq_t<16> kp, const iq_t<16> ki, const uint fc):
    //     LapPosPll(Config{.kp = kp, .ki = ki, .fc = fc}){;}

    iq_t<16> update(const iq_t<16> lap_pos);

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
        ko_ = iq_t<16>(1 << shift_bits) / config.fc;
        // spd_pi_ctrl_.reconfig({.kp = config.kp, .ki = config.ki, .fc = config.fc});
        // pos_i_ctrl_.reconfig({.kp = 0, .ki = (1 << shift_bits), .fc = config.fc});
    }

    iq_t<16> theta() const {return frac(pos_est_) * real_t(TAU);}

    iq_t<16> accpos() const {return pos_est_;}
    iq_t<16> speed() const {return spd_est_;}

};


class SimplePll{
    protected:
        iq_t<16> last_lap_pos = 0;
        iq_t<16> err_int_ = 0;
        iq_t<16> accu_pos_ = 0;
        iq_t<16> pll_pos_ = 0;
    
    public:
        void update(const iq_t<16> lap_pos);
        iq_t<16> theta() const {return frac(pll_pos_) * real_t(TAU);}
    };
    
}