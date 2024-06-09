#ifndef __STEPPER_HPP__

#define __STEPPER_HPP__


#include "../debug/debug_inc.h"
#include "../enum.h"
#include "../src/timer/timers/timer_hw.hpp"



// class T0{
//     BETTER_ENUM(s, uint8_t,     
//     INIT = 0,
//     INACTIVE,
//     ACTIVE,
//     BEEP,
//     CALI,
//     ERROR,
//     EXCEPTION
// )
// };


namespace StepperEnums{
    BETTER_ENUM(RunStatus, uint8_t,     
        INIT = 0,
        INACTIVE,
        ACTIVE,
        BEEP,
        CALI,
        ERROR,
        EXCEPTION
    )
};

class Stepper{
public:
    using RunStatus = StepperEnums::RunStatus;
    Printer & logger = uart1;

    TimerOutChannelPosOnChip & verfChannelA = timer3[3];
    TimerOutChannelPosOnChip & verfChannelB = timer3[2];

    Coil1 coilA{portA[10], portA[11],  verfChannelA};
    Coil1 coilB{portA[8], portA[9],  verfChannelB};

    SVPWM2 svpwm{coilA, coilB};

    SpiDrv mt6816_drv{spi1, 0};
    MT6816 mt6816{mt6816_drv};

    OdometerPoles odo = OdometerPoles(mt6816);

    real_t target_pos;



    static constexpr uint32_t foc_freq = 36000;
    static constexpr uint32_t est_freq = 1800;
    static constexpr uint32_t est_devider = foc_freq / est_freq;

    static constexpr int poles = 50;
    static constexpr float inv_poles = 1.0 / poles;
    static constexpr float openloop_current_limit = 0.3;


    static constexpr uint32_t foc_period_micros = 1000000 / foc_freq;


    uint32_t foc_pulse_micros;
    real_t est_speed;
    real_t raw_pos;
    real_t est_pos;
    real_t est_elecrad;

    real_t run_current;
    real_t run_elecrad;
    real_t elecrad_zerofix;

    struct Targets{
        real_t curr = real_t();
        real_t speed = real_t();
        real_t pos = real_t();
        real_t time = real_t();
    };

    Targets targets;

    struct Setpoints{

    };

    using Range = Range_t<real_t>;

    struct StallObserver{
        real_t max_position_error;
        real_t stall_speed_threshold;
        uint32_t entry_time_ms;
        uint32_t timeout_ms;

        static constexpr uint32_t no_stall_time_ms = 0;

        bool update(const real_t & target_position, const real_t & measured_position, const real_t & measured_speed){
            auto current_time_ms = millis();
            if(abs(measured_speed) < stall_speed_threshold){ // stall means low speed
                if(entry_time_ms == no_stall_time_ms){
                    if(abs(target_position - measured_position) > max_position_error){ //
                        entry_time_ms = current_time_ms;
                    }
                }else{
                    return (current_time_ms - entry_time_ms> timeout_ms);
                }
            }else{
                entry_time_ms = no_stall_time_ms;
            }
            return false;
        }
    };

    struct CurrentCtrl{
    public:
        real_t current_slew_rate = real_t(20.0 / foc_freq);
        real_t current_output = real_t(0);
        Range current_range{real_t(-0.4), real_t(0.4)};

        real_t update(const real_t & current_setpoint){
            real_t current_delta = CLAMP(current_setpoint - current_output, -current_slew_rate, current_slew_rate);
            current_output = current_range.clamp(current_output + current_delta);
            return current_output;
        }
    };

    struct TorqueCtrl{
        CurrentCtrl & currCtrl;

    };

    struct SpeedCtrl{
        real_t kp = real_t(0.03);
        real_t ki;

        real_t kp_clamp = real_t(0.1);
        real_t intergal;
        real_t intergal_clamp;
        real_t ki_clamp;



        real_t current_output;
        real_t elecrad_offset_output;

        real_t elecrad_addition;
        real_t elecrad_addition_clamp = real_t(0.8);


        bool inited = false;
        auto update(const real_t & goal_speed,const real_t & measured_speed){
            if(!inited){
                inited = true;
                elecrad_offset_output = real_t(0);
                elecrad_addition = real_t(0);
            }

            real_t error = goal_speed - measured_speed;
            if(goal_speed < 0) error = - error;
            // real_t abs_error = abs(error);

            real_t kp_contribute = CLAMP(error * kp, -kp_clamp, kp_clamp);
            intergal = CLAMP(intergal + error, -intergal_clamp, intergal_clamp);
            real_t ki_contribute = CLAMP(intergal * ki, -ki_clamp, ki_clamp);

            elecrad_addition = CLAMP(elecrad_addition + kp_contribute + ki_contribute, real_t(0), elecrad_addition_clamp);

            real_t elecrad_offset = real_t(PI / 2) + elecrad_addition;
            if(goal_speed < 0) elecrad_offset = - elecrad_offset;

            // current_output = targets.curr;
            // elecrad_offset_output = elecrad_offset;

            // return std::make_tuple(current_output, elecrad_offset + odo.getElecRad());
            //TODO
            return std::make_tuple(current_output, elecrad_offset);
        }

