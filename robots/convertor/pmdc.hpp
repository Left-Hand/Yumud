#ifndef __PMDC_HPP__

#define __PMDC_HPP__

static auto pos_pid = PID_t<real_t>(3.1, 0.5, 1.02, 1.0);
static auto curr_pid = PID_t<real_t>(20.0, 0.0, 0.0, 1.0);
static auto pos2curr_pid = PID_t<real_t>(11.0, 0.05, 1.1, 1.0);
static real_t omega = real_t(0.3);


void pmdc_test(){

    uart2.init(115200 * 8, Uart::Mode::TxRx);
    IOStream & logger = uart2;
    logger.setSpace(",");
    logger.setEps(4);

    timer3.init(36000);
    timer3[1].setPolarity(true);
    timer3[2].setPolarity(true);
    // timer3[1].setPolarity(false);
    // timer3[2].setPolarity(false);
    auto pwmL = PwmChannel(timer3[2]);
    auto pwmR = PwmChannel(timer3[1]);
    pwmL.init();
    pwmR.init();

    Coil2 motor = Coil2(pwmL, pwmR);
    motor.init();
    // tim1ch1n
    // Exti
    // auto trigGpioA = portA[1];
    // auto trigGpioB = portA[4];
    ABEncoderExti enc(portA[4], portA[1], NvicPriority(0, 7));
    // enc.init();

    // auto odo = Odometer(enc);
    OdometerLines odo(enc, 1100);
    Estimmator est(odo);
    // constexpr auto a = ((uint32_t)(16384  << 16) / 1000) >> 16;

    // AdcChannelConfig{.}
    adc1.init(
        {
            AdcChannelConfig{.channel = AdcChannels::TEMP, .sample_cycles = AdcSampleCycles::T55_5}
        },
        {
            AdcChannelConfig{.channel = AdcChannels::CH0, .sample_cycles = AdcSampleCycles::T55_5}
        });
    // adc1.setRegularTrigger(AdcOnChip::RegularTrigger::SW);
    // adc1.setInjectedTrigger(AdcOnChip::InjectedTrigger::SW);
    // timer3[4] = 0;
    adc1.setInjectedTrigger(AdcOnChip::InjectedTrigger::T3CC4);
    TIM3->CH4CVR = TIM3->ATRLR >> 1;
    adc1.enableCont();
    // adc1.enableScan();
    adc1.enableAutoInject();

    // adc1.enableRightAlign(false);
    real_t motor_curr;
    // adc1.start();
    // adc1.swStartRegular();
    // adc1.swStartInjected();

    est.init();


    motor.enable();
    real_t duty;
    // real_t duty(0);
    real_t target_curr;
    real_t motor_curr_temp;
    real_t target_pos;
    LowpassFilter_t<real_t, real_t> lpf(10);
    String temp_str;

    constexpr int closeloop_freq = 1000;
    Gpio & t_watch = portA[5];
    t_watch.outpp();
    timer4.init(closeloop_freq);
            real_t sense_uni;
    timer4.bindCb(Timer::IT::Update, [&](){
        t_watch.set();
        est.update(t);
        // duty = CLAMP(duty +), -1, 1);
        // motor.setDuty( pos_pid.update(target, odo.getPosition(), est.getSpeed()));
        target_curr = pos2curr_pid.update(target_pos, odo.getPosition(), est.getSpeed());

        u16_to_uni(ADC1->IDATAR1 << 4, sense_uni);
        constexpr float sense_scale = (1000.0 / 680.0) * 3.3;
        motor_curr_temp = sign(duty) * sense_uni * sense_scale;
        // motor_curr = lpf.update(motor_curr_temp, t);4
        motor_curr = lpf.forward(motor_curr_temp, real_t(1.0 / closeloop_freq));

        duty = curr_pid.update(target_curr, motor_curr);
        motor = duty;
        t_watch.clr();
    });

    // timer4.enableIt(Timer::IT::Update, NvicPriority(0, 0));
    // uint16_t adc_out;
    pos2curr_pid.setClamp(0.2);

    while(true){
        // est.update();
        // if(adc1.isInjectedIdle()) adc1.swStartInjected();
            // adc_out = ADC1->RDATAR;

        // }
        // target = 10 * sin(t / 4);
        // target = 4 * floor(t/3);
        // target_pos = real_t(0.12) * t;
        static real_t ang = real_t(0);
        static real_t last_t = real_t(0);

        // target_pos = 2 * sign(frac(ang += omega * (t - last_t)) - 0.5);
        target_pos = sin(ang +=  omega * (t - last_t));
        last_t = t;
        // motor = 0.7 * sin(t);
        // motor = duty;
        // motor = frac(t);
        // target = real_t(0.1);

        // uart2.println(target, est.getPosition(), est.getSpeed(), motor_curr, duty, lpf.update(motor_curr, t));
        // static auto prog = real_t(0); prog += real_t(0.01);
        // uart2.println(motor_curr, );
        // logger.println(odo.getPosition(),est.getSpeed(), target_pos, motor_curr, target_curr);
        // logger.println(duty, motor_curr_temp, motor_curr, odo.getPosition(), est.getSpeed(), 0);
        // logger.println(target_pos, odo.getPosition(), est.getSpeed(), motor_curr );
        // motor.setDuty(sin(t));
        // logger.println(ADC1->IDATAR1);
        // delay(2);

        logger.println(ADC1->IDATAR1);
        if(logger.available()){
            char chr = logger.read();
            if(chr == '\n'){
                temp_str.trim();
                // logger.println(temp_str);
                // if(temp_str.length()) parseLine(temp_str);
                temp_str = "";
            }else{
                temp_str.concat(chr);
            }
        }
        Sys::Clock::reCalculateTime();

    }
}



#endif