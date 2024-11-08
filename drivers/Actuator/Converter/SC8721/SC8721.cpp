#include "SC8721.hpp"

using namespace yumud::drivers;

void SC8721::update(){

}

bool SC8721::verify(){
    return true;
}

void SC8721::reset(){

}


void SC8721::setTargetVoltage(const real_t volt){
    uint16_t data = int(volt * 50);
}

void SC8721::enableExternalFb(const bool en){
   
}