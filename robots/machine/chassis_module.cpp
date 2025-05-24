#include "chassis_actions.hpp"
#include "chassis_ctrl.hpp"



using namespace ymd::robots;
using namespace ChassisActions;

void ChassisModule::recalibrate(const Ray2q<16> & ray){
    TODO();
}



void ChassisModule::freeze(){
    wheels_.freeze();
}


void ChassisModule::closeloop(){

    switch(ctrl_mode_){
        case CtrlMode::NONE:
            // DEBUG_PRINTLN("no mode");
            break;

        case CtrlMode::STRICT_SHIFT:{
            auto p1234 = solver_.inverse(Ray2q<16>{target_jny_.org, 0});
            auto p1 = last_motor_positions[0] +  std::get<0>(p1234);
            auto p2 = last_motor_positions[1] +  std::get<1>(p1234);
            auto p3 = last_motor_positions[2] +  std::get<2>(p1234);
            auto p4 = last_motor_positions[3] +  std::get<3>(p1234);
            // DEBUG_PRINTLN(p1, p2, p3, p4, target_rot_);
            setPosition({p1, p2, p3, p4});
        }
            break;
        case CtrlMode::STRICT_SPIN:{
            auto p1234 = solver_.inverse(Ray2q<16>{0,0, target_rot_});
            auto p1 = last_motor_positions[0] +  std::get<0>(p1234);
            auto p2 = last_motor_positions[1] +  std::get<1>(p1234);
            auto p3 = last_motor_positions[2] +  std::get<2>(p1234);
            auto p4 = last_motor_positions[3] +  std::get<3>(p1234);
            // DEBUG_PRINTLN(p1, p2, p3, p4, target_rot_);
            setPosition({p1, p2, p3, p4});
        }
            break;
        case CtrlMode::SHIFT:{
            auto rot_output = rot_ctrl_.update(0, this->rad(), this->gyr());
            auto pos_output = pos_ctrl_.update(target_jny_.org, this->jny().org, this->spd());
            setCurrent(Ray2q<16>{pos_output, rot_output});
        }
            break;
        case CtrlMode::SPIN:{
            auto rot_output = rot_ctrl_.update(target_rot_, this->rad(), this->gyr());
            setCurrent(Ray2q<16>{Vector2q<16>{0,0}, rot_output});    
        }
            break;
    }
}


void ChassisModule::entry_spin(){
    ctrl_mode_ = CtrlMode::SPIN;
    reset_rot();
    reset_journey();
}

void ChassisModule::entry_strict_spin(){
    ctrl_mode_ = CtrlMode::STRICT_SPIN;
    reset_rot();
    reset_journey();
}

void ChassisModule::entry_shift(){
    ctrl_mode_ = CtrlMode::SHIFT;
    reset_rot();
    reset_journey();
}

void ChassisModule::entry_strict_shift(){
    ctrl_mode_ = CtrlMode::STRICT_SHIFT;
    reset_rot();
    reset_journey();
}

void ChassisModule::setCurrent(const Ray2q<16> & ray){
    auto && curr = solver_.inverse(ray);
    // DEBUG_PRINTLN(curr)
    wheels_.setCurrent(curr);
    // wheels_.setSpeed(curr);
}

void ChassisModule::setPosition(const Ray2q<16> & ray){
    // DEBUG_PRINTLN("!!!!", ray);
    auto && pos = solver_.inverse(ray);
    // DEBUG_PRINTLN(curr)
    // DEBUG_PRINTLN("????", pos);
    wheels_.setPosition(pos);
    // wheels_.setSpeed(curr);
}

void ChassisModule::setPosition(const std::tuple<real_t, real_t, real_t, real_t> pos){
    wheels_.setPosition(pos);
}



void ChassisModule::trim(const Ray2q<16> & ray){
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
    gyr_sum_ = 0;
}

void ChassisModule::init(){
    auto init_steppers = [&](){
        for(size_t i = 0; i < 4; i++){
            auto & stp_ = wheels_[i].motor();
            stp_.reset();
            stp_.locateRelatively(0);
        }    
    };

    init_steppers();

    auto & self = *this;
    self << new FreezeAction(self);
    self << new DelayAction(2000);
}

void ChassisModule::tick800(){
    wheels_.request();

    static int i = 0;
    i = (i + 1) %4;
    if(i == 0){

        // auto time = Sys::t;
        // auto delta = solver.inverse(Vector2q<16>{0, 0.00_r}, 0.7_r);
        // wheels.setCurrent(delta);
        // acc_gyr_sensor_.update();
        // mag_sensor_.update();

        // auto mag = Vector3<real_t>(mag_sensor_.read_mag());
        // gyr_ = Vector3<real_t>(acc_gyr_sensor_.read_gyr()).z + real_t(0.0035);
        // gyr_ = Vector3<real_t>(acc_gyr_sensor_.read_gyr()).z + real_t(0.005);
        // gyr_ = Vector3<real_t>(acc_gyr_sensor_.read_gyr()).z + real_t(0.002);
        auto gyr_raw = Vector3<real_t>(gyr_sensor_.read_gyr().unwrap()).z + real_t(0.00113);
        // auto gyr_raw = Vector3<real_t>(acc_gyr_sensor_.read_gyr()).z;
        // auto gyr_raw = Vector3<real_t>(acc_gyr_sensor_.read_gyr()).z + real_t(0.00625);
        // auto gyr_raw = Vector3<real_t>(acc_gyr_sensor_.read_gyr()).z + real_t(0.00525);
        // gyr_ = Vector3<real_t>(acc_gyr_sensor_.read_gyr()).z - real_t(0.009);
        // auto gyr_raw = Vector3<real_t>(acc_gyr_sensor_.read_gyr()).z - real_t(0.009);
        // auto gyr_raw = Vector3<real_t>(acc_gyr_sensor_.read_gyr()).z - real_t(0.134);

        static KalmanFilter_t<real_t> kf{10,0.01_r};
        gyr_ = ABS(gyr_raw) > 3 ? real_t(0) : kf.update(gyr_raw);


        DEBUG_PRINTLN(gyr_, rad());

        gyr_sum_ += gyr_;

        auto calculate_journey = [this]() -> Ray2q<16>{
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




//侧向移动
void ChassisModule::sideways(const real_t dist){
    auto & self = *this;
    self << new SideAction(self, dist);
}

//径向移动
void ChassisModule::straight(const real_t dist){
    auto & self = *this;
    self << new StraightAction(self, dist);
}

//平移
void ChassisModule:: shift(const Vector2q<16> & diff){
    auto & self = *this;
    self << new ShiftAction(self, diff);
}

//旋转
void ChassisModule::spin(const real_t ang){
    auto & self = *this;
    self << new SpinAction(self, ang);
}

void ChassisModule::strict_spin(const real_t ang){
    auto & self = *this;
    self << new StrictSpinAction(self, ang);
}

void ChassisModule::strict_shift(const Vector2q<16> & offs){
    auto & self = *this;
    self << new StrictShiftAction(self, offs);
}

// void ChassisModule::wait(const real_t dur){
//     auto & self = *this;
//     self << new DelayAction(int(dur * 1000));
// }

void ChassisModule::follow(const Ray2q<16> & to){
    auto & self = *this;
    auto from = Ray2q<16>{{0,0}, real_t(PI/2)};

    auto p_opt = from.intersection(to);
    auto p = p_opt.value();

    self.straight(p.y);
    self.spin(min_rad_diff(real_t(PI/2), to.rad));
    self.straight((to.org - p).length());
    // self << new ShiftAction(self, to.org);
    // self << new SpinAction(self, to.rad);
}