        real_t getElecradOffset(){
            return elecrad_offset_output;
        }

        real_t getCurrent(){
            return current_output;
        }
    };


    struct PositionCtrl{
        real_t kp;
        real_t ki;
        real_t kd;
        real_t ks;

        real_t kp_clamp;
        real_t intergal;
        real_t ki_clamp;
        real_t kd_enable_speed_threshold;//minimal speed for activation kd
        real_t kd_clamp;
        real_t ks_enable_speed_threshold; // minimal speed for activation ks
        real_t ks_clamp;

        real_t target_position;
        real_t target_speed;

        real_t current_slew_rate = real_t(20.0 / foc_freq);
        real_t current_minimal = real_t(0.05);
        real_t current_clamp = real_t(0.3);

        real_t err_to_current_ratio = real_t(20);

        real_t current_output;
        real_t elecrad_output;
        real_t last_error;

        auto update(const real_t & _target_position, const real_t & measured_position, const real_t & measuresd_speed){
            target_position = _target_position;
            real_t error = target_position - measured_position;

            // real_t kp_contribute = CLAMP(error * kp, -kp_clamp, kp_clamp);

            // if((error.value ^ last_error.value) & 0x8000){ // clear intergal when reach target
            //     intergal = real_t(0);0
            // }else{
            //     intergal += error;
            // }

            // real_t ki_contribute = CLAMP(intergal * ki, -ki_clamp, ki_clamp);

            // real_t kd_contribute;
            // if(abs(measuresd_speed) > kd_enable_speed_threshold){ // enable kd only highspeed
            //     kd_contribute =  CLAMP(- measuresd_speed * kd, -kd_clamp, kd_clamp);
            // }else{
            //     kd_contribute = real_t(0);
            // }

            // real_t speed_error = target_speed - measuresd_speed;
            // real_t ks_contribute;
            // if(abs(speed_error) > ks_enable_speed_threshold){
            //     ks_contribute = CLAMP(speed_error * ks, -ks_clamp, ks_clamp);
            // }else{
            //     ks_contribute = real_t(0);
            // }

            // last_error = error;

            // real_t current_delta = CLAMP(kp_contribute + ki_contribute, -current_slew_rate, current_slew_rate);
            // current_output = CLAMP(current_output + current_delta, -current_clamp, current_clamp);
            // // return current_slew_rate;

            if(true){
                real_t abs_error = abs(error);
                current_output = CLAMP(abs_error * err_to_current_ratio, current_minimal, current_clamp);
                // if(abs(error) < inv_poles * 2){
                if(false){
                    // elecrad_offset_output = error * poles * TAU;
                    // elecrad_output = odo.position2rad(target_position);
                    //TODO
                    // elecrad_output = real_t(0);
                }else{
                    // elecrad_output = est_elecrad + (error > 0 ? 2.5 : - 2.5);
                    //TODO
                    // elecrad_output = real_t(0);
                }
            }

            return std::make_tuple(current_output, elecrad_output);
        }
    };

    std::vector<String> splitString(const String& input, char delimiter) {
        std::vector<String> result;

        int startPos = 0;
        int endPos = input.indexOf(delimiter, startPos);

        while (endPos != -1) {
            String token = input.substring(startPos, endPos);
            result.push_back(token.c_str());

            startPos = endPos + 1;
            endPos = input.indexOf(delimiter, startPos);
        }

        if (startPos < input.length()) {
            String lastToken = input.substring(startPos);
            result.push_back(lastToken.c_str());
        }

        return result;
    }

    void setCurrent(const real_t & _current, const real_t & _elecrad){
        coilA = cos(_elecrad) * _current;
        coilB = sin(_elecrad) * _current;
    }

    real_t openloop_elecrad;
    SpeedCtrl ctrl;
    CurrentCtrl currCtrl;
    PositionCtrl posctrl;

    RunStatus run_status = RunStatus::INIT;

    using DoneFlag = bool;
    using InitFlag = bool;

    enum class ErrorCode:uint8_t{
        OK = 0,
        COIL_A_DISCONNECTED,
        COIL_A_NO_SIGNAL,
        COIL_B_DISCONNECTED,
        COIL_B_NO_SIGNAL,
        ODO_DISCONNECTED,
        ODO_NO_SIGNAL
    };

    ErrorCode error_code = ErrorCode::OK;
    String error_message;

    bool auto_shutdown_activation = true;
    bool auto_shutdown_actived = false;
    uint16_t auto_shutdown_timeout_ms = 200;
    uint16_t auto_shutdown_last_wake_ms = 0;

    bool shutdown_when_error_occurred;
    bool shutdown_when_exception_occurred;

