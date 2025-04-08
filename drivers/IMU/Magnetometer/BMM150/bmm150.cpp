#include "bmm150.hpp"

using namespace ymd;
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

Option<Vector3_t<real_t>> BMM150::get_magnet(){
    return Some{Vector3_t<real_t>{0,0,0}};
}
