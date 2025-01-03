#include "bkp.hpp"
#include "sys/core/sdk.h"

using namespace ymd;

void Bkp::init(){
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);
}

void Bkp::writeData(uint8_t index, uint16_t data){
    if(index <= 0 || index > 10) return;
    PWR_BackupAccessCmd(ENABLE);
    BKP_WriteBackupRegister(index << 2, data);
    PWR_BackupAccessCmd(DISABLE);
}

uint16_t Bkp::readData(uint8_t index){
    if(index <= 0 || index > 10) return 0;
    return BKP_ReadBackupRegister(index << 2);
}

BkpItem & Bkp::operator [] (uint8_t index) {
    if(index <= 0 || index > 10) return items[0];
    return items[index - 1];
}


void BkpItem::store(const uint16_t data){
    bkp.writeData(index, data);
}
uint16_t BkpItem::load(){
    return bkp.readData(index);
}
