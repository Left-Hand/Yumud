#include "ina3221.hpp"
#include "sys/debug/debug.hpp"

using namespace ymd::drivers;

#define INA3221_DEBUG

#ifdef INA3221_DEBUG
#undef INA3221_DEBUG
#define INA3221_DEBUG(...) DEBUG_PRINTLN(__VA_ARGS__);
#define INA3221_PANIC(...) PANIC{__VA_ARGS__}
#define INA3221_ASSERT(cond, ...) ASSERT{cond, ##__VA_ARGS__}
#define READ_REG(reg) readReg(reg.address, reg.as_ref()).loc().expect();
#define WRITE_REG(reg) writeReg(reg.address, reg.as_val()).loc().expect();
#else
#define INA3221_DEBUG(...)
#define INA3221_PANIC(...)  PANIC_NSRC()
#define INA3221_ASSERT(cond, ...) ASSERT_NSRC(cond)
#define READ_REG(reg) +readReg(reg.address, reg.as_ref());
#define WRITE_REG(reg) +writeReg(reg.address, reg.as_val());
#endif


using DeviceResult = INA3221::DeviceResult;
INA3221 & INA3221::init(){
    INA3221_ASSERT(verify(), "INA3221 verify failed");

    this->enableChannel(1);
    this->setShuntConversionTime(INA3221::ConversionTime::_140us);
    this->setBusConversionTime(INA3221::ConversionTime::_140us);
    this->setAverageTimes(INA3221::AverageTimes::_1);
    this->enableContinuous();
    this->enableMeasureBus();
    this->enableMeasureShunt();
    // while(true){
    //     INA3221_DEBUG(config_reg);
    // }
    return *this;
}

bool INA3221::ready(){
    return true;
}


bool INA3221::verify(){

    INA3221_ASSERT(i2c_drv.verify().ok(), "INA3221 drv lost");

    READ_REG(chip_id_reg);
    READ_REG(manu_id_reg);

    return 
        INA3221_ASSERT(chip_id_reg.key == (chip_id_reg.as_val()), "wrong chip id", chip_id_reg.as_val()) and 
        INA3221_ASSERT(manu_id_reg.key == (manu_id_reg.as_val()), "wrong manu id", manu_id_reg.as_val());
}

INA3221 & INA3221::update(){
    readBurst(shuntvolt1_reg.address, &shuntvolt1_reg, 6);

    return *this;
}
 
INA3221 & INA3221::update(const size_t index){
    readBurst(shuntvolt1_reg.address + ((index - 1) * 2), &shuntvolt1_reg + ((index - 1) * 2), 2);
    // READ_REG(config_reg);
    // INA3221_DEBUG(config_reg.as_val());
    // switch(index){
    //     case 1: READ_REG(shuntvolt1_reg); READ_REG(busvolt1_reg); break;
    //     case 2: READ_REG(shuntvolt2_reg); READ_REG(busvolt2_reg); break;
    //     case 3: READ_REG(shuntvolt3_reg); READ_REG(busvolt3_reg); break;
    // }
    return *this;
} 

INA3221 & INA3221::setAverageTimes(const uint16_t times){
    uint8_t temp = CTZ(times);
    uint8_t temp2;

    if(times <= 64){
        temp2 = temp >> 1;
    }else{
        temp2 = 4 + (temp - 7); 
    }

    config_reg.average_times = temp2;
    WRITE_REG(config_reg);

    return *this;
}

void INA3221::setAverageTimes(const AverageTimes times){
    config_reg.average_times = uint8_t(times);
    WRITE_REG(config_reg);
}

INA3221 & INA3221::enableChannel(const size_t index, const bool en){
    switch(index){
        default: INA3221_PANIC();
        case 1:
            config_reg.ch1_en = en;
            break;
        case 2:
            config_reg.ch2_en = en;
            break;
        case 3:
            config_reg.ch3_en = en;
            break;
    }
    WRITE_REG(config_reg);

    return *this;
}


INA3221 & INA3221::setBusConversionTime(const ConversionTime time){
    config_reg.bus_conv_time = uint8_t(time);
    WRITE_REG(config_reg);

    return *this;
}


INA3221 & INA3221::setShuntConversionTime(const ConversionTime time){
    config_reg.shunt_conv_time = uint8_t(time);
    WRITE_REG(config_reg);

    return *this;
}


INA3221 & INA3221::reset(){
    config_reg.rst = true;
    WRITE_REG(config_reg);
    config_reg.rst = false;

    return *this;
}


int INA3221::getShuntVoltuV(const size_t index){

    RegAddress addr;
    ShuntVoltReg & reg = [&]() -> ShuntVoltReg &{
        switch(index){
        default: INA3221_PANIC();
        case 1:
            addr = shuntvolt1_reg.address;
            return shuntvolt1_reg;
        case 2:
            addr = shuntvolt2_reg.address;
            return shuntvolt2_reg;
        case 3:
            addr = shuntvolt3_reg.address;
            return shuntvolt3_reg;
        }
    }();

    +readReg(addr, reg.as_ref());

    return reg.to_uv();
}



int INA3221::getBusVoltmV(const size_t index){
    RegAddress addr;
    BusVoltReg & reg = [&]() -> BusVoltReg &{
        switch(index){
        default: INA3221_PANIC();
        case 1:
            addr = busvolt1_reg.address;
            return busvolt1_reg;
        case 2:
            addr = busvolt2_reg.address;
            return busvolt2_reg;
        case 3:
            addr = busvolt3_reg.address;
            return busvolt3_reg;
        }
    }();

    +readReg(addr, reg.as_ref());

    return reg.to_mv();
}


real_t INA3221::getShuntVolt(const size_t index){
    return iq_t<16>(iq_t<8>(getShuntVoltuV(index)) / 100) / 10000;
    // return real_t(real_t(getShuntVoltuV(index))) / 10000;
}

real_t INA3221::getBusVolt(const size_t index){
    return real_t(getBusVoltmV(index)) / 1000;
}


INA3221 & INA3221::setInstantOVC(const size_t index, const real_t volt){
    RegAddress addr;
    switch(index){
        default: INA3221_PANIC();
        case 1:
            addr = instant_ovc1_reg.address1;
            break;
        case 2:
            addr = instant_ovc2_reg.address2;
            break;
        case 3:
            addr = instant_ovc3_reg.address3;
            break;
    }

    +writeReg(addr, ShuntVoltReg::to_i16(volt));

    return *this;
}


INA3221 & INA3221::setConstantOVC(const size_t index, const real_t volt){
    RegAddress addr;
    switch(index){
        default: INA3221_PANIC();
        case 1:
            addr = constant_ovc1_reg.address1;
            break;
        case 2:
            addr = constant_ovc2_reg.address2;
            break;
        case 3:
            addr = constant_ovc3_reg.address3;
            break;
    }

    +writeReg(addr, ShuntVoltReg::to_i16(volt));

    return *this;
}

void INA3221::enableMeasureBus(const bool en){
    config_reg.bus_measure_en = en;
    WRITE_REG(config_reg);
}


void INA3221::enableMeasureShunt(const bool en){
    config_reg.shunt_measure_en = en;
    WRITE_REG(config_reg);
}

void INA3221::enableContinuous(const bool en){
    config_reg.continuos = en;
    WRITE_REG(config_reg);
}