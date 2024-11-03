#pragma once

#include "hal/bus/spi/spidrv.hpp"

#include "drivers/Encoder/MagEncoder.hpp"
#include "drivers/device_defs.h"

namespace yumud::drivers{

class MT6816:public MagEncoder{
protected:
    SpiDrv bus_drv;

    real_t lap_position;
    size_t errcnt = 0;
    bool fast_mode = true;

    struct Semantic:public Reg16{
        using Reg16::operator=;
        uint16_t pc:1;
        uint16_t no_mag:1;
        uint16_t data_14bit:14;
    };

    Semantic last_semantic;

    uint16_t getPositionData();
public:
    MT6816(const SpiDrv & _bus_drv):bus_drv(_bus_drv){;}
    MT6816(SpiDrv && _bus_drv):bus_drv(_bus_drv){;}
    MT6816(Spi & _bus, const uint8_t index):bus_drv(SpiDrv{_bus, index}){;}

    void init() override;


    void update() override;


    real_t getLapPosition() override{return lap_position;}

    uint32_t getErrCnt() const {return errcnt;}

    bool stable() override {return last_semantic.no_mag == false;}
};

};