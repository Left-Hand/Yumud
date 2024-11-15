#pragma once

#include "hal/bus/can/can.hpp"
#include "drivers/Encoder/odometer.hpp"

#include "dsp/controller/PID.hpp"
#include "dsp/filter/LowpassFilter.hpp"

#include "robots/foc/stepper/observer/observer.hpp"

#include <bitset>

namespace ymd::drivers{
using namespace ymd::foc;
class M3508Port{
public:
    class M3508{
    protected:
        M3508Port & port;
        const size_t index;
        scexpr int reduction_ratio = 19;
        
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

        uint32_t last_micros;
        uint32_t micros_delta;


        class M3508Encoder : public Encoder{
        protected:
            M3508 & owner;

        public:
            M3508Encoder(M3508 & _owner):owner(_owner){;}
            virtual void init() override {}
            virtual void update() override{}
            virtual real_t getLapPosition() override {return owner.lap_position;}
            virtual bool stable() override {return true;}
        };

        M3508Encoder enc_{*this};
        Odometer odo_{enc_};
        
        M3508(M3508Port & _port, const size_t _index):
            port(_port), index(_index){reset();}

        auto & operator = (const M3508 & other) = delete;
        auto & operator = (M3508 && other) = delete;



        friend class M3508Port;
        friend class M3508Encoder;

        void applyTargetCurrent(const real_t _curr){port.setTargetCurrent(_curr, index);}

        void updateMeasurements(const real_t _lap_position, const real_t _curr, const real_t _spd, const real_t _temp);
    public:
        void init();
        void tick();
        void reset();
        void setTargetCurrent(const real_t curr);
        void setTargetSpeed(const real_t spd);
        void setTargetPosition(const real_t pos);

        real_t getPosition() {return odo_.getPosition() / reduction_ratio;}
        real_t getCurrent() const {return curr;}
        real_t getSpeed() const {return speed / reduction_ratio * real_t(2.5);}
        real_t getTemperature() const {return temperature;}
        uint32_t delta(){return micros_delta;}
        auto & enc() {return enc_;}
        auto & odo() {return odo_;}
    };
protected:
    #define M3508_CHECK_INDEX if(index > size or index == 0) PANIC(); 
    scexpr size_t max_size = 8;
    
    Can & can;
    size_t size = 8;
    std::bitset<8> occupation;

    // using CurrData_t = int16_t;
    // using AngleData_t = uint16_t;
    // using SpeedData_t = int16_t;
    
    struct TxData{
        uint16_t curr_data_msb[4];  
    }__packed;

    struct RxData{
        uint16_t angle_8192_msb;
        uint16_t curr_data_msb;
        uint16_t speed_rpm_msb;
        uint8_t temp;
        uint8_t __resv__;
    }__packed;

    scexpr uint16_t curr_to_currdata(const real_t curr){
        int16_t temp = int16_t((curr / 20)* 16384);
        return BSWAP_16(temp);
    }

    scexpr real_t currdata_to_curr(const uint16_t currdata_msb){
        int16_t currdata = BSWAP_16(currdata_msb);
        return (real_t(currdata) / 16384) * 20;
    };

    std::array<M3508, max_size> inst_ = {
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
    
    void setTargetCurrent(const real_t curr, const size_t index){
        M3508_CHECK_INDEX
        curr_cache[index - 1] = curr_to_currdata(curr);
    }

    void updateInst(const CanMsg & msg, const size_t index){
        M3508_CHECK_INDEX
        auto rx_data = RxData(msg);
        auto & inst = inst_[index - 1];

        inst.updateMeasurements(
            (real_t(uint16_t(BSWAP_16(rx_data.angle_8192_msb))) >> 13),
            currdata_to_curr(rx_data.curr_data_msb),
            real_t(int16_t(BSWAP_16(rx_data.speed_rpm_msb))) / 60,
            rx_data.temp);
    }
public:
    M3508Port(Can & _can):can(_can){reset();}
    
    void init();

    void reset();

    void tick();

    M3508 & operator[](const size_t index){
        M3508_CHECK_INDEX
        return inst_[index - 1];
    }

    auto & insts(){
        return inst_;
    }

    #undef M3508_CHECK_INDEX
};

};