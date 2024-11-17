#include "bmm150.hpp"

using namespace ymd::drivers;


void BMM150::init(){

}

void BMM150::update(){

}

bool BMM150::verify(){
    return true;
}

void BMM150::reset(){

}

std::tuple<real_t, real_t, real_t> BMM150::getMagnet(){
    return{0,0,0};
}
