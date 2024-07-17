#include "smc.h"


void SmartCar::ctrl(){
    Sys::Clock::reCalculateTime();

    static constexpr real_t target_dir = real_t(PI / 2);
    body.update();
    measurer.update();

    {
        static constexpr real_t wheel_l = 0.182;
        odo.update();

        real_t now_pos = odo.getPosition() * wheel_l;
        static real_t last_pos = now_pos;
        
        real_t pos_delta = now_pos - last_pos;
        last_pos = now_pos;

        real_t now_spd = pos_delta * ctrl_freq;
        
        msm.front_spd = now_spd;
        // DEBUG_PRINTLN(now_pos, front_spd);
    }

    real_t turn_output = turn_ctrl.update(target_dir, msm.current_dir, measurer.get_omega());

    real_t side_acc = measurer.get_accel().y;
    real_t side_volocity = side_velocity_observer.update(msm.lane_offset, side_acc);

    //-----------------
    //控制器输出
    real_t side_output = side_ctrl.update(0, msm.lane_offset, -side_volocity);

    real_t centripetal_output = centripetal_ctrl.update(msm.front_spd, -msm.omega);

    // DEBUG_VALUE(centripetal_output);

    real_t speed_output = velocity_ctrl.update(setp.targ_spd, msm.front_spd);
    //-----------------

    if(switches.hand_mode == false){
        motor_strength.left = turn_output;
        motor_strength.right = -turn_output;

        motor_strength.left += speed_output;
        motor_strength.right += speed_output;

        motor_strength.hri = side_output;
        motor_strength.hri += centripetal_output;
    }

}
