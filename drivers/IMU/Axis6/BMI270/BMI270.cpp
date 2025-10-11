#include "BMI270.hpp"

using namespace ymd;
using namespace ymd::drivers;

using Error = BMI270::Error;

template<typename T = void>
using IResult = Result<T, Error>;

IResult<> BMI270::init(){
    TODO();
    return Ok();
}


IResult<> BMI270::update(){
    TODO();
    return Ok();
}


IResult<> BMI270::validate(){
    TODO();
    return Ok();
}


IResult<> BMI270::reset(){
    TODO();
    return Ok();
}


IResult<> BMI270::set_pmu_mode(const PmuType pmu, const PmuMode mode){
    TODO();
    return Ok();
}

// PmuMode BMI270::get_pmu_mode(const PmuType pmu){
//     return Ok();
// }

// IResult<Vec3<q24>> BMI270::read_acc(){
//     uint8_t buf[6];
//     return Ok();
// }