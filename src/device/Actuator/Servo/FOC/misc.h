
// struct MotorPosition{
//     real_t lapPositionHome = real_t(0);
//     real_t lapPosition = real_t(0);
//     real_t lapPositionLast = real_t(0);

//     int16_t accTurns = 0;
//     real_t accPosition = real_t(0);
//     // real_t accPositionFixed = real_t(0);
//     real_t accPositionLast = real_t(0);

//     real_t vel = real_t(0);
//     real_t accPositionAgo = real_t(0);
//     real_t elecRad = real_t(0);
//     // real_t elecRadFixed = real_t(0);
// }motorPosition;
// real_t readLapPosition(){
//     real_t ret;
//     u16_to_uni((uint16_t)(TIM2->CNT), ret);
//     return ret;
// }
// void updatePosition(){
//     motorPosition.lapPosition = readLapPosition();
//     real_t deltaLapPosition = motorPosition.lapPosition - motorPosition.lapPositionLast;

//     if(deltaLapPosition > real_t(0.5f)){
//         motorPosition.accTurns -= 1;
//     }else if (deltaLapPosition < real_t(-0.5f)){
//         motorPosition.accTurns += 1;
//     }

//     motorPosition.lapPositionLast = motorPosition.lapPosition;
//     motorPosition.accPositionLast = motorPosition.accPosition;
//     motorPosition.accPosition = real_t(motorPosition.accTurns) + (motorPosition.lapPosition - motorPosition.lapPositionHome);
// }
// void setMotorDuty(const real_t & duty){
//     if(duty >= real_t(0)){
//         uint16_t value = (int)(duty * pwm_arr);
//         TIM4->CH1CVR = value;
//         TIM4->CH2CVR = 0;
//         TIM4->CH3CVR = value / 2;
//     }else{
//         uint16_t value = (int)((-duty) * pwm_arr);
//         TIM4->CH1CVR = 0;
//         TIM4->CH2CVR = value;
//         TIM4->CH3CVR = value / 2;
//     }
// }

// void setABCoilDuty(const real_t & aduty, const real_t & bduty){
//     if(aduty >= real_t(0)){
//         uint16_t value = (int)(aduty * pwm_arr);
//         TIM4->CH1CVR = value;
//         TIM4->CH2CVR = 0;
//     }else{
//         uint16_t value = (int)((-aduty) * pwm_arr);
//         TIM4->CH1CVR = 0;
//         TIM4->CH2CVR = value;
//     }

//     if(bduty >= real_t(0)){
//         uint16_t value = (int)(bduty * pwm_arr);
//         TIM4->CH3CVR = value;
//         TIM4->CH4CVR = 0;
//     }else{
//         uint16_t value = (int)((-bduty) * pwm_arr);
//         TIM4->CH3CVR = 0;
//         TIM4->CH4CVR = value;
//     }
// }


// void setIrState(const bool on){
//     TIM3->CH1CVR = on ? (ir_arr / 3) : 0;
// }
