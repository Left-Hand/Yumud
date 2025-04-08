#include "selfcheck.hpp"

using namespace ymd::foc;

void SelfCheckTasker::run(){
    switch(sub_state){
        case SubState::INIT:
            sw_state(SubState::TEST_MAG);
            break;

        case SubState::TEST_MAG:
            odo_.update();

            if(not odo_.encoder.stable()){
                // throw_error(ErrorCode::ODO_NO_SIGNAL, ("odometer is not stable"));
                // return RunStatus::ERROR;
                
            }else{
                odo_.reset();
                odo_.update();
                move_range = Range_t<real_t>::from_center(odo_.getPosition(), 0);
                sw_state(SubState::TEST_A);
            }
            break;

        case SubState::TEST_A:
            // svpwm.set_ab_current(sin(cnt * real_t((PI / subdivide_micros))) * check_current, idle_current);

            odo_.update();
            move_range = move_range.merge(odo_.getPosition());
            
            if(cnt > subdivide_micros * check_times){
                if(move_range.length() < minimal_motion){
                    // THROW_ERROR(ErrorCode::COIL_A_DISCONNECTED, ("coil a connection failed" + String(move_range)).c_str());
                    // return RunStatus::ERROR;
                }else{
                    move_range = Range_t<real_t>::from_center(odo_.getPosition(), 0);
                    sw_state(SubState::TEST_B);
                }
            }
            break;

        case SubState::TEST_B:
            // svpwm.set_ab_current(idle_current, sin(cnt * real_t(PI / subdivide_micros)) * check_current);

            odo_.update();
            move_range = move_range.merge(odo_.getPosition());

            if(cnt > subdivide_micros * check_times){
                if(move_range.length() < minimal_motion){
                    // THROW_ERROR(ErrorCode::COIL_B_DISCONNECTED, ("coil b connection failed" + (move_range).toString()).c_str());
                    // return RunStatus::ERROR;
                }else{
                    move_range = Rangei::from_center(odo_.getPosition(), 0);
                    sw_state(SubState::DONE);
                }
            }
            break;

        case SubState::CHECK_DIR:
            sw_state(SubState::STOP);
            break;
        case SubState::STOP:

            sw_state(SubState::DONE);
            break;
        case SubState::DONE:
            break;
    }
    cnt++;
}

void SelfCheckTasker::reset(){
    sub_state = SubState::INIT;
    cnt = 0;
}

bool SelfCheckTasker::done(){
    return sub_state == SubState::DONE;
}