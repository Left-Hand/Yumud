#include "BMI270.hpp"

using namespace ymd;
using namespace ymd::drivers;

using Error = BMI270::Error;

template<typename T = void>
using IResult = Result<T, Error>;

IResult<> BMI270::init(){
    return Ok();
}


IResult<> BMI270::update(){
    return Ok();
}


IResult<> BMI270::validate(){
    return Ok();
}


IResult<> BMI270::reset(){
    return Ok();
}

