#include "crc.hpp"

using namespace yumud;

Crc & crc = Crc::getInstance();

void Crc::init(){
    #ifdef N32G45X
    RCC_EnableAHBPeriphClk(RCC_AHB_PERIPH_CRC, ENABLE);
    #else
    RCC_AHBPeriphClockCmd(RCC_CRCEN, ENABLE);
    #endif
}

void Crc::clear(){
    #ifdef N32G45X
    CRC32_ResetCrc();
    #else
    CRC_ResetDR();
    #endif
}

uint32_t Crc::update(const uint32_t * begin, const uint32_t * end){
    #ifdef N32G45X
    return CRC32_CalcBufCrc(begin, end - begin);
    #else
    return CRC_CalcBlockCRC(begin, end - begin);
    #endif
}