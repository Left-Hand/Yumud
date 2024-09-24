#include "m3508.hpp"

using M3508 = M3508Port::M3508;

void M3508::init(){
    odo_.init();
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

    curr_delta = real_t(0.02);
    curr_setpoint = 0;
}


void M3508::tick(){
    odo_.update();

    real_t expect_curr;
    switch(ctrl_method){
        default:
        case CtrlMethod::NONE:
            break;
        case CtrlMethod::CURR:
            expect_curr = targ_curr;
            break;
        case CtrlMethod::SPD:
            expect_curr = spd_pid.update(targ_spd, getSpeed());
            break;
        case CtrlMethod::POS:
            expect_curr = pos_pid.update(targ_pos, getPosition());
            break;
    }

    curr_setpoint = STEP_TO(curr_setpoint, expect_curr, curr_delta);
    applyTargetCurrent(curr_setpoint);
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
    targ_pos = pos;
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
        }
    }

    
    if(size > 0){
        while(can.pending() >= 3);
        can.write(CanMsg{0x200, tx_datas[0]});
    }

    if(size > 4){
        while(can.pending() >= 3);
        can.write(CanMsg{0x1FF, tx_datas[1]});
    }
}