#include "robots/stepper/stepper.hpp"

    Stepper::ExitFlag Stepper::selfcheck_task(const Stepper::InitFlag init_flag){
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