    void shutdown(){
        coilA.enable(false);
        coilB.enable(false);
    }

    void wakeup(){
        coilA.enable(true);
        coilB.enable(true);
    }

    struct ShutdownFlag{
    protected:
        bool state = false;
    public:

        ShutdownFlag() = default;

        auto & operator = (const bool & _state){
            state = _state;

            //TODO
            // if(state) shutdown();
            // else wakeup();

            return *this;
        }

        operator bool() const{
            return state;
        }
    };


    ShutdownFlag shutdown_flag;

    bool cali_debug_enabled = false;
    bool command_debug_enabled = true;
    bool run_debug_enabled = true;

    #define CALI_DEBUG(...)\
    if(cali_debug_enabled){\
    logger.println(__VA_ARGS__);};

    #define COMMAND_DEBUG(...)\
    if(command_debug_enabled){\
    logger.println(__VA_ARGS__);};

    #define RUN_DEBUG(...)\
    if(run_debug_enabled){\
    logger.println(__VA_ARGS__);};

    void throw_error(const ErrorCode & _error_code,const char * _error_message) {
        error_message = String(_error_message);
        run_status = RunStatus::ERROR;
        if(shutdown_when_error_occurred){
            shutdown_flag = true;
        }
    }

    void throw_exception(const ErrorCode & ecode, const char * emessage){
        error_message = String(emessage);
        run_status = RunStatus::EXCEPTION;
        if(shutdown_when_exception_occurred){
            shutdown_flag = true;
        }
    }


    void parseCommand(const char & argc, const std::vector<String> & argv){
        switch(argc){
            case 'C':
                if(argv.size() == 0) goto syntax_error;
                targets.curr = real_t(argv[0]);
                COMMAND_DEBUG("SetCurrent to ", targets.curr);
            
                break;
            case 'E':
                if(argv.size() == 0) goto syntax_error;
                elecrad_zerofix = real_t(argv[0]);
                break;
            case 'S':
                if(argv.size() == 0) goto syntax_error;
                targets.speed = real_t(argv[0]);
                COMMAND_DEBUG("SetSpeed to ", targets.curr);
                break;
            case 'D':
                if(argv.size() == 0){
                    COMMAND_DEBUG("ShutdownState: ", bool(shutdown_flag));
                }else{
                    COMMAND_DEBUG("Shutdown Command REcved");
                    shutdown_flag = bool(argv[0]);
                }
                break;

            case 'I':
                if(argv.size() == 0){
                    COMMAND_DEBUG("Run state:", run_status._to_string());
                }
                // else{
                //     COMMAND_DEBUG("Run state switch Command Recved");
                //     if(RunStatus::_is_valid(argv[0].c_str())){
                //         run_status = RunStatus::_from_string_nocase(argv[0].c_str());
                //     }
                //     COMMAND_DEBUG("Run state switched to:", argv[0]);
                // }
                break;
            case 'G':
                if(argv.size() == 0){
                    COMMAND_DEBUG("cali_debug_enabled", cali_debug_enabled);
                    COMMAND_DEBUG("command_debug_enabled", command_debug_enabled);
                    COMMAND_DEBUG("run_debug_enabled", run_debug_enabled);
                }
                
                // else if{argv.size() == 1}
                break;
            case 'R':
                __disable_irq();
                NVIC_SystemReset();
            syntax_error:
                COMMAND_DEBUG("SyntexError", argc);
                break;
            default:
                break;
        }
    }

    void parseLine(const String & line){
        if(line.length() == 0) return;
        auto tokens = splitString(line, ' ');
        auto argc = tokens[0][0];
        tokens.erase(tokens.begin());
        parseCommand(argc, tokens);
    }


