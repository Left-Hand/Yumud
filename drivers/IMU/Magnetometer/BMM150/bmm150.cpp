#include "bmm150.hpp"

using namespace ymd;
using namespace ymd::drivers;


using Error = ImuError;

template<typename T = void>
using IResult= Result<T, Error>;

void BMM150::init(){

}

void BMM150::update(){

}

bool BMM150::validate(){
    return true;
}

void BMM150::reset(){

}

IResult<Vector3<q24>> BMM150::read_mag(){
    TODO();
    return Ok{Vector3<q24>{0,0,0}};
}
