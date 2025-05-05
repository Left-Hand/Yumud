#include "MPU9250.hpp"

using namespace ymd;
using namespace ymd::drivers;

using Error = MPU9250::Error;

Result<void, Error> MPU9250::init(){

    return Ok{};
}

Result<void, Error> MPU9250::validate(){

    return Ok{};
}

Result<void, Error> MPU9250::enableChainMode(Enable en){
    // https://blog.csdn.net/u010205478/article/details/52739149

    
    //     MPU9250_WR_Reg(MPU9250_ADDR,INT_PIN_CFG,0x02);//Set PassBy
    // i2c_dev = MPU9250_RD_Reg(MAG_ADDR,0x00);//Read MAG ID,should be 0x48

    delay(5);
    return Ok{};
}