    DoneFlag active_prog(const InitFlag & init_flag = false){

        auto [curr_out, elecrad_out] = posctrl.update(10 * sin(t), est_pos, est_speed);
        

        run_current = real_t(-0.2);
        // run_elecrad = TAU * frac(t);

        // run_current = currCtrl.update(curr_out);
        run_elecrad = elecrad_out;

        // setCurrent(run_current, run_elecrad + elecrad_zerofix);
        setCurrent(run_current, odo.getElecRad() + real_t(PI / 2) + elecrad_zerofix);

        // uint32_t foc_begin_micros = nanos();
        odo.update();

        raw_pos = odo.getPosition();

        static real_t last_raw_pos;
        static uint32_t est_cnt;


        if(init_flag){
            est_pos = raw_pos;
            est_speed = real_t();
            last_raw_pos = raw_pos;
            est_cnt = 0;

            setCurrent(real_t(0), real_t(0));
        }

        if(auto_shutdown_activation){
            if(run_current){
                auto_shutdown_actived = false;
                wakeup();
                auto_shutdown_last_wake_ms = millis();
            }else{
                if(millis() - auto_shutdown_last_wake_ms > auto_shutdown_timeout_ms){
                    auto_shutdown_actived = true;
                    shutdown();
                    auto_shutdown_last_wake_ms = millis();
                    
                }
            }
        }

        real_t delta_raw_pos = raw_pos - last_raw_pos;
        last_raw_pos = raw_pos;

        est_pos = raw_pos + delta_raw_pos;
        est_elecrad = odo.getElecRad();
        // Fixed();

        {//estimate speed and update controller
            static real_t est_delta_raw_pos_intergal = real_t();

            est_cnt++;
            if(est_cnt == est_devider){ // est happens
                real_t est_speed_new = est_delta_raw_pos_intergal * (int)est_freq;

                est_speed.value = (est_speed_new.value + est_speed.value * 3) >> 2;

                est_delta_raw_pos_intergal = real_t();
                est_cnt = 0;

                ctrl.update(real_t(targets.speed), est_speed);
            }else{
                est_delta_raw_pos_intergal += delta_raw_pos;
            }
        }



        // uint32_t foc_end_micros = nanos();
        // foc_pulse_micros = foc_end_micros - foc_begin_micros;

        {
            // struct SpeedCtrl{
            //     real_t kp;
            //     real_t ki;
            //     real_t kd;
            //     real_t ks;

            //     real_t kp_clamp;
            //     real_t intergal;
            //     real_t ki_clamp;
            //     real_t kd_enable_speed_threshold;//minimal speed for activation kd
            //     real_t kd_clamp;
            //     real_t ks_enable_speed_threshold; // minimal speed for activation ks
            //     real_t ks_clamp;
            // };
        }

        return false;
    }

