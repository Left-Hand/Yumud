#include "wheels.hpp"

using namespace ymd::robots;

#define WHEELS_ASSERT(...) ASSERT(__VA_ARGS__)

void Wheels::init(){
    for(Wheel & wheel : instances_){
        auto & motor = wheel.motor();
        motor.reset();
        // wheel->reset();
    }
    
    clock::delay(10ms);

    for(Wheel & wheel : instances_){
        auto & motor = wheel.motor();
        motor.locateRelatively(0);
        static constexpr auto curr = 0.0_r;
        motor.setOpenLoopCurrent(curr);
        motor.setCurrentLimit(curr);
    }
}


bool Wheels::validate(){
    return true;
}

void Wheels::freeze(){
    for(Wheel & wheel : instances_){
        wheel.freeze();
    }
}

void Wheels::request(){
    // for(auto & inst : instances_){
    //     inst.motor().getPosition();
    // }

    auto & wheels = *this;
    static int i = 0;
    i = (i+1)%4;

    while(can_.available()){
        auto msg = can_.read();
        
        bool accepted = false;
        for(size_t j = 0; j < 4; j++){
            auto & motor = wheels[j].motor();
            if(motor.update(msg)){
                accepted = true;
                break;
            }
        }

        if(!accepted){
            // DEBUG_PRINTLN(msg.id() >> 7);
        }
    }


    switch(i){
        case 0:
            // wheels[0].setPosition(0.1_r * sin(t));
            wheels[1].updatePosition();
            break;
        case 1:
            // wheels[1].setPosition(0.2_r * sin(t));
            wheels[2].updatePosition();
            break;
        case 2:
            // wheels[2].setPosition(0.3_r * sin(t));
            wheels[3].updatePosition();
            break;
        case 3:
            // wheels[3].setPosition(0.4_r * sin(t));
            wheels[0].updatePosition();
            break;
    }

    // auto & can = instances_
};

void Wheels::setPosition(const std::tuple<real_t, real_t, real_t, real_t> & pos){
    #define SET_POS(n) instances_[n].setPosition(std::get<n>(pos))
    // #define SET_POS(n) instances_[n].setVector(std::get<n>(pos))
    // DEBUG_PRINTLN("?????", pos)
    SET_POS(0);
    SET_POS(1);
    SET_POS(2);
    SET_POS(3);
    #undef SET_POS
}

void Wheels::setCurrent(const std::tuple<real_t, real_t, real_t, real_t> & curr){
    #define SET_CURR(n) instances_[n].setCurrent(CLAMP2(std::get<n>(curr), config_.max_curr));
    SET_CURR(0);
    SET_CURR(1);
    SET_CURR(2);
    SET_CURR(3);
    #undef SET_CURR
}

void Wheels::setSpeed(const std::tuple<real_t, real_t, real_t, real_t> & spd){
    #define SET_SPD(n) instances_[n].setSpeed(std::get<n>(spd))
    SET_SPD(0);
    SET_SPD(1);
    SET_SPD(2);
    SET_SPD(3);
    #undef SET_SPD
}

void Wheels::setDelta(const std::tuple<real_t, real_t, real_t, real_t> & pos){
    #define FWD_POS(n) instances_[n].forwardPosition(std::get<n>(pos))
    FWD_POS(0);
    FWD_POS(1);
    FWD_POS(2);
    FWD_POS(3);
    #undef FWD_POS
}


void Wheels::forward(const std::tuple<real_t, real_t, real_t, real_t> & pos){
    #define FWD_POS(n) instances_[n].forwardPosition(std::get<n>(pos))
    FWD_POS(0);
    FWD_POS(1);
    FWD_POS(2);
    FWD_POS(3);
    #undef FWD_POS
}


std::tuple<real_t, real_t, real_t, real_t> Wheels::getPosition(){
    return {
        instances_[0].readPosition(),
        instances_[1].readPosition(),
        instances_[2].readPosition(),
        instances_[3].readPosition()
    };
}