#include "robots/foc/stepper/stepper.hpp"

FOCStepper::RunStatus FOCStepper::check_task(const FOCStepper::InitFlag init_flag){
    static constexpr int subdivide_micros = 1024;
    static constexpr int check_times = 2;
    static constexpr real_t minimal_motion = inv_poles / 8 * real_t(0.9);
    static constexpr real_t check_current = real_t(1.2);
    static constexpr real_t idle_current = 0;

    enum class SubState{
        INIT,
        TEST_MAG,
        TEST_A,
        TEST_B,
        CHECK_DIR,
        STOP,
        DONE
    };

    static SubState sub_state = SubState::INIT;
    static int cnt;

    auto sw_state = [](const SubState new_state){
        sub_state = new_state;
        cnt = 0;
    };

    if(init_flag){
        run_status = RunStatus::CHECK;
        sub_state = SubState::INIT;
        cnt = 0;
        return RunStatus::NONE;
    }

    static Range_t<real_t> move_range;
    {
        switch(sub_state){
            case SubState::INIT:
                sw_state(SubState::TEST_MAG);
                break;

            case SubState::TEST_MAG:
                odo.update();

                if(not odo.encoder.stable()){
                    throw_error(ErrorCode::ODO_NO_SIGNAL, ("odometer is not stable"));
                    return RunStatus::ERROR;
                }else{
                    odo.reset();
                    odo.update();
                    move_range = Range::from_center(odo.getPosition(), 0);
                    sw_state(SubState::TEST_A);
                }
                break;

            case SubState::TEST_A:
                svpwm.setABCurrent(sin(cnt * real_t((PI / subdivide_micros))) * check_current, idle_current);

                odo.update();
                move_range = move_range.merge(odo.getPosition());
                
                if(cnt > subdivide_micros * check_times){
                    if(move_range.length() < minimal_motion){
                        // THROW_ERROR(ErrorCode::COIL_A_DISCONNECTED, ("coil a connection failed" + String(move_range)).c_str());
                        return RunStatus::ERROR;
                    }else{
                        move_range = Range::from_center(odo.getPosition(), 0);
                        sw_state(SubState::TEST_B);
                    }
                }
                break;

            case SubState::TEST_B:
                svpwm.setABCurrent(idle_current, sin(cnt * real_t(PI / subdivide_micros)) * check_current);

                odo.update();
                move_range = move_range.merge(odo.getPosition());

                if(cnt > subdivide_micros * check_times){
                    if(move_range.length() < minimal_motion){
                        // THROW_ERROR(ErrorCode::COIL_B_DISCONNECTED, ("coil b connection failed" + (move_range).toString()).c_str());
                        return RunStatus::ERROR;
                    }else{
                        move_range = Rangei::from_center(odo.getPosition(), 0);
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
                return RunStatus::EXIT;
                break;
        }
        cnt++;
    }
    return RunStatus::NONE;
}

