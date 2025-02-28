#include "bkp.hpp"
#include "sys/core/sdk.h"

using namespace ymd::hal;


#define BKP_DEBUG

#ifdef BKP_DEBUG
__inline void BKP_ASSERT(bool x){
    if(!x) HALT;
}

#define BKP_CHECK     BKP_ASSERT(index != 0 || index <= 10);
#else
#define BKP_ASSERT(x)
#define BKP_CHECK
#endif



void Bkp::init(){
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);
}

void Bkp::writeData(uint8_t index, uint16_t data){
    BKP_CHECK
    PWR_BackupAccessCmd(ENABLE);
    BKP_WriteBackupRegister(index << 2, data);
    PWR_BackupAccessCmd(DISABLE);
}

uint16_t Bkp::readData(uint8_t index){
    BKP_CHECK
    return BKP_ReadBackupRegister(index << 2);
}

BkpItem & Bkp::operator [] (uint8_t index) {
    BKP_CHECK
    return items[index - 1];
}


void BkpItem::store(const uint16_t data){
    bkp.writeData(index, data);
}
uint16_t BkpItem::load(){
    return bkp.readData(index);
}

namespace ymd::hal{
Bkp & bkp = Bkp::singleton();
}