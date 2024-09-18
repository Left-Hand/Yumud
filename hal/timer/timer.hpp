#pragma once

#include "timer_oc.hpp"
#include "timer_utils.hpp"

#ifdef HDW_SXX32
class TimerHw{};

class BasicTimer:public TimerHw{
protected:
    TIM_TypeDef * instance;
    // std::array<std::function<void(void)>, 8> cbs;

    uint getClk();
    void enableRcc();

public:
    using IT = TimerUtils::IT;
    using Mode = TimerUtils::Mode;
    
    BasicTimer(TIM_TypeDef * _base):instance(_base){;}

    void init(const uint32_t ferq, const Mode mode = Mode::Up, const bool en = true);
    void init(const uint16_t period, const uint16_t cycle, const Mode mode = Mode::Up, const bool en = true);
    void enable(const bool en = true);

    void enableIt(const IT it,const NvicPriority request, const bool en = true){
        NvicPriority::enable(request, ItToIrq(instance, it), en);
        TIM_ITConfig(instance, (uint16_t)it, (FunctionalState)en);
    }
    void enableArrSync(const bool _sync = true){TIM_ARRPreloadConfig(instance, (FunctionalState)_sync);}

    volatile uint16_t & cnt(){return instance->CNT;}
    volatile uint16_t & arr(){return instance->ATRLR;}

    virtual void bindCb(const IT ch, std::function<void(void)> && cb) = 0;

    BasicTimer & operator = (const real_t duty){instance->CNT = uint16_t(instance->ATRLR * duty); return *this;}
};

class GenericTimer:public BasicTimer{
protected:
    TimerOC channels[4];
public:
    GenericTimer(TIM_TypeDef * _base):BasicTimer(_base),
            channels{
                TimerOC(instance, TimerChannel::Channel::CH1),
                TimerOC(instance, TimerChannel::Channel::CH2),
                TimerOC(instance, TimerChannel::Channel::CH3),
                TimerOC(instance, TimerChannel::Channel::CH4)
            }{;}

    void initAsEncoder(const Mode mode = Mode::Up);
    void enableSingle(const bool _single = true);
    TimerChannel & ch(const int index){return channels[CLAMP(index, 1, 4) - 1];}
    TimerOC & oc(const int index){return channels[CLAMP(index, 1, 4) - 1];}

    virtual TimerChannel & operator [](const int index){return ch(index);}
    virtual TimerChannel & operator [](const TimerChannel::Channel channel){return channels[(uint8_t)channel >> 1];}

    GenericTimer & operator = (const real_t duty){instance->CNT = uint16_t(instance->ATRLR * duty); return *this;}
};

class AdvancedTimer:public GenericTimer{
protected:
    uint8_t calculateDeadzone(const uint deadzone_ns);

    TimerOCN n_channels[3];
public:
    using LockLevel = TimerUtils::BdtrLockLevel;

    AdvancedTimer(TIM_TypeDef * _base):GenericTimer(_base),
            n_channels{
                TimerOCN(instance, TimerChannel::Channel::CH1N),
                TimerOCN(instance, TimerChannel::Channel::CH2N),
                TimerOCN(instance, TimerChannel::Channel::CH3N),
            }{;}

    void initBdtr(const uint32_t ns = 200, const LockLevel level = LockLevel::Off);
    void enableCvrSync(const bool _sync = true){TIM_CCPreloadControl(instance, (FunctionalState)_sync);}
    void setDeadZone(const uint32_t ns);

    TimerChannel & operator [](const int index) override{
        bool is_co = index < 0;
        if(is_co) return n_channels[CLAMP(-index, 1, 3) - 1];
        else return channels[CLAMP(index, 1, 4) - 1];
    }

    TimerChannel & operator [](const TimerChannel::Channel ch) override {
        bool is_co = (uint8_t) ch & 0b1;
        if(is_co){
            return n_channels[((uint8_t)ch - 1) >> 1];
        }else{
            return channels[(uint8_t)ch >> 1];
        }
    }

    TimerOCN & ocn(const int index){return n_channels[CLAMP(index, 1, 3) - 1];}


    AdvancedTimer & operator = (const real_t duty){instance->CNT = uint16_t(instance->ATRLR * duty); return *this;}
};

#endif