    DoneFlag cali_prog(const InitFlag & init_flag = false){
        enum class SubState{
            ALIGN,
            INIT,
            PRE_FORWARD,
            FORWARD,
            BREAK,
            REALIGN,
            PRE_BACKWARD,
            BACKWARD,
            PRE_LANDING,
            LANDING,
            STOP,
            ANALYSIS,
            EXAMINE,
            DONE
        };

        constexpr int forwardpreturns = 15;
        constexpr int forwardturns = 50;
        constexpr int backwardpreturns = forwardpreturns;
        constexpr int backwardturns = forwardturns;

        constexpr int landingpreturns = 15;
        constexpr int landingturns = 50;

        constexpr int subdivide_micros = 256;
        constexpr int align_ms = 100;
        constexpr int break_ms = 100;
        constexpr int stop_ms = 400;

        constexpr float cali_current = 0.3;
        constexpr float align_current = 0.4;


        static SubState sub_state = SubState::DONE;
        static uint32_t cnt = 0;
        // static real_t openloop_elecrad_step = real_t(TAU / subdivide_micros);

        static real_t raw_position_accumulate = real_t(0);
        static real_t last_raw_lap_position = real_t(0);

        static real_t landing_position_accumulate = real_t(0);

        // using real = real_t;

        static int openloop_pole;

        static std::array<std::pair<real_t, real_t>, 50> forward_test_data;
        static std::array<std::pair<real_t, real_t>, 50> backward_test_data;
        static std::array<real_t, 50> elecrad_test_data;

        static std::array<real_t, 50> forward_err;
        static std::array<real_t, 50> backward_err;

        if(init_flag){
            sub_state = SubState::ALIGN;
            cnt = 0;
            return false;
        }

        auto sw_state = [](const SubState & new_state){
            sub_state = new_state;
            cnt = 0;
        };

        auto accumulate_raw_position = [](const real_t & raw_lap_position){
            real_t deltaLapPosition = raw_lap_position - last_raw_lap_position;
            if(deltaLapPosition > real_t(0.5f)){
                deltaLapPosition -= real_t(1);
            }else if (deltaLapPosition < real_t(-0.5f)){
                deltaLapPosition += real_t(1);
            }

            raw_position_accumulate += deltaLapPosition;
            last_raw_lap_position = raw_lap_position;
        };

        if(sub_state == SubState::DONE){
            return true;
        }

        else{

            switch(sub_state){
                case SubState::ALIGN:
                    setCurrent(real_t(align_current), real_t(0));
                    if(cnt >= (int)((foc_freq / 1000) * align_ms)){
                        sw_state(SubState::INIT);
                    }
                    break;
                case SubState::INIT:
                    odo.reset();
                    odo.inverse();
                    odo.update();

                    sw_state(SubState::PRE_FORWARD);
                    break;

                case SubState::PRE_FORWARD:

                    setCurrent(real_t(cali_current), real_t(cnt % subdivide_micros) / real_t(subdivide_micros) * TAU);

                    if(cnt >= forwardpreturns * subdivide_micros){
                        odo.update();
                        raw_position_accumulate = real_t(0);
                        last_raw_lap_position = odo.getRawLapPosition();
                        openloop_pole = odo.getRawPole();

                        sw_state(SubState::FORWARD);
                    }
                    break;
                case SubState::FORWARD:
                    odo.update();
                    accumulate_raw_position(odo.getRawLapPosition());

                    setCurrent(real_t(cali_current), real_t(cnt % subdivide_micros) / real_t(subdivide_micros) * TAU);

                    if(cnt % subdivide_micros == 0){
                        openloop_pole++;

                        const uint8_t cali_index = odo.warp_mod(openloop_pole, 50);
                        real_t openloop_position = real_t(cnt / subdivide_micros) * real_t(inv_poles);

                        forward_test_data[cali_index].first = openloop_position;
                        forward_test_data[cali_index].second = raw_position_accumulate;

                        forward_err[cali_index] = openloop_position - raw_position_accumulate;
                    }

                    if(cnt >= forwardturns * subdivide_micros){
                        sw_state(SubState::BREAK);
                    }
                    break;
                case SubState::BREAK:
                    setCurrent(real_t(cali_current), real_t(0));
                    if(cnt >= (int)((foc_freq / 1000) * break_ms)){
                        sw_state(SubState::REALIGN);
                    }
                    break;
                case SubState::REALIGN:
                    setCurrent(real_t(align_current), real_t(0));
                    if(cnt >= (int)((foc_freq / 1000) * align_ms)){
                        sw_state(SubState::PRE_BACKWARD);
                    }
                    break;
                case SubState::PRE_BACKWARD:

                    setCurrent(real_t(cali_current), -real_t(cnt % subdivide_micros) / real_t(subdivide_micros) * TAU);

                    if(cnt >= backwardpreturns * subdivide_micros){
                        odo.update();
                        raw_position_accumulate = real_t(0);
                        last_raw_lap_position = odo.getRawLapPosition();
                        openloop_pole = odo.getRawPole();

                        sw_state(SubState::BACKWARD);
                    }
                    break;

                case SubState::BACKWARD:
                    odo.update();
                    accumulate_raw_position(odo.getRawLapPosition());

                    setCurrent(real_t(cali_current), -real_t(cnt % subdivide_micros) / real_t(subdivide_micros) * TAU);

                    if(cnt % subdivide_micros == 0){
                        openloop_pole--;

                        const uint8_t cali_index = odo.warp_mod(openloop_pole, 50);
                        real_t openloop_position = -real_t(cnt / subdivide_micros) * real_t(inv_poles);

                        backward_test_data[cali_index].first = openloop_position;
                        backward_test_data[cali_index].second = raw_position_accumulate;

                        backward_err[cali_index] = openloop_position - raw_position_accumulate;
                    }

                    if(cnt >= backwardturns * subdivide_micros){
                        real_t forward_mean = std::accumulate(std::begin(forward_err), std::end(forward_err), real_t(0)) / int(forward_err.size());
                        for(auto & forward_err_item : forward_err){
                            forward_err_item -= forward_mean;
                        }

                        real_t backward_mean = std::accumulate(std::begin(backward_err), std::end(backward_err), real_t(0)) / int(backward_err.size());
                        for(auto & backward_err_item : backward_err){
                            backward_err_item -= backward_mean;
                        }

                        for(uint8_t i = 0; i < poles; i++){
                            odo.cali_map[i] = mean(forward_err[i], backward_err[i]);
                        }

                        raw_position_accumulate = real_t(0);
                        sw_state(SubState::PRE_LANDING);
                    }
                    break;
                
                case SubState::PRE_LANDING:
                    odo.update();

                    setCurrent(real_t(cali_current), sin(real_t(cnt % (subdivide_micros * 2)) / real_t(subdivide_micros) * PI));
                    if(cnt >= landingpreturns * subdivide_micros){
                        landing_position_accumulate = real_t(0);
                        openloop_pole = 0;
                        odo.setElecRadOffset(real_t(0));
                        sw_state(SubState::LANDING);
                    }
                    break;
                case SubState::LANDING:
                    odo.update();
                    // accumulate_raw_position(odo.getRawLapPosition());

                    setCurrent(real_t(cali_current), sin(real_t(cnt % (subdivide_micros * 2)) / real_t(subdivide_micros) * PI));
                    if(cnt % subdivide_micros == 0){
                        landing_position_accumulate += raw_position_accumulate;
                        elecrad_test_data[openloop_pole] = odo.getElecRad();
                        openloop_pole++;
                    }

                    if(cnt >= landingturns * subdivide_micros){
                        sw_state(SubState::STOP);
                    }
                case SubState::STOP:
                    odo.update();

                    setCurrent(real_t(align_current), real_t(0));
                    if(cnt >= (int)((foc_freq / 1000) * stop_ms)){
                        sw_state(SubState::ANALYSIS);
                    }

                    
                    break;
                case SubState::ANALYSIS:
                    // odo.runCaliAnalysis();
                    // for(uint8_t i = 0; i < 50; i++){
                    //     odo.cali_map[i] = mean(forward_test_data[i].second, backward_test_data[i].second);
                    // }

                    // for(auto & forward_err_item : forward_err){
                    //     forward_err_item -= round(forward_err_item);
                    // }

                    // for(auto & backward_err_item : backward_err){
                    //     backward_err_item -= round(backward_err_item);
                    // }

                    // {
                    //     real_t forward_max = *std::max_element(std::begin(forward_err), std::end(forward_err));
                    //     real_t forward_min = *std::min_element(std::begin(forward_err), std::end(forward_err));
                    // }
                    // {
                    //     real_t landing_position_err = landing_position_accumulate / landingturns;
                    //     constexpr int iter_times = 160;
                    //     // for(uint8_t i = 0; i < iter_times; i ++){
                    //     //     odo.update();
                    //     //     // delayMicros
                    //     //     // CALI_DEBUG(odo.getRawLapPosition());
                    //     //     delayMicroseconds(20);
                    //     // }
                    //     for(uint8_t i = 0; i < iter_times; i ++){
                    //         odo.update();
                    //         CALI_DEBUG(odo.getRawLapPosition() - landing_position_err);
                    //     }
                    // }
                    odo.update();
                    // odo.fixElecRadOffset(odo.getLapPosition() - landing_position_accumulate / landingturns);

                    // odo.addPostDynamicFixPosition(-odo.warp_around_zero(landing_position_accumulate / landingturns));
                    // odo.fixElecRad();
                    // odo.adjustZeroOffset();
                    // setCurrent(real_t(align_current), real_t(PI / 2));
                    // delay(100);
                    // odo.update();
                    // CALI_DEBUG(l-anding_position_accumulate / landingturns, odo.getElecRad());

                    for(auto & item : elecrad_test_data){
                        CALI_DEBUG(item);
                    }

                    odo.setElecRadOffset(std::accumulate(std::begin(elecrad_test_data), std::end(elecrad_test_data), real_t(0)) / real_t(int(elecrad_test_data.size())));
                                    CALI_DEBUG(odo.getElecRad());
                    // setCurrent(real_t(align_current), real_t(0));
                    // delay(100);
                    // odo.update();
                    // CALI_DEBUG(odo.getRawLapPosition(), odo.getElecRad());

                    // setCurrent(real_t(align_current), real_t(-PI / 2));
                    // delay(100);
                    // odo.update();
                    // CALI_DEBUG(odo.getRawLapPosition(), odo.getElecRad());

                    // setCurrent(real_t(align_current), real_t(0));
                    // delay(100);
                    // odo.update();
                    // CALI_DEBUG(odo.getRawLapPosition(), odo.getElecRad());
                    sw_state(SubState::EXAMINE);
                    break;
                case SubState::EXAMINE:
                    // for(uint8_t i = 0; i < 50; i++){
                    //     // logger << forward_test_data[i].first << ", " << forward_test_data[i].second << ", " << forward_err[i] << ", " << backward_test_data[i].first << ", " << backward_test_data[i].second << ", " <<  backward_err[i] << endl;
                    //     CALI_DEBUG(forward_err[i], backward_err[i]);
                    // }

                    // for(uint8_t i = 0; i < poles; i++){
                        
                    // }
                    // logger.setEps(4);
                    // logger.setSpace(", ");
                    // for(uint8_t i = 0; i < poles; i++){
                    //     CALI_DEBUG(forward_err.at(i), backward_err.at(i));
                    // }

                    odo.locateRelatively(real_t(0));
                    setCurrent(real_t(0), real_t(0));
                    sw_state(SubState::DONE);
                    break;

                case SubState::DONE:
                    return true;
                    break;
                default:
                    break;
            }
            cnt++;
        }
        return false;
    }

