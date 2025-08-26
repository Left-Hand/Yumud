#pragma once
// class Test:pub
// void buck_test(){
//     uart2.init(115200 * 8, Uart::Mode::TxRx);
//     IOStream & logger = uart2;
//     logger.setSpace(",");
//     logger.set_eps(4);


//     timer1.init(32000, Timer::TimerCountMode::Up);


//     timer1.initBdtr(16);


//     auto & ch = timer1.ch(1);
//     auto & chn = timer1.chn(1);

//     ch.setIdleState(true);
//     // ch.init();
//     chn.setIdleState(false);
//     chn.init();

//     auto buck_pwm = PwmIntf(ch);
//     buck_pwm.setClamp(real_t(0.1), real_t(0.9));
//     buck_pwm.init();

//     adc1.init(
//         {
//             AdcChannelConfig{.channel = AdcChannels::CH0, .cycles = AdcSampleCycles::T239_5}
//         },
//         {
//             AdcChannelConfig{.channel = AdcChannels::CH0, .cycles = AdcSampleCycles::T239_5}
//             // AdcChannelConfig{.channel = AdcChannels::CH1, .cycles = AdcSampleCycles::T239_5},
//         });

//     static constexpr int buck_freq = 1000;
//     timer3.init(buck_freq);

//     real_t adc_fl1, adc_fl2, adc_out, duty;
//     buckRuntimeValues buck_rv;
//     LowpassFilter<real_t, real_t> lpf(50);
//     LowpassFilter<real_t, real_t> lpf2(30);

//     // TIM3->CH4CVR = TIM3->ATRLR >> 1;
//     // timer3.bindCb(Timer::IT::CC4, [&](){

//     //     duty = real_t(0.3) + 0.14 * sin(4 * TAU * t);
//     //     // duty = real_t(0.5);
//     //     u16_to_uni(ADC1->IDATAR1<<4, adc_out);
//     //     // adc_fl1 = lpf.forward(adc_out, real_t(1.0 / buck_freq));
//     //     // adc_fl2 = lpf2.forward(adc_fl1, real_t(1.0 / buck_freq));

//     //     // buck_rv.curr.measure = real_t(adc_fl2);
//     //     buck_pwm = real_t(0.1);
//     // });
//     // timer3.enableIt(Timer::IT::CC4, NvicPriority(0, 0));

//     adc1.setInjectedTrigger(AdcOnChip::InjectedTrigger::T3CC4);
//     // adc1.setPga(AdcOnChip::Pga::X1);
//     adc1.enableCont();
//     adc1.enableAutoInject(); // must be enabled for ext.inj

//     opa2.init(1);

//     timer1.enable();
//     Pll pll;


//     while(true){

//         // if(adc1.isRegularIdle()){
//         //     // adc1.refreshInjectedData();
//         //     // adc1.swStartInjected();
//         //     adc1.swStartRegular();
//         // }

//         // if(adc1.isInjectedIdle()){
//         //     // adc1.refreshInjectedData();
//         //     // adc1.swStartInjected();
//         //     adc1.swStartInjected();
//         // }
//         // u16_to_uni(ADC1->RDATAR << 4, adc_out);


//         static real_t last_t = t;
//         static real_t angle = real_t(0);
//         const real_t omega = 50 + 20 * sin(t);
//         real_t delta_t = t - last_t;
//         last_t = t;

//         angle += delta_t * omega;
//         real_t pll_input = sin(angle);
//         real_t pll_output =  pll.update(pll_input,real_t(0.04));
//         logger.println(pll_input, cos(pll_output), pll.omega, ADC1->IDATAR1);
//         Sys::Clock::reCalculateTime();
//     }
// }
