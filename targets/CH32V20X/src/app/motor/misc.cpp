#if 0
    // struct Measurer{
    //     real_t phase_res;
    //     real_t phase_ind;
    //     size_t measure_times;
    // };

    // Measurer meas;

    // size_t measure_times = 0;
    real_t phase_diff = 0;

    real_t phase_res = 0.65_r;
    real_t phase_ind;

    [[maybe_unused]] auto cb_measure_res = [&]{

        targ_pos = 0;

        curr_sens.update(0);

        // scexpr int fs = CHOPPER_FREQ / 2;
        // scexpr int test_freq = 200;
        // scexpr int test_freq = 500;
        scexpr real_t test_volt = 0.9_r;
        // scexpr int test_freq = 1000;
        svpwm.set_ab_volt(test_volt, 0);
        // phase_res = test_volt / curr_sens.ab().a;
        // phase_res = test_volt  * 0.66_r / curr_sens.uvw().u;
        phase_res = test_volt / curr_sens.ab().a;
    };

    [[maybe_unused]] auto cb_measure_ind = [&]{

        targ_pos = 0;

        curr_sens.update(0);

        scexpr int fs = CHOPPER_FREQ / 2;
        // scexpr int test_freq = 200;
        scexpr int test_freq = 500;
        scexpr real_t test_volt = 0.6_r;
        // scexpr int test_freq = 1000;
        static int cnt = 0; 
        scexpr int div = fs / test_freq;

        static bool upedge_captured = true;

        cnt++;
        if(cnt >= div){
            cnt = 0;
            upedge_captured = false;
        }
        scexpr real_t omega = real_t((TAU * test_freq) / fs);

        {
            static real_t last_curr = 0;
            real_t this_curr = curr_sens.ab().a;
            // spll.update(this_curr);

            if(upedge_captured == false and last_curr < 0 and this_curr > 0){                
                auto phase_diff_pu = (real_t(cnt) / div);
                if(
                    phase_diff_pu < real_t(0.25) // RL网络最大滞后90度 即1/4圆
                    // and phase_diff_pu > real_t(0.03) // 滤除噪声
                ){
                    phase_diff = LPFN<4>(phase_diff, phase_diff_pu * real_t(TAU));
                    // phase_diff = phase_diff_pu * real_t(TAU);
                }
                // phase_ind = tan(phase_diff) * phase_res / (real_t(TAU) * test_freq);
                phase_ind = tan(phase_diff) * phase_res * real_t(1/TAU)/ test_freq;


                // phase_ind = spll.theta();
                // phase_ind = real_t(cnt) / div;
                // phase_ind = phase_diff;
                // phase_ind = (real_t(cnt) / div) * real_t(TAU);
                // upedge_captured = true;
            }
            last_curr = this_curr;
        }

        const auto __res = omega * cnt;
        ab_volt = {test_volt * sin(__res),0};
        svpwm.set_ab_volt(ab_volt[0], ab_volt[1]);
    };

    [[maybe_unused]] auto cb_sensorless = [&]{

        // targ_pos = real_t(6.0) * sin(2 * t);
        targ_pos = real_t(1.0) * clock::time();

        const auto & ab_curr = curr_sens.ab();
        // const auto dq_curr = curr_sens.dq();

        // smo_ob.update(ab_volt[0], ab_volt[1], ab_curr[0], ab_curr[1]);
        lbg_ob.update(ab_volt[0], ab_volt[1], ab_curr[0], ab_curr[1]);
        // nlr_ob.update(ab_volt[0], ab_volt[1], ab_curr[0], ab_curr[1]);
        // pll.update(lbg_ob.theta());
        // sl_meas_rad = pll.theta() + 0.3_r;
        sl_meas_rad = lbg_ob.theta();
        // curr_sens.update(pll.theta());
        curr_sens.update(sl_meas_rad);
        // const auto rad = sl_meas_rad + 3.0_r;

        // const auto d_volt = d_pi_ctrl.update(0.01_r, dq_curr.d);
        // const auto d_volt = d_pi_ctrl.update(0.01_r, dq_curr.d);
        const auto d_volt = 0;
        // const auto d_volt = d_pi_ctrl.update(0.3_r, dq_curr.d);
        // const auto q_volt = q_pi_ctrl.update(0.3_r + 0.24_r * sin(3 * t), dq_curr.q);
        // const auto q_volt = q_pi_ctrl.update(0.3_r + 0.24_r * sin(3 * t), dq_curr.q);
        // const auto q_volt = q_pi_ctrl.update(-0.3_r + 0.2_r * frac(3 * t), dq_curr.q);
        const auto q_volt = 4;
        // const auto q_volt = q_pi_ctrl.update(-0.3_r, dq_curr.q);

        // ab_volt = {4 * cos(rad), 4 * sin(rad)};
        // ab_volt = dq_to_ab(DqVoltage{d_volt, q_volt}, sl_meas_rad + 1.0_r * sin(t));
        // ab_volt = dq_to_ab(DqVoltage{d_volt, q_volt}, sl_meas_rad + 1.5_r);
        ab_volt = DqVoltage{d_volt, q_volt}.to_ab(sl_meas_rad + 2.0_r);
        svpwm.set_ab_volt(ab_volt[0], ab_volt[1]);
    };




    [[maybe_unused]] auto cb_pulse = [&]{
        static int cnt = 0;

        scexpr real_t pulse_volt = 6;
        scexpr int sustain = (0.0003) * 25000;
        scexpr int dur = (0.02) * 25000;

        cnt ++;
        if(cnt >= sustain + dur){
            cnt = 0;
            sector_cnt = (sector_cnt + 1) % 6;
        }

        const int sector = []() -> int{
            switch(sector_cnt){
                default:
                case 0: return 0;
                case 1: return 3;
                case 2: return 1;
                case 3: return 4;
                case 4: return 2;
                case 5: return 5;
            }
        }();

        real_t pulse_rad = real_t(PI/3) * real_t(sector);

        real_t pulse_s = sin(pulse_rad);
        real_t pulse_c = cos(pulse_rad);

        real_t pulse_out = (cnt < sustain) ? pulse_volt : real_t(0);
        // real_t pulse_out = 0;


        ab_volt = {pulse_out * pulse_c, pulse_out * pulse_s};
        svpwm.set_ab_volt(ab_volt[0], ab_volt[1]);

        curr_sens.update(0);
    };

    [[maybe_unused]] auto cb_sing = [&]{
        
        // static q20 mt = 0;
        // mt += q20(1.0 / FOC_FREQ);

        // const auto [s,c] = sincos(mt);

        // real_t sing_out = 4 * sin(2400 * frac(sing_t) * iq_t<16>(TAU) + 3 * sin(40 * frac(sing_t) * iq_t<16>(TAU)));

        // ab_volt = {sing_out * sing_c, sing_out * sing_s};
        // svpwm.set_ab_volt(ab_volt[0], ab_volt[1]);

        // curr_sens.update(0);
    };

    [[maybe_unused]] auto cb_openloop = [&]{
        static q20 mt = 0;
        mt += q20(1.0 / FOC_FREQ);

        scexpr auto omega = ({
            // real_t(6 * TAU);
            real_t(1);
        });
        // const auto max_amp = real_t(6.7) + 2 * sin(7*mt);
        const auto max_amp = real_t(2.5);
        auto & ob = lbg_ob;
        // auto & ob = nlr_ob;
        // const auto max_amp = real_t(8.7);

        const auto begin_m = clock::micros();
        // const auto max_amp = real_t(2.8) + sin(t);
        // auto theta = omega * t + real_t(12) * sin(2 * real_t(TAU) * t);
        // auto theta = omega * clock::time();
        // const auto theta = 0;
        // const auto t = clock::time();


        mg_meas_rad = mt * omega;
        const auto rad = ({
            mg_meas_rad;
            // (mt < 0.2_r) ? q16() : 
            // q20(lbg_ob.theta()) + q20(PI/2);
            // q16(ob.theta()) + q16(CLAMP(mt * 0.4_r, 0, 0.07_r));
            // q16(mg_meas_rad);
            // sl_meas_rad + ;
            // const auto [s,c] = sincos(mt);
        });
        const auto [s,c] = sincos(rad);
        // const auto [s,c] = sincos(mt *8);
        const auto amp = CLAMP(2 + mt * 3, 0, max_amp);
        ab_volt = {amp * c, amp * s};
        // ab_volt = {amp, amp};
        svpwm.set_ab_volt(ab_volt[0], ab_volt[1]);
        curr_sens.update(rad);
        // odo.update();
        // const real_t meas_lap = odo.getLapPosition();

        // const real_t meas_rad = (frac(frac(meas_lap - 0.25_r) * 7) * real_t(TAU));
        // mg_meas_rad = meas_rad;

        const auto & ab_curr = curr_sens.ab();
        ob.update(ab_volt[0], ab_volt[1], ab_curr[0], ab_curr[1]);
        // nlr_ob.update(ab_volt[0], ab_volt[1], ab_curr[0], ab_curr[1]);
        // nlr_ob.update(ab_volt[0], ab_volt[1], ab_curr[0], ab_curr[1]);
        // pll.update(ob.theta());
        // sl_meas_rad = pll.theta() + 0.3_r;
        sl_meas_rad = ob.theta();

        // curr_sens.capture();

        // exe_micros  = uint32_t(clock::micros()) - begin_m;
        exe_micros = clock::micros() - begin_m;
    };


    




    [[maybe_unused]] bool can_en = false;

    DEBUGGER.set_splitter(',');
    DEBUGGER.no_brackets();

    
    // OutputStream::Config

    CurrentBiasCalibrater calibrater = {{
        .period_ticks = 10000,
        .fc = 20,
        .fs = FOC_FREQ
    }};

    // constexpr auto alpha = LowpassFilterD_t<double>::solve_alpha(5.0, FOC_FREQ);
    // LowpassFilterD_t<iq_t<16>> speed_measurer = {
    // LowpassFilterD_t<iq_t<16>> speed_measurer = {

    [[maybe_unused]] auto measure_bias = [&]{
        
        // mp6540.enable(false);
        // curr_sens.capture();
        calibrater.update(curr_sens.raw(), curr_sens.mid());

        // odo.update();
        // speed_measurer.update(odo.getPosition());

        // mp6540.enable(true);
        if(calibrater.is_done()) {
            // const auto guard = DEBUGGER.createGuard();
            // DEBUGGER.force_sync();
            // DEBUG_PRINTLN("Done Current Bias Measure!!");
            // DEBUG_PRINTLN(calibrater.result());
            // clock::delay(100ms);
            // ASSERT(false);
            // clock::delay(1000ms); sys::reset();
        }
    };

    [[maybe_unused]] auto cb_hfi = [&]{

        static int cnt = 0;
        // scexpr int hfi_freq = 4096;
        // scexpr int hfi_freq = 2500;
        // scexpr int hfi_freq = 1024;
        // scexpr int hfi_freq = 512;
        // scexpr int hfi_freq = 256;
        // scexpr int divider = CHOPPER_FREQ / 2 / hfi_freq;
        scexpr int divider = 16;
        scexpr size_t hfi_freq = FOC_FREQ / divider;
        scexpr real_t hfi_base_volt = 1.2_r;
        scexpr real_t openloop_base_volt = 0.0_r;
        cnt = (cnt + 1) % divider;


        real_t hfi_rad = real_t(TAU) * real_t(cnt) / divider;
        real_t hfi_c = cos(hfi_rad);
        real_t hfi_out = hfi_base_volt * hfi_c;

        real_t openloop_rad = -frac(2.1_r * clock::time())*real_t(TAU);
        const auto [openloop_s, openloop_c] = sincos(openloop_rad);
        // real_t s = sin(hfi_rad);

        ab_volt = {hfi_out + openloop_base_volt * openloop_c, openloop_base_volt * openloop_s};
        svpwm.set_ab_volt(ab_volt[0], ab_volt[1]);

        // curr_sens.updatUVW();
        // curr_sens.updateAB();
        // curr_sens.updateDQ(0);
        curr_sens.update(0);
        // real_t mul = curr_sens.ab()[1] * s;
        [[maybe_unused]] const real_t mul = curr_sens.ab()[1] * hfi_c;
        // real_t last_hfi_result = hfi_result;
        // hfi_result = LPF(last_hfi_result, mul);
        // static dsp::ButterBandpassFilter<q16, 4> hfi_filter{{.fl = 1, .fh = 40, .fs = FOC_FREQ}};
        // static dsp::LowpassFilter<q20> hfi_filter_lpf{{.fc = 2, .fs = FOC_FREQ}};
        // static dsp::LowpassFilter<q20> hfi_filter_lpf{{.fc = hfi_freq, .fs = FOC_FREQ}};
        static dsp::LowpassFilter<q20> hfi_filter_lpf{{.fc = hfi_freq * 0.1_r, .fs = FOC_FREQ}};
        static dsp::LowpassFilter<q20> hfi_filter_mid_lpf{{.fc = 1, .fs = FOC_FREQ}};
        // static dsp::Highpa<q24> hfi_filter_lpf{{.fc = 2, .fs = FOC_FREQ}};
        // static dsp::ButterLowpassFilter<q20,  2> hfi_filter_lpf{{.fc = 20, .fs = FOC_FREQ}};
        // static dsp::ButterHighpassFilter<q20, 2> hfi_filter_hpf{{.fc = 600, .fs = FOC_FREQ}};
        hfi_filter_lpf.update(curr_sens.ab()[1]);
        hfi_filter_mid_lpf.update(hfi_filter_lpf.get());
        // hfi_filter_hpf.update(hfi_filter_lpf.get());
        // hfi_filter_hpf.update(mul);
        [[maybe_unused]] const auto temp = hfi_filter_lpf.get();
        // hfi_result = temp;
        hfi_result = hfi_filter_lpf.get();
        hfi_mid_result = hfi_filter_mid_lpf.get();
        // hfi_result = curr_sens.ab()[1] * hfi_c;
        // hfi_result = hfi_filter_hpf.get();
        // hfi_result = LPFN<9>(hfi_result, mul);

        // hfi_result = mul;
        
        // dt = clock::micros() - m;
    };

    adc1.attach(AdcIT::JEOC, {0,0}, 
        cb_sensored
    );

            
        // auto s_curr = [&](){
            //     return uvw_curr.u * uvw_curr.u + uvw_curr.v * uvw_curr.v + uvw_curr.w * uvw_curr.w;
            // }();
            // DEBUG_PRINTLN_IDLE(uvw_curr.u, uvw_curr.v, uvw_curr.w,sector_cnt, s_curr);
            // DEBUG_PRINTLN_IDLE(ab_curr[1], ab_volt[0], hfi_result, acos(hfi_result * real_t(1 / 0.045 )));
            
            // DEBUG_PRINTLN_IDLE(hfi_result);
            // DEBUG_PRINTLN_IDLE(ab_volt[0]);
            // DEBUG_PRINTLN_IDLE(ab_curr[0], ab_curr[1], ab_volt[0]);
            // DEBUG_PRINTLN_IDLE(pos, uvw_curr[0], uvw_curr[1], uvw_curr[2], dq_curr[0], dq_curr[1], targ_pos, pos, smo_ob.getTheta(), dt > 100 ? 1000 + dt : dt);
            // clock::delay(2ms);
            
            // DEBUG_PRINTLN_IDLE(rad, sin(rad), cos(rad), atan2(sin(rad), cos(rad)));
            // DEBUG_PRINTLN_IDLE(pos, uvw_curr[0], uvw_curr[1], uvw_curr[2], dt > 100 ? 1000 + dt : dt);
            // Sys::Clock::reCalculateTime();
            
        // CanMsg msg = {0x11, std::make_tuple<uint8_t, uint8_t>(0x55, 0xAA)};
        // if(can1.pending() == 0 and can_en) can1.write(msg);

        
        // DEBUG_PRINTLN_IDLE(uvw_curr[0], uvw_curr[1], uvw_curr[2], dq_curr, odo.getPosition()); 
        // DEBUG_PRINTLN_IDLE(1); 
            // DEBUG_PRINTLN_IDLE(ab_curr, dq_curr, can1.available(), can1.getTxErrCnt(), std::setbase(2), 11);
            // DEBUG_PRINTLN_IDLE(ab_curr, dq_curr, can1.available(), can1.getTxErrCnt(), std::setbase(2), CAN1->ERRSR);
            // , real_t(pwm_v), real_t(pwm_w), std::dec, data[0]>>12, data[1] >>12, data[2]>>12);
        // DEBUG_PRINTLN_IDLE(odo.getPosition(), odo.getSpeed(), pll.pos_est_, pll.spd_est_, dq_curr.d, dq_curr.q);
        // clock::delay(2ms);
        // DEBUGGER.no_brackets(true);
        // DEBUG_PRINTLN_IDLE(odo.getPosition(), Vec2<real_t>(1,1));
        // clock::delay(2ms);

        // DEBUGGER.force_sync();
        // if(false){
        //     const auto guard = DEBUGGER.createGuard();
        //     DEBUGGER.no_brackets(false);
        //     DEBUG_PRINTLN(odo.getPosition(), Vec2<real_t>(1,1));
        //     // DEBUGGER.flush();
        // }

        // if(true){
        //     const auto guard = DEBUGGER.createGuard();
        //     // DEBUGGER.set_splitter('|');
        //     DEBUGGER.no_brackets(true);
        //     DEBUG_PRINTLN(odo.getPosition(), Vec2<real_t>(1,1));
        //     // DEBUGGER.flush();
        // }
        // DEBUG_PRINTLN(std::setprecision(3), std::dec, adc_data_cache[0], adc_data_cache[1], adc_data_cache[2], (ADC1->IDATAR1 + ADC1->IDATAR2 + ADC1->IDATAR3)/3);
        // (ADC1->IDATAR1 + ADC1->IDATAR2 + ADC1->IDATAR3)/3

        // DEBUG_PRINTLN_IDLE(std::setprecision(3), std::dec, uvw_curr[0], uvw_curr[1], uvw_curr[2], ADC1->IDATAR1, ADC1->IDATAR2, ADC1->IDATAR3); 
        // DEBUG_PRINTLN(std::setprecision(3), std::dec, ADC1->IDATAR1, ADC1->IDATAR2, ADC1->IDATAR3, (ADC1->IDATAR1 + ADC1->IDATAR2 + ADC1->IDATAR3)/3); 
        // DEBUG_PRINTLN(std::setprecision(3), std::dec, uvw_curr[0], uvw_curr[1], uvw_curr[2], ab_curr[0], ab_curr[1], est_rad, rad);
        // DEBUG_PRINTLN(std::setprecision(2), std::dec, int(uvw_curr[0]*100), int(uvw_curr[1]*100), int(uvw_curr[2]*100));
        // DEBUG_PRINTLN(std::setprecision(2), std::dec, uvw_curr[0], uvw_curr[1], uvw_curr[2]);
        // DEBUG_PRINTLN(std::setprecision(2), std::dec, int(uvw_curr[0]*100));
        // DEBUG_PRINTLN(std::setprecision(3), std::dec, ADC1->IDATAR1, ADC1->IDATAR2, ADC1->IDATAR3);
        // DEBUG_PRINTLN(std::setprecision(3), std::dec, uvw_curr[0], uvw_curr[1], uvw_curr[2], ab_curr[0], ab_curr[1]);
        // DEBUG_PRINTLN(std::setprecision(3), std::dec, real_t(adc1.inj(1)), real_t(adc1.inj(2)), real_t(adc1.inj(3)));
        // DEBUG_PRINTLN(std::setprecision(3), std::dec, real_t(u_sense), real_t(v_sense), real_t(w_sense));
        // pwm_u = sin(t) * 0.5_r + 0.5_r;
        // DEBUG_PRINTLN(std::setprecision(3), std::dec, real_t(pwm_u), real_t(pwm_v), real_t(pwm_w));
        // clock::delay(5ms);
        // DEBUG_PRINTLN(std::setprecision(3), std::dec, real_t(adc1.inj(1)), uint16_t(adc1.inj(1)));
        // DEBUG_PRINTLN(std::setprecision(3), std::dec, real_t(u_sense), s_lpf_u_curr);
        // auto [a,b] = Vec2<real_t>{real_t(0), real_t(0.2)}.rotated(open_rad);
        // DEBUG_PRINTLN(a,b);
        // DEBUG_PRINTLN(std::setprecision(3), std::dec, TIM1->CH1CVR, TIM1->CH4CVR, ADC1->IDATAR1);
        // TIM1->CH4CVR = 1000;
        // cb();
        // clock::delay(10ms);
        // DEBUG_PRINTLN(spi1.cs_port.isIndexValid(0), spi1.cs_port.isIndexValid(1), spi1.cs_port.isIndexValid(2))
        // DEBUG_PRINTLN("0");
        // bmi.check();
        // clock::delay(20ms);


        // static PIController pos_pi_ctrl{
    // {
    //     .kp = 27.0_r,
    //     .ki = 0.00_r,
    //     .out_min = -50.6_r,
    //     .out_max = 50.6_r
    // }};

    static PIController speed_pi_ctrl{
    {
        .kp = 2.3_r,
        // .kp = 0,
        .ki = 0.0277_r,
        // .ki = 0.0001_r,
        // .out_min = -0.3_r,
        // .out_max = 0.3_r

        // .kp = 2.3_r,
        // .ki = 0.009_r,
        .out_min = -0.5_r,
        .out_max = 0.5_r
    }};




    static PIController d_pi_ctrl{
    {
        .kp = 0.0_r,
        .ki = 0.011_r,
        .out_min = -6.0_r,
        .out_max = 6.0_r
    }};

    static PIController q_pi_ctrl{
    {
        .kp = 0.0_r,
        .ki = 0.011_r,
        .out_min = -6.0_r,
        .out_max = 6.0_r
    }};

