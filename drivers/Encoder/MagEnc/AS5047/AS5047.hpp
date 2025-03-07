#pragma once

#include "hal/bus/spi/spidrv.hpp"

#include "drivers/Encoder/MagEncoder.hpp"
#include "drivers/device_defs.h"

namespace ymd::drivers{

class AS5047:public MagEncoderIntf{
protected:

    using RegAddress = uint16_t;

    struct ErrflReg:public Reg8<>{
        scexpr RegAddress address = 0x001;
        uint8_t frame_error:1;
        uint8_t invalid_cmd_error:1;
        uint8_t parity_error:1;
        uint8_t :5;
    };

    struct ProgReg:public Reg8<>{
        scexpr RegAddress address = 0x002;

        uint8_t prog_otp_en:1;
        uint8_t otp_reflash:1;
        uint8_t start_otp_prog:1;
        uint8_t prog_verify:1;
        uint8_t :4;
    };

    // struct DiaagcReg:public Reg8<>{
    //     scexpr RegAddress address = 0x3ffc;

    //     uint8_t 
    // };


    hal::SpiDrv spi_drv_;

    real_t lap_position;
    size_t errcnt = 0;
    bool fast_mode = true;


    uint16_t getPositionData();


    void writeReg(const RegAddress addr, const uint8_t data);
    void readReg(const RegAddress addr, uint8_t & data);



public:
    AS5047(const hal::SpiDrv & spi_drv):spi_drv_(spi_drv){;}
    AS5047(hal::SpiDrv && spi_drv):spi_drv_(spi_drv){;}
    AS5047(hal::Spi & spi, const uint8_t index):spi_drv_(hal::SpiDrv{spi, index}){;}

    void init() override;

    void update() override;
    real_t getLapPosition() override{return lap_position;}
    uint32_t getErrCnt() const {return errcnt;}

};

};