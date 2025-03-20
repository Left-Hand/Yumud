#pragma once

#include "hal/bus/spi/spidrv.hpp"

#include "drivers/Encoder/MagEncoder.hpp"
#include "core/io/regs.hpp"

namespace ymd::drivers{

class MT6816:public MagEncoderIntf{
protected:
    hal::SpiDrv spi_drv_;

    real_t lap_position;
    size_t errcnt = 0;
    bool fast_mode = true;

    struct Semantic:public Reg16<>{
        using Reg16::operator=;
        uint16_t pc:1;
        uint16_t no_mag:1;
        uint16_t data_14bit:14;
    };

    Semantic last_semantic;

    uint16_t getPositionData();
public:
    MT6816(const hal::SpiDrv & spi_drv):spi_drv_(spi_drv){;}
    MT6816(hal::SpiDrv && spi_drv):spi_drv_(spi_drv){;}
    MT6816(hal::Spi & _bus, const uint8_t index):spi_drv_(hal::SpiDrv{_bus, index}){;}

    void init() override;


    void update() override;


    real_t getLapPosition() override{return lap_position;}

    uint32_t getErrCnt() const {return errcnt;}

    bool stable() override {return last_semantic.no_mag == false;}
};

};