class BldcMotor{
// public:
//     SVPWM3 & svpwm_;
//     CurrentSensor & curr_sensor_;

//     BldcMotor(SVPWM3 & svpwm, CurrentSensor & curr_sensor):
//         svpwm_(svpwm),
//         odo_(odo),
//         curr_sensor_(curr_sensor){;}

//     using Torque = real_t;
//     using Speed = real_t;
//     using Position = real_t;

//     struct CurrentCtrl{
//         DqVoltage update(const DqCurrent targ_curr, const DqCurrent meas_curr){
//             return {
//                 d_pi_ctrl.update(targ_curr.d, meas_curr.d),
//                 q_pi_ctrl.update(targ_curr.q, meas_curr.q)
//             };
//         }

//         PIController d_pi_ctrl = {
//         {
//             .kp = 0.0_r,
//             .ki = 0.011_r,
//             .out_min = -6.0_r,
//             .out_max = 6.0_r
//         }};
        
//         PIController q_pi_ctrl = {
//         {
//             .kp = 0.0_r,
//             .ki = 0.011_r,
//             .out_min = -6.0_r,
//             .out_max = 6.0_r
//         }};
//     };

//     struct TorqueCtrl{
//         DqCurrent update(const Torque targ_torque){
//             return {0, targ_torque};
//         }
//     };

//     struct TraditionalSpeedCtrl{
//         Torque update(const Speed targ_spd, const Speed meas_spd){
//             return {
//                 speed_pi_ctrl.update(targ_spd, meas_spd)
//             };
//         }


//         PIController speed_pi_ctrl = {{
//             .kp = 2.3_r,
//             // .kp = 0,
//             .ki = 0.0277_r,
//             // .ki = 0.0001_r,
//             // .out_min = -0.3_r,
//             // .out_max = 0.3_r
    
//             // .kp = 2.3_r,
//             // .ki = 0.009_r,
//             .out_min = -0.5_r,
//             .out_max = 0.5_r
//         }};
//     };

//     struct TraditionalPositionCtrl{
//         Speed update(const Position targ_pos, const Position meas_pos, const Speed meas_spd){
//             const auto targ_spd = 0;
//             return {
//                 35.8_r * (targ_pos - meas_pos) + 0.7_r*(targ_spd - meas_spd)
//             };
//         }


//         PIController speed_pi_ctrl = {{
//             .kp = 2.3_r,
//             // .kp = 0,
//             .ki = 0.0277_r,
//             // .ki = 0.0001_r,
//             // .out_min = -0.3_r,
//             // .out_max = 0.3_r
    
//             // .kp = 2.3_r,
//             // .ki = 0.009_r,
//             .out_min = -0.5_r,
//             .out_max = 0.5_r
//         }};
//     };

//     CurrentCtrl curr_ctrl_  = {};
//     TorqueCtrl torque_ctrl_ = {};
//     TraditionalSpeedCtrl spd_ctrl_ = {};
//     TraditionalPositionCtrl pos_ctrl_ = {};

//     void tick(){
//         odo_.update();

//         const auto targ_pos = real_t(0);
//         const auto lap_pos = odo_.getLapPosition();
//         const auto meas_pos = odo_.getPosition();
//         const auto meas_spd = odo_.getSpeed();

//         const real_t meas_rad = (frac(frac(lap_pos - 0.25_r) * 7) * real_t(TAU));

//         curr_sensor_.update(meas_rad);
//         const auto meas_dq_curr = curr_sensor_.dq();

//         const auto cmd_spd = pos_ctrl_.update(targ_pos, meas_pos, meas_spd);
//         const auto cmd_torque = spd_ctrl_.update(cmd_spd, meas_spd);
//         const auto cmd_dq_curr = torque_ctrl_.update(cmd_torque);
//         const auto cmd_dq_volt = curr_ctrl_.update(cmd_dq_curr, meas_dq_curr);

//         const auto cmd_ab_volt = cmd_dq_volt.to_ab(meas_rad);

//         svpwm_.set_ab_volt(cmd_ab_volt[0], cmd_ab_volt[1]);
//     }
// private:



// };

