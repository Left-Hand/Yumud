#include "smc.h"

using namespace SMC;

void SmartCar::ctrl(){
    Sys::Clock::reCalculateTime();


    body.update();
    measurer.update();

    real_t turn_output = turn_ctrl.update(real_t(PI / 2),
            measurer.get_dir(),
            measurer.get_omega());

    real_t side_volocity = side_velocity_observer.update(
            measurer.get_lane_offset(switches.align_mode),
            measurer.get_accel().y);

    //-----------------
    //控制器输出
    real_t side_output = side_ctrl.update(0,
            measurer.get_lane_offset(switches.align_mode),
            -side_volocity);

    real_t centripetal_output = centripetal_ctrl.update(
            measurer.get_front_speed(),
            -measurer.get_omega());

    real_t speed_output = velocity_ctrl.update(
            setp.targ_spd,
            measurer.get_front_speed());
    //-----------------

    real_t min_strength = 0.1;

    if(switches.hand_mode == false){
        motor_strength.left = 0;
        motor_strength.right = 0;

        if(!is_blind){
            motor_strength.left += turn_output;
            motor_strength.right -= turn_output;
        }
        motor_strength.left += speed_output;
        motor_strength.right += speed_output;

        motor_strength.left = MAX(motor_strength.left, min_strength);
        motor_strength.right = MAX(motor_strength.right, min_strength);
        
        motor_strength.hri = 0;
        motor_strength.hri += side_output;
        motor_strength.hri += centripetal_output;

        if(is_blind){
            motor_strength.hri = 0;
        }
    }

}
