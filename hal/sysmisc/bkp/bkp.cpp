#include "bkp.hpp"
#include "core/sdk.hpp"

using namespace ymd::hal;


#define BKP_DEBUG

#ifdef BKP_DEBUG
__inline void BKP_ASSERT(bool x){
    if(!x) __builtin_trap();
}

#define BKP_GUARD_RANK     BKP_ASSERT(rank != 0 || rank <= 10);
#else
#define BKP_ASSERT(x)
#define BKP_GUARD_RANK
#endif



void Bkp::init(){
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);
}

void Bkp::store(uint8_t rank, uint16_t data){
    BKP_GUARD_RANK
    PWR_BackupAccessCmd(ENABLE);
    BKP_WriteBackupRegister(rank << 2, data);
    PWR_BackupAccessCmd(DISABLE);
}

uint16_t Bkp::load(uint8_t rank){
    BKP_GUARD_RANK
    return BKP_ReadBackupRegister(rank << 2);
}

BkpItem Bkp::get(uint8_t rank) {
    BKP_GUARD_RANK
    return BkpItem(rank);
}

void BkpItem::store(const uint16_t data){
    Bkp::store(rank_, data);
}

uint16_t BkpItem::load(){
    return Bkp::load(rank_);
}