    // [[maybe_unused]] auto smo_ob = SmoObserver{{
    //     0.7_r, 
    //     0.04_r,
    //     8.22_r, 
    //     0.3_r
    // }};
    // [[maybe_unused]] RolbgObserver lbg_ob;

    // [[maybe_unused]] NonlinearObserver nlr_ob = {
    //     {
    //         .phase_inductance = 1.45E-3_r,
    //         .phase_resistance = 1.2_r,
    //         .observer_gain = 0.2_r,
    //         .pm_flux_linkage = 3.58e-4_r,
    //         .freq = FOC_FREQ,
    //     }
    // };

    // uint32_t dt;

    // std::array<real_t, 2> ab_volt;

    // scexpr real_t r_ohms = 7.1_r;
    // scepxr real_t l_mh = 1.45_r;

    // scexpr iq_t<16> pll_freq = iq_t<16>(0.2);
    [[maybe_unused]]
    LapPosPll pll = {
        {
            // .kp = real_t(2 * pll_freq),
            .kp = 470.17_r,
            // .ki = real_t(pll_freq * pll_freq),
            .ki = 20.3_r,
            .fc = FOC_FREQ
        }
    };



class CurrentBiasCalibrater{
public:
    struct Config{
        uint32_t period_ticks;
        uint32_t fc;
        uint32_t fs;
    };

