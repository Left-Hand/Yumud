#pragma once

#include "drivers/device_defs.h"

//http://www.baiheee.com/Documents/091203/091203224635.htm

namespace ymd::drivers{
class SDcard{
protected:
    struct CommandContent{
        
    };
    
    struct Command{
        const uint8_t __header__:1 = 0b0;
        const uint8_t cmd_info:1 = 0b1;
        
        const uint8_t content1:6;
        const uint8_t content2:8;
        const uint8_t content3:8;
        const uint8_t content4:8;
        const uint8_t content5:8;
        
        uint8_t crc:7;
        const uint8_t __end__:1 = 0b1;
    };

    struct Response:public Reg16<>{

    };



    struct CID{
        uint64_t:1;
        uint64_t crc:7;
        uint64_t mdt:12;
        uint64_t :4;
        uint64_t psn:32;
        uint64_t prv:8;
        uint64_t pnm:40;
        uint64_t oid:16;
        uint64_t mid:8;
    };

    hal::SpiDrv spi_drv_;

    void writeCommand(const Command & command){
        spi_drv_.end();
        spi_drv_.forceWrite<uint8_t>(0xFF);
        
    }
    

public:
    SDcard(const hal::SpiDrv & spi_drv):spi_drv_(spi_drv){;}
    SDcard(hal::SpiDrv && spi_drv):spi_drv_(std::move(spi_drv)){;}
    SDcard(hal::Spi & spi, const uint8_t index):spi_drv_(hal::SpiDrv(spi, index)){;}
    
    void init();
};

}