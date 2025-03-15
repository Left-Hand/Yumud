#include "BMI088.hpp"
#include "sys/debug/debug.hpp"

using namespace ymd;
using namespace ymd::drivers;


void BMI088::init(){

}

void BMI088::update(){

}

Option<Vector3R> BMI088::getAcc(){
    return Some{Vector3R{0,0,0}};
}

Option<Vector3R> BMI088::getGyr(){
    return Some{Vector3R{0,0,0}};
}

real_t BMI088::getTemperature(){
    return 0;
}