    using Lpf = LowpassFilter<iq_t<16>>;
    using Lpfs = std::array<Lpf, 3>;

    Lpfs lpfs_ = {};

protected:
    uint32_t period_ticks_;
    uint32_t elapsed_ticks_;
    uint32_t fs_;

    // real_t last_midp_curr_ = 0;
public:
    CurrentBiasCalibrater(const Config & config){
        reconf(config);
        reset();
    }

    void reconf(const Config & config){
        period_ticks_ = config.period_ticks;
        fs_ = config.fs;

        // const auto alpha = Lpf::solve_alpha(config.fc, config.fs);
        lpfs_[0].reconf({config.fc, config.fs});
        lpfs_[1].reconf({config.fc, config.fs});
        lpfs_[2].reconf({config.fc, config.fs});

    }

    void reset(){
        elapsed_ticks_ = 0;
        for(auto & lpf : lpfs_){
            lpf.reset();
        }
    }

    void update(const UvwCurrent & uvw, const real_t mid_point){
        lpfs_[0].update(uvw.u);
        lpfs_[1].update(uvw.v);
        lpfs_[2].update(uvw.w);
        elapsed_ticks_ ++;

        // constexpr auto stable_curr_slewrate = 10.0_r;
        // constexpr auto stable_threshold = stable_curr_slewrate / FOC_FREQ;

        // const auto mid_point_diff = ABS(mid_point - last_midp_curr_);
        // last_midp_curr_ = mid_point;

        // curr_stable_checker.update(mid_point_diff < stable_threshold);
    }

    bool is_done(){
        return elapsed_ticks_ >= period_ticks_;
    }

    UvwCurrent result() const{
        return {
            lpfs_[0].get(),
            lpfs_[1].get(),
            lpfs_[2].get(),
        };
    }
};
#endif