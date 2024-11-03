#include "icm42688.hpp"

using namespace yumud::drivers;

void ICM42688::init(){

}

void ICM42688::update(){

}


bool ICM42688::verify(){
    return false;
}

void ICM42688::reset(){

}

std::tuple<real_t, real_t, real_t> ICM42688::getAccel(){
    return {0,0,0};
}


std::tuple<real_t, real_t, real_t> ICM42688::getGyro(){
    return {0,0,0};
}