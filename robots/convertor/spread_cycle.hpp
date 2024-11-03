#pragma once

// namespace SpreadCycle{

// constexpr float coil_inductor_mH = 0.83;
// constexpr float coil_resistor_Ohm = 2.24;
// constexpr float busbar_voltage = 9.0;

// constexpr float c1 = busbar_voltage / coil_inductor_mH;
// constexpr float coil_current = 0.23;
// constexpr float c2 = - coil_current * coil_resistor_Ohm / coil_inductor_mH;

// PwmChannel coil_pwm_p(timer1[3]);
// PwmChannel coil_pwm_n(timer1[4]);

// real_t dual_duty[2] = {
//     real_t(0.3),
//     real_t(0.14)
// };

// uint16_t dual_cvr[2] = {699,2299};

// real_t chopper_current = real_t(0.2);

// uint32_t chopper_run_cnt = 0;
// struct {
//     bool odd_is_forward:1;
//     bool even_is_forward:1;
// }chopper_run_mode;

// void chopper_run(){

//     if(chopper_run_cnt == 0){
//         chopper_run_mode.odd_is_forward = true;
//         chopper_run_mode.even_is_forward = false;
//         chopper_run_cnt = 1;
//         return;
//     }

//     chopper_run_cnt++;
//     bool cycle_is_odd = chopper_run_cnt % 2;

//     bool cycle_is_forward = cycle_is_odd ? chopper_run_mode.odd_is_forward : chopper_run_mode.even_is_forward;
//     // bool cycle_is_forward = cycle_is_odd;

//     // int pwm_duty_p = cycle_is_forward ? 0 : dual_cvr[cycle_is_odd];
//     // int pwm_duty_n = cycle_is_forward ? dual_cvr[cycle_is_odd] : 0;
//     int pwm_duty = dual_cvr[cycle_is_odd];
//     // int backward_duty = dual_cvr[1];
//     if(cycle_is_forward){
//         timer1[3] = pwm_duty;
//         timer1[4] = 0;
//     }else{
//         timer1[3] = 0;
//         timer1[4] = pwm_duty;
//     }

// }


// void chopper_test(){
//     uart1.init(115200 * 8, Uart::Mode::TxRx);
//     IOStream & logger = uart1;
//     logger.setSpace(",");
//     logger.setEps(4);

//     timer1.init(36000);
//     timer1.enableArrSync();
//     // timer1.enableCvrSync();
//     timer1[3].enableSync();
//     timer1[4].enableSync();
//     timer1[3].setPolarity(false);
//     timer1[4].setPolarity(false);

//     timer1[3].init();
//     timer1[4].init();

//     coil_pwm_p.setClamp(real_t(0.4));
//     coil_pwm_n.setClamp(real_t(0.4));

//     timer3.init(36000);
//     timer3[2].init();
//     timer3[3].init();
//     timer3[2].setPolarity(true);
//     timer3[3].setPolarity(true);

//     timer3[2] = real_t(0.4);
//     timer3[3] = real_t(0.4);



//     timer1.bindCb(Timer::IT::Update, std::function<void(void)>(chopper_run));
//     timer1.enableIt(Timer::IT::Update, NvicPriority(0, 0));

//     while(true){
//         logger.println(int(timer1[3]), int(timer1[4]));
//     }
// }



// };
