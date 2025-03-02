#include "tb.h"

static auto pos_pid = PID_t<real_t>(3.1, 0.5, 1.02, 1.0);
static auto curr_pid = PID_t<real_t>(20.0, 0.0, 0.0, 1.0);
static auto pos2curr_pid = PID_t<real_t>(11.0, 0.05, 1.1, 1.0);
static real_t omega = real_t(0.3);


class AT82XX{
protected:
    TimerOC & forward_pwm;
    TimerOC & backward_pwm;
    TimerOC & vref_pwm;
    bool enabled = true;
    bool softctrl = false;
    bool fastdecay = false;
public:
    AT82XX(TimerOC & _forward_pwm, TimerOC & _backward_pwm, TimerOC & _vref_pwm):
            forward_pwm(_forward_pwm), backward_pwm(_backward_pwm), vref_pwm(_vref_pwm){;}

    void init(){

        // forward_pwm.enableSync();
        // backward_pwm.enableSync();
        // vref_pwm.enableSync();
        if(softctrl){
            forward_pwm.setPolarity(fastdecay);
            backward_pwm.setPolarity(fastdecay);
        }
        // vref_pwm.setPolarity(true);

        forward_pwm.init();
        backward_pwm.init();
    }

    void setCurrentClamp(const real_t abs_max_value){
        vref_pwm = ABS(abs_max_value) / 3.3;
    }

    void enable(const bool en = true){
        enabled = en;
        if(!en){
            // forward_pwm = real_t(1);
            // backward_pwm = real_t(1);
            // vref_pwm = real_t(0);
        }
    }

    void setCurrent(const real_t curr){

        if(!enabled) return;

        if(softctrl){
            if(!fastdecay){
                if(curr > 0){
                    forward_pwm = 0;
                    backward_pwm = curr;
                }else if(curr < 0){
                    forward_pwm = -curr;
                    backward_pwm = 0;
                }else{
                    forward_pwm = 0;
                    backward_pwm = 0;
                }
            }else{
                switch(int(sign(curr))){
                    case -1:
                        forward_pwm = 0;
                        backward_pwm = -curr;
                        break;
                    case 0:
                        forward_pwm = 0;
                        backward_pwm = 0;
                        break;
                    case 1:
                        forward_pwm = curr;
                        backward_pwm = 0;
                        break;
                    default://impossible
                        break;
                }
            }
        }
        
        else{
            setCurrentClamp(ABS(curr));
            if(curr > 0){
                forward_pwm = real_t(0);
                backward_pwm = real_t(1);
            }else{
                forward_pwm = real_t(1);
                backward_pwm = real_t(0);
            }
        }
    }

    real_t getCurrent(){
        scexpr int pga_scale = 16;
        scexpr int res_m_ohms = 100;
        return (real_t(ADC1->IDATAR2 * (3300 / res_m_ohms/32)) / pga_scale / 4) / 32;
        
    }

    AT82XX & operator = (const real_t curr){setCurrent(curr); return *this;}
};

template<typename real>
class MaxFollower_t{
public:
    real last_out = 0;
    size_t since = 0;
    const size_t max_since = 40;
    const real_t ratio = 0.9;
    bool inited = false;
    public:
    real update(const real data){
        since++;
        if(!inited){
            last_out = data;
            since = 0;
            inited = true;
        }else{
            real update_threshold = real(last_out * ratio);
            if(data >= update_threshold){
                last_out = data;
                since = 0;
            }else if(since > max_since){
                last_out = 0;
            }
        }
        return last_out;
    }
};

using MaxFollower = MaxFollower_t<real_t>;


class CurrentGen:public hal::AnalogInIntf{
protected:
    MaxFollower_t<real_t> follower;
    LowpassFilterZ_t<real_t> lpf{0.999};
    real_t data_cache;
public:

    real_t update(const real_t new_data){
        data_cache = lpf.update(follower.update(new_data));
        return this->operator real_t();
    }

    real_t uni() const override{
        return data_cache;
    }
};


void pmdc_tb(){

    uart1.init(115200 * 4);
    IOStream & logger = uart1;
    logger.setSpace(",");
    logger.setEps(4);

    adc1.init(
        {
        },{
            AdcChannelConfig{AdcUtils::Channel::CH3, AdcUtils::SampleCycles::T28_5},
            AdcChannelConfig{AdcUtils::Channel::CH4, AdcUtils::SampleCycles::T28_5},
        });

    adc1.setTrigger(AdcOnChip::RegularTrigger::SW, AdcOnChip::InjectedTrigger::T1TRGO);
    // adc1.enableContinous();
    adc1.enableAutoInject();
    adc1.setPga(AdcOnChip::Pga::X16);
    // adc1.setInjectedTrigger(AdcOnChip::InjectedTrigger::SW);

    // constexpr auto a = F_CPU / 4096;
    timer1.init(4096, 1, TimerUtils::Mode::CenterAlignedUpTrig);
    timer1.enableArrSync();
    TIM_SelectOutputTrigger(TIM1, TIM_TRGOSource_Update);

    auto & pwmL = timer1.oc(1);
    pwmL.init();
    pwmL.enableSync();
    auto & pwmR = timer1.oc(2);
    pwmR.init();
    pwmR.enableSync();

    // timer1.oc(4).init();
    // timer1.oc(4)= 0.99;

    timer3.init(4096, 1, TimerUtils::Mode::CenterAlignedDownTrig);
    timer3.enableArrSync();
    GPIO_PinRemapConfig(TIM3_REMAP, DISABLE);

    auto & pwmRef = timer3.oc(2);
    auto & pwmRef2 = timer3.oc(3);
    pwmRef.init();
    pwmRef = 0.3;
    pwmRef2.init();
    pwmRef2 = 0;
    AT82XX driver{pwmL, pwmR, pwmRef};
    real_t feed_current, filt_current;

    // MaxFollower follower;
    CurrentGen cg;

    adc1.bindCb(AdcUtils::IT::JEOC, [&](){
        feed_current = driver.getCurrent();
        filt_current = cg.update(feed_current);
    });

    adc1.enableIT(AdcUtils::IT::JEOC, {0,0});

    // timer3.bindCb(TimerUtils::IT::Update, [&](){

    // });

    // timer3.enableIt(TimerUtils::IT::Update, {0,0});


    driver.init();
    driver.enable();

    while(true){
        // real_t targ_current = 0.20 + 0.04 * sin(7 * t);
        // real_t targ_current = 0.7 + 0.15 * sin(7 * t);
        real_t targ_current = sin(6 * t);
        driver = targ_current;
        // feed_current = driver.getCurrent();
        // filt_current = follower.update(feed_current);
        // delay(1);
        if(!logger.pending())logger.println(targ_current, feed_current, filt_current);
        // , bool(portA[8]), bool(portA[9]), pwmRef.arr()
        // , filt_current, TIM1->CH1CVR, TIM1->CH2CVR, bool(portA[8]), bool(portA[9]));
    }
}