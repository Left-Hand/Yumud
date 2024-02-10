#include "bkp.hpp"

Bkp* Bkp::instance = nullptr;

void Bkp::init(){
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);
}

void Bkp::writeData(uint8_t index, uint16_t data){
    if(!index || index > 10) return;
    PWR_BackupAccessCmd(ENABLE);
    BKP_WriteBackupRegister(index << 2, data);
    PWR_BackupAccessCmd(DISABLE);
}

uint16_t Bkp::readData(uint8_t index){
    if(!index || index > 10) return 0;
    return BKP_ReadBackupRegister(index << 2);
}

BkpItem Bkp::operator [] (uint8_t index) {
    return BkpItem(index);
}