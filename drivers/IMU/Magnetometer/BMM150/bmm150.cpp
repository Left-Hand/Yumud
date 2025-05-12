#include "bmm150.hpp"

using namespace ymd;
using namespace ymd::drivers;


void BMM150::init(){

}

void BMM150::update(){

}

bool BMM150::validate(){
    return true;
}

void BMM150::reset(){

}

Option<Vector3_t<q24>> BMM150::read_mag(){
    return Some{Vector3_t<q24>{0,0,0}};
}
