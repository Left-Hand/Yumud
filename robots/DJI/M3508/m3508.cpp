#include "m3508.hpp"

using M3508 = M3508Port::M3508;
#define LIMIT 1000

void M3508::init(){
    odo_.init();
    port.occupation.set(index - 1, 1);
}

void M3508::reset(){
    odo_.reset();

    lap_position = 0;
    curr = 0;
    speed = 0;
    temperature = 0;

    spd_pid = {3, 0, 0};
    pos_pid = {3, 0, 0};

    ctrl_method = CtrlMethod::NONE;
    
    targ_curr = 0;
    targ_spd = 0;
    targ_pos = 0;

    curr_delta = real_t(0.04);
    curr_setpoint = 0;
}


void M3508::tick(){
    uint32_t this_micros = micros();
    micros_delta =this_micros - last_micros; 
    last_micros = this_micros;
    real_t expect_curr = 0;
    switch(ctrl_method){
        default:
        case CtrlMethod::NONE:
            break;
        case CtrlMethod::CURR:
            expect_curr = targ_curr;
            break;
        case CtrlMethod::SPD:
            // // expect_curr = spd_pid.update(targ_spd, getSpeed());
            //             // expect_curr = pos_pid.update(targ_pos, getPosition());
            // scexpr real_t targ_spd = 4;
            // real_t pos_err = targ_pos - getPosition();
            // real_t spd_err = targ_spd - getSpeed();

            // scexpr real_t kp = real_t(30); 
            // scexpr real_t kd = real_t(7);
            
            // expect_curr = sqrt(ABS(pos_err)) * sign(pos_err) * kp + spd_err * kd;  
            // // expect_curr = (pos_err) * kp + spd_err * kd;  
            // break;
            break;
        case CtrlMethod::POS:{
            targ_spd = (targ_spd * 15 + targ_spd_ester.update(targ_pos)) >> 4;
            real_t pos_err = targ_pos - getPosition();
            real_t spd_err = targ_spd - getSpeed();

            // scexpr real_t kp = real_t(2.35); 
            // scexpr real_t kd = real_t(0.85);

            scexpr real_t kp = real_t(2.75); 
            scexpr real_t kd = real_t(1.45);
            
            // scexpr real_t kd = real_t(1.95);

            // scexpr real_t kp = real_t(4); 
            // scexpr real_t kd = real_t(2.75);
            
            // scexpr real_t kp = real_t(6); 
            // scexpr real_t kd = real_t(3.75);

            // scexpr real_t kp = real_t(8); 
            // scexpr real_t kd = real_t(7.45);

            // scexpr real_t kp = real_t(9); 
            // scexpr real_t kd = real_t(10.45);

            // scexpr real_t kp = real_t(10); 
            // scexpr real_t kd = real_t(13.45);

            // scexpr real_t kp = real_t(11); 
            // scexpr real_t kd = real_t(16.45);

            // scexpr real_t ki = real_t(0.066);
            // scexpr real_t ki = real_t(0.0);
            // static real_t ci = 0;
            // ci += (ki * (delta()>>4) * pos_err) >> 12;
            // ci = CLAMP2(ci, curr_limit * 0.1);

            // if(SIGN_DIFF(pos_err, spd_err)) ci = SIGN_AS(curr_limit, pos_err);
            // scexpr real_t kd = real_t(13);
            // scexpr real_t kd = real_t(-20);
            
            expect_curr = sqrt(ABS(pos_err)) * sign(pos_err) * kp + spd_err * kd;  
            // expect_curr = (pos_err) * kp + spd_err * kd;  
            break;
        }
    }
    
    expect_curr = CLAMP2(expect_curr, curr_limit);
    curr_setpoint = STEP_TO(curr_setpoint, expect_curr, curr_delta);
    if(ctrl_method != CtrlMethod::NONE){
        applyTargetCurrent(curr_setpoint);
    }
}

void M3508::setTargetCurrent(const real_t curr){
    ctrl_method = CtrlMethod::CURR;
    targ_curr = curr;
}


void M3508::setTargetSpeed(const real_t spd){
    ctrl_method = CtrlMethod::SPD;
    targ_spd = spd;
}

void M3508::setTargetPosition(const real_t pos){
    ctrl_method = CtrlMethod::POS;
    targ_pos = pos;
}

void M3508::updateMeasurements(const real_t _lap_position, const real_t _curr, const real_t _spd, const real_t temp){
    odo_.update();
    lap_position = _lap_position;
    curr = _curr;
    // speed = _spd;
    speed = (speed * 15 + spd_ester.update(odo_.getPosition())) >> 4;
    temperature = temp;
}
        
void M3508Port::init(){
    for(auto & inst : inst_){
        inst.init();
    }
}


void M3508Port::reset(){
    for(auto & inst : inst_){
        inst.reset();
    }
}

void M3508Port::tick(){
    while(can.available()){
        const auto & msg = can.front();
        if(msg.id() > 0x200 and msg.id() <= 0x208){
            can.read();
            size_t index = msg.id() - 0x200;
            updateInst(msg, index);
        }else{
            break;
        }
    }

    
    if((occupation & std::bitset<8>(0x0f)).any()){
        // while(can.pending() >= 3);
        can.write(CanMsg{0x200, std::make_tuple(tx_datas[0])});
    }

    if((occupation & std::bitset<8>(0xf0)).any()){
        // while(can.pending() >= 3);
        can.write(CanMsg{0x1FF, std::make_tuple(tx_datas[1])});
    }

    for(size_t i = 0; i < size; i++){
        if(occupation.test(i)){
            inst_[i].tick();
        }
    }
}