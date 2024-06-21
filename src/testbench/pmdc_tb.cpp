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
    bool hardmode = true;
public:
    AT82XX(TimerOC & _forward_pwm, TimerOC & _backward_pwm, TimerOC & _vref_pwm):
            forward_pwm(_forward_pwm), backward_pwm(_backward_pwm), vref_pwm(_vref_pwm){;}

    void init(){

        // forward_pwm.enableSync();
        // backward_pwm.enableSync();
        // vref_pwm.enableSync();

        forward_pwm.setPolarity(false);
        backward_pwm.setPolarity(false);
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

        if(hardmode){
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
        }
        
        else{
            if(curr > 0){
                forward_pwm = real_t(0);
                backward_pwm = real_t(1);
                vref_pwm = curr;
            }else{
                forward_pwm = real_t(1);
                backward_pwm = real_t(0);
                vref_pwm = -curr;
            }
        }
    }

    real_t getCurrent(){
        static constexpr int pga_scale = 4;
        static constexpr int res_m_ohms = 100;
        return real_t((ADC1->IDATAR2 * 3300 / 4096) / pga_scale) / res_m_ohms;
        
    }

    AT82XX & operator = (const real_t curr){setCurrent(curr); return *this;}
};

template<typename real>
class MaxFollower_t{
    // protected:
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
            if(data >= last_out * ratio){
                last_out = data;
                since = 0;
            }else if(since > max_since){
                last_out = 0;
            }
        }
        return last_out;
    }
};

using maxFollower = MaxFollower_t<real_t>;


class CurrentGen:public AnalogInChannel{
protected:
    MaxFollower_t<real_t> follower;
    LowpassFilterZ_t<real_t> lpf{0.999};
    real_t cache;
public:

    void update(const real_t new_data){
        cache = lpf.update(follower.update(new_data));
    }
    real_t uni() const override{
        return cache;
    }
};


void pmdc_tb(){

    uart1.init(115200 * 8);
    IOStream & logger = uart1;
    logger.setSpace(",");
    logger.setEps(4);

    adc1.init(
        {
        },{
            AdcChannelConfig{AdcUtils::Channel::CH3, AdcUtils::SampleCycles::T239_5},
            AdcChannelConfig{AdcUtils::Channel::CH4, AdcUtils::SampleCycles::T239_5},
        });

    adc1.setTrigger(AdcOnChip::RegularTrigger::SW, AdcOnChip::InjectedTrigger::T1CC4);
    adc1.enableContinous();
    adc1.enableAutoInject();
    adc1.setPga(AdcOnChip::Pga::X4);

    // RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
    // RCC_ADCCLKConfig(RCC_PCLK2_Div8);

    // ADC_DeInit(ADC1);
    // ADC_InitTypeDef ADC_InitStructure;
    // ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
    // ADC_InitStructure.ADC_ScanConvMode = DISABLE;
    // ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
    // // ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T1_CC3;
    // ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
    // ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
    // ADC_InitStructure.ADC_NbrOfChannel = 1;
    // ADC_InitStructure.ADC_Pga = ADC_Pga_4;
    // ADC_Init(ADC1, &ADC_InitStructure);

    // ADC_RegularChannelConfig(ADC1, ADC_Channel_TempSensor, 1, ADC_SampleTime_239Cycles5);
    
    // ADC_InjectedSequencerLengthConfig(ADC1, 2);
    // ADC_InjectedChannelConfig(ADC1,ADC_Channel_3,1,ADC_SampleTime_239Cycles5);
    // ADC_InjectedChannelConfig(ADC1,ADC_Channel_4,2,ADC_SampleTime_239Cycles5);

    // ADC_ExternalTrigInjectedConvConfig(ADC1, ADC_ExternalTrigInjecConv_T1_CC4);
    // // ADC_ExternalTrigInjectedConvEdgeConfig(ADC1,adcexternaltriginjeced);
    // ADC_ExternalTrigInjectedConvCmd(ADC1, ENABLE);


    timer1.init(1024, 1, TimerUtils::Mode::CenterAlignedUpTrig);
    timer1.enableArrSync();
    // TIM_SelectOutputTrigger(TIM1, TIM_TRGOSource_Update);

    auto & pwmL = timer1.oc(1);
    pwmL.init();
    pwmL.enableSync();
    auto & pwmR = timer1.oc(2);
    pwmR.init();
    pwmR.enableSync();

    // timer1.oc(4).init();
    // timer1.oc(4)= 0.99;

    timer3.init(1024, 1, TimerUtils::Mode::CenterAlignedDownTrig);
    timer3.enableArrSync();
    GPIO_PinRemapConfig(TIM3_REMAP, DISABLE);

    auto & pwmRef = timer3.oc(2);
    auto & pwmRef2 = timer3.oc(3);
    pwmRef.init();
    pwmRef = 0.3;
    pwmRef2.init();
    pwmRef2 = 0.2;
    portA[7].OutAfPP();
    portA[7] = false;
    AT82XX driver{pwmL, pwmR, pwmRef};
    real_t drv_current;

    // maxFollower max_follower;
    CurrentGen cg;

    // timer1.bindCb(TimerUtils::IT::Update, [&](){
    //     adc1.swStartInjected();
    //     while(!adc1.isIdle());
    //     drv_current = driver.getCurrent();
    // });

    // timer1.enableIt(TimerUtils::IT::Update, {0,0});


    driver.init();
        // pwmRef = real_t(1.3)/3.3;
    driver.setCurrentClamp(0.7);
    driver.enable();

    while(true){
        // real_t targ_current = 0.20 + 0.04 * sin(7 * t);
        real_t targ_current = 0.16;
        driver.setCurrent(targ_current);
        drv_current = driver.getCurrent();
        cg.update(drv_current);
        if(!logger.pending()) logger.println(targ_current, drv_current, real_t(cg), TIM1->CH1CVR, TIM1->CH2CVR, bool(portA[9]));
    }
}