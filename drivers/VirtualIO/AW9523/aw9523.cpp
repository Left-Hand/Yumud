#include "aw9523.hpp"

using namespace ymd;
using namespace ymd::drivers;

void AW9523::init(){
    reset();
    delay(10);
    setLedCurrentLimit(CurrentLimit::Low);
    for(uint8_t i = 0; i< 16; i++){
        write_reg((RegAddress)((uint8_t)RegAddress::dim + i), (uint8_t)0);
    }
    ledMode = 0xffff;
}


void AW9523::writeByIndex(const int index, const bool data){
    if(!isIndexValid(index))return;
    if(data) buf |= 1 << index;
    else buf &= ~(1 << index);
    writePort(buf);
}

bool AW9523::readByIndex(const int index){
    if(!isIndexValid(index)) return false;
    readPort();
    return (buf & (1 << index));
}

void AW9523::setMode(const int index, const hal::GpioMode mode){
    if(false == isIndexValid(index))return;
    uint16_t mask = 1 << index;
    if(hal::GpioUtils::isIn(mode)) dir |= mask;
    else dir &= ~mask;
    write_reg(RegAddress::dir, dir);

    if(index < 8){
        ctl.p0mod = hal::GpioUtils::isPP(mode);
        write_reg(RegAddress::ctl, ctl);
    }
}

void AW9523::enableIrqByIndex(const int index, const bool en ){
    if(false == isIndexValid(index))return;
    write_reg(RegAddress::inten, (uint8_t)(en << index));
}

void AW9523::enableLedMode(const hal::Pin pin, const bool en){
    uint index = CTZ((uint16_t)pin);
    if(en) ledMode &= ~(1 << index);
    else ledMode |= (1 << index);
    write_reg(RegAddress::ledMode, ledMode);
}

void AW9523::setLedCurrentLimit(const CurrentLimit limit){
    ctl.isel = (uint8_t)limit;
    write_reg(RegAddress::ctl, ctl);
}

void AW9523::setLedCurrent(const hal::Pin pin, const uint8_t current){
    uint index = CTZ((uint16_t)pin);
    if(index < 8) index += 4;
    else if(index < 12) index -= 8;
    write_reg((RegAddress)((uint8_t)RegAddress::dim + index), current);
}


bool AW9523::verify(){
    uint8_t chipId;
    read_reg(RegAddress::chipId, chipId);
    return (chipId == valid_chipid);
}
