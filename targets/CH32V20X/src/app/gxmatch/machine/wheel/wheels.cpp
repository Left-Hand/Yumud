#include "wheels.hpp"

using namespace gxm;

#define WHEELS_ASSERT(...) ASSERT(__VA_ARGS__)

void Wheels::init(){
    //TODO
    WHEELS_ASSERT(verify());
}
bool Wheels::verify(){
    return true;
}

void Wheels::update(){
    for(auto & inst : instances_){
        inst.motor().getPosition();
    }

    // auto & can = instances_
};

void Wheels::setPosition(const std::tuple<real_t, real_t, real_t, real_t> & pos){
    #define SET_POS(n) instances_[n].setPosition(std::get<n>(pos))
    SET_POS(0);
    SET_POS(1);
    SET_POS(2);
    SET_POS(3);
}

void Wheels::setDelta(const std::tuple<real_t, real_t, real_t, real_t> & pos){
    #define FWD_POS(n) instances_[n].forwardPosition(std::get<n>(pos))
    FWD_POS(0);
    FWD_POS(1);
    FWD_POS(2);
    FWD_POS(3);
}


void Wheels::forward(const std::tuple<real_t, real_t, real_t, real_t> & pos){
    #define FWD_POS(n) instances_[n].forwardPosition(std::get<n>(pos))
    FWD_POS(0);
    FWD_POS(1);
    FWD_POS(2);
    FWD_POS(3);
}


std::tuple<real_t, real_t, real_t, real_t> Wheels::getPosition(){
    return {
        instances_[0].getPosition(),
        instances_[1].getPosition(),
        instances_[2].getPosition(),
        instances_[3].getPosition()
    };
}