    DoneFlag selfcheck_prog(const InitFlag & init_flag){
        constexpr int subdivide_micros = 2048;
        // constexpr int dur = 600;

        enum class SubState{
            INIT,
            TEST_A,
            REINIT,
            TEST_B,
            ANALYSIS,
            EXAMINE,
            STOP,
            DONE
        };

        static SubState sub_state = SubState::INIT;

        static int cnt;

        auto sw_state = [](const SubState & new_state){
            sub_state = new_state;
            cnt = 0;
        };

        if(init_flag){
            sub_state = SubState::INIT;
            cnt = 0;
        }

        static Range_t<real_t> move_range;

        if(cnt < 0){
            return false;
        }

        else{

            switch(sub_state){
                case SubState::INIT:
                    odo.reset();
                    odo.update();
                    odo.update();
                    move_range.start = odo.getPosition();
                    move_range.end = move_range.start;
                    break;
                case SubState::TEST_A:
                    coilA = cos(real_t(cnt) * real_t(PI / subdivide_micros));
                    
                    odo.update();
                    move_range.merge(odo.getPosition());
                    if(cnt > subdivide_micros) sw_state(SubState::TEST_B);
                case SubState::TEST_B:
                case SubState::ANALYSIS:
                default:
                    break;
            }
        }
        return false;
    }


