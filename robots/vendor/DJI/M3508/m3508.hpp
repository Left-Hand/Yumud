#pragma once

#include "hal/bus/can/can.hpp"
#include "drivers/Encoder/odometer.hpp"

#include "dsp/controller/pid_ctrl.hpp"
#include "dsp/filter/rc/LowpassFilter.hpp"

#include "robots/foc/stepper/observer/observer.hpp"

#include <bitset>

namespace ymd::robots{
using namespace ymd::foc;




class M3508Port{
public:
class M3508{
public:
    class M3508Encoder final: public drivers::EncoderIntf{
    public:
        using Error = drivers::EncoderError;

        M3508Encoder(M3508 & owner):owner_(owner){;}
        void init() {}
        Result<void, Error> update(){
            //pass
            return Ok();
        }
        Result<real_t, Error> read_lap_position() {
            return Ok(owner_.lap_position);
        }

    private:
        M3508 & owner_;
    };

    M3508Encoder enc_{*this};
    drivers::Odometer odo_{enc_};
    
    M3508(M3508Port & _port, const size_t _index):
        port(_port), index(_index){reset();}

    auto & operator = (const M3508 & other) = delete;
    auto & operator = (M3508 && other) = delete;


    friend class M3508Port;
    friend class M3508Encoder;

    void apply_target_current(const real_t _curr){port.set_target_current(_curr, index);}

    void update_measurements(const real_t _lap_position, const real_t _curr, const real_t _spd, const real_t _temp);
public:
    void init();
    void tick();
    void reset();
    void set_target_current(const real_t curr);
    void set_target_speed(const real_t spd);
    void set_target_position(const real_t pos);

    real_t get_position() {return odo_.getPosition() / reduction_ratio;}
    real_t get_current() const {return curr;}
    real_t get_speed() const {return speed / reduction_ratio * real_t(2.5);}
    real_t read_temp() const {return temperature;}
    auto delta(){return micros_delta;}
    auto & enc() {return enc_;}
    auto & odo() {return odo_;}
private:
    M3508Port & port;
    const size_t index;
    static constexpr int reduction_ratio = 19;
    
    real_t lap_position = 0;
    real_t curr = 0;
    real_t speed = 0;
    real_t temperature = 0;
    real_t curr_limit = 10;

    using PID = PID_t<real_t>;

    PID spd_pid = {3, 0, 0};
    PID pos_pid = {3, 0, 0};

    enum class CtrlMethod:uint8_t{
        NONE,
        CURR,
        SPD,
        POS
    };


    CtrlMethod ctrl_method = CtrlMethod::NONE;
    
    real_t targ_curr = 0;
    real_t targ_spd = 0;
    real_t targ_pos = 0;

    real_t curr_delta = real_t(0.02);
    real_t curr_setpoint = 0;

    real_t last_t = 0;

    SpeedEstimator::Config spe_config{
        .err_threshold = real_t(0.02),
        .est_freq = 200,
        .max_cycles = 10,
    };
    
    SpeedEstimator spd_ester{spe_config};
    SpeedEstimator targ_spd_ester{spe_config};

    Microseconds last_micros;
    Microseconds micros_delta;

};

    M3508Port(hal::Can & can):can_(can){reset();}
    
    void init();

    void reset();

    void tick();

    M3508 & operator[](const size_t index);
    auto & slaves(){
        return slaves_;
    }


private:

    static constexpr size_t MAX_SLAVES_COUNT = 8;
    
    hal::Can & can_;
    size_t size_ = MAX_SLAVES_COUNT;
    std::bitset<MAX_SLAVES_COUNT> connected_flags_;

    struct TxData{
        uint16_t curr_data_msb[4];  
    }__packed;

    static_assert(sizeof(TxData) == 8);

    struct RxData{
        uint16_t angle_8192_msb;
        uint16_t curr_data_msb;
        uint16_t speed_rpm_msb;
        uint8_t temp;
        uint8_t __resv__;
    }__packed;

    static_assert(sizeof(RxData) == 8);


    std::array<M3508, MAX_SLAVES_COUNT> slaves_ = {
        M3508{*this, 1},
        M3508{*this, 2},
        M3508{*this, 3},
        M3508{*this, 4},
        M3508{*this, 5},
        M3508{*this, 6},
        M3508{*this, 7},
        M3508{*this, 8},
    };
    
    union{
        uint16_t curr_cache[8];
        struct{
            TxData tx_datas[2];
        };
    };
    

    void set_target_current(const real_t curr, const size_t index);

    void update_slave(const hal::CanMsg & msg, const size_t index);

};

};