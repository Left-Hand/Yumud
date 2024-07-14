#include "bkp.hpp"


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


BkpItem & BkpItem::operator = (const uint16_t data){
    bkp.writeData(index, data);
    return *this;
}
BkpItem::operator uint16_t(){
    return bkp.readData(index);
}


Bkp & bkp = Bkp::getInstance();