    DoneFlag beep_prog(const InitFlag & init_flag = false){
        // drivedClass dri;
        // dri.run();
        struct Tone{
            uint32_t freq_hz;
            uint32_t sustain_ms;
        };

        constexpr int freq_G4 = 392;
        constexpr int freq_A4 = 440;
        constexpr int freq_B4 = 494;
        constexpr int freq_C5 = 523;
        constexpr int freq_D5 = 587;
        constexpr int freq_E5 = 659;
        constexpr int freq_F5 = 698;
        constexpr int freq_G5 = 784;

        static const auto tones = std::to_array<Tone>({
            {.freq_hz = freq_A4,.sustain_ms = 100},  // 6
            {.freq_hz = freq_D5,.sustain_ms = 100},  // 2
            {.freq_hz = freq_E5,.sustain_ms = 100},  // 3
            {.freq_hz = freq_G5,.sustain_ms = 100},  // 5
            {.freq_hz = freq_E5,.sustain_ms = 100},  // 3
            {.freq_hz = freq_D5,.sustain_ms = 100},  // 2
            
            {.freq_hz = freq_A4,.sustain_ms = 100},  // 6
            {.freq_hz = freq_D5,.sustain_ms = 100},  // 2
            {.freq_hz = freq_E5,.sustain_ms = 100},  // 3
            {.freq_hz = freq_G5,.sustain_ms = 100},  // 5
            {.freq_hz = freq_E5,.sustain_ms = 100},  // 3
            {.freq_hz = freq_D5,.sustain_ms = 100},  // 2
            
            {.freq_hz = freq_B4,.sustain_ms = 100},  // 7
            {.freq_hz = freq_C5,.sustain_ms = 100},  // 1
            {.freq_hz = freq_B4,.sustain_ms = 100},  // 7
            {.freq_hz = freq_G4,.sustain_ms = 100},  // 5

            {.freq_hz = freq_F5,.sustain_ms = 100}   // 6
        });

        constexpr float tone_current = 0.4;

        static uint32_t cnt;
        static uint32_t tone_index;
        static uint32_t play_begin_ms;
        // static Tone * tone = nullptr;

        if(init_flag){
            cnt = 0;
            tone_index = 0;
            play_begin_ms = millis();
        }

        if(millis() >= tones[tone_index].sustain_ms + play_begin_ms){ // play one note done
            if(tone_index >= tones.size()) return true; // play done
            else{
                tone_index++;
                play_begin_ms = millis();
            }
        }
        
        {
            // auto play_us = cnt * (1000000 / foc_freq);
            const auto & tone = tones[tone_index];
            auto tone_cnt = foc_freq / tone.freq_hz / 2;
            // auto tone_ms = tones[tone_index].sustain_ms;
            bool phase = (cnt / tone_cnt) % 2;
            setCurrent(real_t(tone_current), phase ? real_t(0.5) : real_t(-0.5));
            cnt++;
        }
        // for(const auto & tone : tones){
        //     uint32_t play_begin_ms = millis();
        //     uint32_t tone_period_us = 1000000 / tone.freq_hz;
        //     tone_period_us /= 2;
        //     while(millis() - play_begin_ms < tone.sustain_ms){
        //         // svpwm.setDQCurrent(Vector2(real_t(tone_current), real_t(0)), real_t());
        //         
        //         delayMicroseconds(tone_period_us);
        //         setCurrent(tone_current, );
        //         // svpwm.setDQCurrent(Vector2(real_t(0), real_t(tone_current)), real_t());
        //         delayMicroseconds(tone_period_us);
        //     }
        // }

        return false;
    }

    class PanelLed{
    public:
        enum class Method:uint8_t{
            Sine = 0,
            Saw,
            Squ,
            Tri
        };
    protected:
        RgbLedConcept<true> & led;

        using Color = Color_t<real_t>;

        Color color_a;
        Color color_b;
        uint16_t period;

        Method method;
    public:
        PanelLed(RgbLedConcept<true> & _led) : led(_led){;}

        void init(){
            led.init();
        }

        void setPeriod(const uint16_t & _period){
            period = _period;
        }

        void setTranstit(const Color & _color_a, const Color & _color_b, const Method & _method){
            color_a = _color_a;
            color_b = _color_b;
            method = _method;
        }

        void run(){
            real_t ratio;
            real_t _t = t * real_t(100000 / period) * real_t(0.01);
            switch(method){
            case Method::Saw:
                ratio = frac(_t);
                break;
            case Method::Sine:
                ratio = abs(2 * frac(_t) - 1);
                break;
            case Method::Squ:
                ratio = sign(2 * frac(_t) - 1) * 0.5 + 0.5;
                break;
            case Method::Tri:
                ratio = abs(2 * frac(_t) - 1);
                break;
            }

            Color color_mux = color_a.linear_interpolate(color_b, ratio);
            led = color_mux;
        }
    };

    RgbLedDigital<true> led_instance{portC[14], portC[15], portC[13]};
    PanelLed panel_led = PanelLed{led_instance};



