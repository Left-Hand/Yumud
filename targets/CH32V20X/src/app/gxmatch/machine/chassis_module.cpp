#include "chassis_actions.hpp"
#include "chassis_ctrl.hpp"



using namespace gxm;
using namespace ChassisActions;

// void ChassisModule::positionTrim(const Vector2 & trim){
//     wheels_.setDelta(solver_.inverse(trim, 0));
// }

// void ChassisModule::rotationTrim(const real_t raderr){
//     wheels_.setDelta(solver_.inverse({0,0}, raderr));
// }

void ChassisModule::forwardMove(const Vector2 & vel, const real_t spinrate){
    // wheels_.setSpeed(solver_.inverse(vel, spinrate));
}

// void ChassisModule::calibrateRotation(const real_t rad){
    
// }


// void ChassisModule::calibratePosition(const Vector2 & pos){
    
// }

void ChassisModule::recalibrate(const Ray & ray){
    TODO();
}

void ChassisModule::test(){

}
    

void ChassisModule::freeze(){
    wheels_.freeze();
}

bool ChassisModule::arrived(){
    return false;
}

void ChassisModule::closeloop(){

    switch(ctrl_mode_){
        case CtrlMode::NONE:
            // DEBUG_PRINTLN("no mode");
            break;
        case CtrlMode::SHIFT:{
            auto rot_output = rot_ctrl_.update(0, this->rad(), this->gyr());
            auto pos_output = pos_ctrl_.update(target_jny_.org, this->jny().org, this->spd());
            setCurrent(Ray{pos_output, rot_output});
        }
            break;
        case CtrlMode::SPIN:{
            auto rot_output = rot_ctrl_.update(target_rot_, this->rad(), this->gyr());
            setCurrent(Ray{Vector2{0,0}, rot_output});    
        }
            break;
    }
}


void ChassisModule::entry_spin(){
    ctrl_mode_ = CtrlMode::SPIN;
    reset_rot();
    reset_journey();
}

void ChassisModule::entry_shift(){
    ctrl_mode_ = CtrlMode::SHIFT;
    reset_rot();
    reset_journey();
}

void ChassisModule::setCurrent(const Ray & ray){
    auto && curr = solver_.inverse(ray);
    // DEBUG_PRINTLN(curr)
    wheels_.setCurrent(curr);
    // wheels_.setSpeed(curr);
}


void ChassisModule::trim(const Ray & ray){
    auto & self = *this;
    self << new TrimAction(self, ray);
}

void ChassisModule::reset_journey(){
    // current_journey_ = 0;
    // last_motor_positions.fill(0);
    for(size_t i = 0; i < 4; i++){
        last_motor_positions[i] = wheels_[i].readPosition();
    }
}

void ChassisModule::reset_rot(){
    current_rot_ = 0;
}

void ChassisModule::tick800(){
    wheels_.request();

    static int i = 0;
    i = (i + 1) %4;
    if(i == 0){

        // auto time = Sys::t;
        // auto delta = solver.inverse(Vector2{0, 0.00_r}, 0.7_r);
        // wheels.setCurrent(delta);
        acc_gyr_sensor_.update();

        gyr_ = Vector3(acc_gyr_sensor_.getGyr()).z + real_t(0.0035);
        current_rot_ += gyr_ * real_t(1.0/200);

        auto calculate_journey = [this]() -> Ray{
            return solver_.forward(
                wheels_[0].readPosition() - last_motor_positions[0],
                wheels_[1].readPosition() - last_motor_positions[1],
                wheels_[2].readPosition() - last_motor_positions[2],
                wheels_[3].readPosition() - last_motor_positions[3]
            );
        };

        current_jny_ = calculate_journey();

        spd_ = (current_jny_.org - last_pos_) * 200;
        last_pos_ = current_jny_.org;

        // current_journey_ = ray_j.org.y;
        // DEBUG_PRINTLN(ray_j);
        // DEBUG_PRINTLN(current_journey_)
        // DEBUG_PRINTLN(
        //         wheels_[0].readPosition() - last_motor_positions[0],
        //         wheels_[1].readPosition() - last_motor_positions[1],
        //         wheels_[2].readPosition() - last_motor_positions[2],
        //         wheels_[3].readPosition() - last_motor_positions[3]
        // )
        closeloop();
    }
}



