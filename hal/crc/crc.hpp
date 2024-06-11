#ifndef __CRC_HPP__

#define __CRC_HPP__

#include "src/platform.h"
#include <initializer_list>

class Crc{
protected:
    // CRC_TypeDef * instance;

    // Crc(CRC_TypeDef * _instance):instance(_instance){;}
    Crc(){;}
public:
    static Crc & getInstance(){
        static auto crc_instance = Crc();
        return crc_instance;
    }
    void init(){
        RCC_AHBPeriphClockCmd(RCC_CRCEN, ENABLE);
    }

    void clear(){
        CRC_ResetDR();
    }

    uint32_t update(const std::initializer_list<uint32_t> & list){
        return CRC_CalcBlockCRC((uint32_t *)list.begin(), list.size());
    }
};

extern Crc & crc;
#endif