    void run(){
        switch(run_status){
            case RunStatus::INIT:
                run_status = RunStatus::CALI;
                cali_prog(true);
                break;
            case RunStatus::CALI:
                if(cali_prog()){
                    // beep_prog(true);
                    // run_status = RunStatus::BEEP;
                    active_prog(true);
                    run_status = RunStatus::ACTIVE;
                }
                break;
            
            case RunStatus::BEEP:
                if(beep_prog()){
                    active_prog(true);
                    run_status = RunStatus::ACTIVE;
                }
                break;

            case RunStatus::INACTIVE:
                run_status = RunStatus::ACTIVE;
                break;
            case RunStatus::ACTIVE:
                if(active_prog()){
                    run_status = RunStatus::INACTIVE;
                }
                break;
            default:
                break;
        }
    }

    void stepper_test(){

        uart1.init(115200 * 4);

        logger.setEps(4);

        timer1.init(4096, 1, Timer::TimerMode::CenterAlignedDownTrig);
        timer1.enableArrSync();

        timer3.init(1024, 1, Timer::TimerMode::CenterAlignedDownTrig);
        timer3.enableArrSync();

        svpwm.init();

        coilA.setClamp(real_t(1));
        coilB.setClamp(real_t(1));

        coilA.init();
        coilB.init();

        spi1.init(18000000);
        spi1.bindCsPin(portA[15], 0);



        odo.init();

        // adc1.init(
        //     {},{
        //         AdcChannelConfig{.channel = AdcChannels::CH3, .sample_cycles = AdcSampleCycles::T71_5},
        //         AdcChannelConfig{.channel = AdcChannels::CH4, .sample_cycles = AdcSampleCycles::T71_5}
        //     });
        // adc1.setRegularTrigger(AdcOnChip::RegularTrigger::SW);
        // adc1.setInjectedTrigger(AdcOnChip::InjectedTrigger::SW);
        // timer3[4] = 0;
        // adc1.setPga(AdcOnChip::Pga::X64);
        // adc1.setInjectedTrigger(AdcOnChip::InjectedTrigger::T3CC4);
        // TIM3->CH4CVR = TIM3->ATRLR >> 1;
        // adc1.enableCont();
        // adc1.enableScan();
        // adc1.enableAutoInject();

        // auto & bled = portC[13];
        // bled.OutPP();
        panel_led.init();


        // svpwm.setDQCurrent(Vector2(real_t(cali_current), real_t(0)), real_t(0));
        // setCurrent(real_t(0.3), real_t());
        // delay(200);
        // odo.locateElecrad();

        timer4.init(foc_freq);
        timer4.enableIt(Timer::IT::Update, NvicPriority(0, 0));
        timer4.bindCb(Timer::IT::Update, [&](){this->run();});

        // delay(200);
        // motor.enable();
        // motor.trackPos(real_t(0));
        // motor.setMaxCurrent(real_t(0.45));

        logger.setEps(4);
        panel_led.setPeriod(200);
        panel_led.setTranstit(Color(), Color(0,1,0,0), PanelLed::Method::Squ);
        while(true){
            // real_t total = real_t(3);
            // static real_t freq = real_t(10);
            // static real_t freq_dir = real_t(1);
            // const real_t freq_delta = real_t(20);
            // if(freq > real_t(300)) freq_dir = real_t(-1);
            // else if(freq < real_t(4)) freq_dir = real_t(1);
            // static real_t last_t = t;
            // real_t delta = (t - last_t);
            // freq += delta * freq_dir * freq_delta;
            // last_t = t;
            // static real_t ang = real_t(0);
            // ang += freq * delta;
            // real_t target = (total / freq) * sin(ang);

            // target_pos = sign(frac(t) - 0.5);
            // target_pos = sin(t);
            // RUN_DEBUG(odo.getPosition(), est_pos, est_speed, ctrl.elecrad_offset_output, odo.getRawLapPosition(), odo.getLapPosition());
            // RUN_DEBUG(est_speed, est_pos, targets.curr, elecrad_zerofix);
            // , est_speed, t, odo.getElecRad(), openloop_elecrad);
            // logger << est_pos << est_speed << run_current << elecrad_zerofix << endl;
            // RUN_DEBUG(est_pos, est_speed, run_current, elecrad_zerofix);
            // RUN_DEBUG(est_pos, est_speed, run_current, run_elecrad);
            static String temp_str = "";

            // bool led_status = (millis() / 200) % 2;
            // bled = led_status;
            panel_led.run();

            if(logger.available()){
                char chr = logger.read();
                if(chr == '\n'){
                    temp_str.trim();
                    // RUN_DEBUG(temp_str);
                    if(temp_str.length()) parseLine(temp_str);
                    // RUN_DEBUG(temp_str);
                    temp_str = "";
                }else{
                    temp_str.concat(chr);
                }
            }

            Sys::Clock::reCalculateTime();
        }
        while(true);
    }
};

#endif