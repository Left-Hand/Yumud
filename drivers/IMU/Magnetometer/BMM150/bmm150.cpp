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

Option<Vector3R> BMM150::getMagnet(){
    return Some{Vector3R{0,0,0}};
}
