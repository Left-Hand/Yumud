#pragma once

#include <optional>

#include "core/io/regs.hpp"

#include "hal/bus/i2c/i2cdrv.hpp"
#include "hal/bus/spi/spidrv.hpp"


struct RS485Drv{

};


namespace ymd::drivers{


class MP6570_Phy{
public: 
    using SlaveAddress = uint8_t;

    MP6570_Phy(const hal::I2cDrv & i2c_drv) : i2c_drv_(i2c_drv) {}
    MP6570_Phy(hal::I2cDrv && i2c_drv) : i2c_drv_(std::move(i2c_drv)) {}

    MP6570_Phy(const SlaveAddress spi_slave_addr, const hal::SpiDrv & spi_drv)
            : spi_slave_addr_(spi_slave_addr), spi_drv_(spi_drv) {}
    MP6570_Phy(const SlaveAddress spi_slave_addr, hal::SpiDrv && spi_drv)
            : spi_slave_addr_(spi_slave_addr), spi_drv_(std::move(spi_drv)) {}

    
    BusError write_reg(const uint8_t reg_addr, const uint16_t data);
    
    BusError read_reg(const uint8_t reg_addr, uint16_t & data);
private:
    std::optional<hal::I2cDrv> i2c_drv_;
    SlaveAddress spi_slave_addr_ = 0;
    std::optional<hal::SpiDrv> spi_drv_;
};




class MP6570 {
public:



private:
    using Phy = MP6570_Phy;
    Phy phy_;

public:
    using RegAddress = uint8_t;

    struct R16_ThetaCmdL:public Reg16<>{
        scexpr RegAddress address = 0x01;

        uint16_t :16;
    };

    struct R16_ThetaCmdM:public Reg16<>{
        scexpr RegAddress address = 0x02;

        uint16_t :16;
    };

    struct R16_ThetaCmH:public Reg16<>{
        scexpr RegAddress address = 0x03;

        uint16_t :16;
    };

    struct R16_CycleReg:public Reg16<>{
        scexpr RegAddress address = 0x04;

        uint8_t pole_pair;
        uint8_t cycle_p;
    };

    struct R16_Ctrl1:public Reg16<>{
        scexpr RegAddress address = 0x05;
        uint16_t nstep:9;
        uint16_t fgsel:5;
        uint16_t theta_cmd_type:1;
        uint16_t theta_dir:1;
    };

    struct R16_Gain1:public Reg16<>{
        scexpr RegAddress address = 0x06;

        uint16_t iq_limit:11;
        uint16_t gain_coe:4;
        uint16_t :1;
    };

    struct R16_ThetaBias:public Reg16<>{
        scexpr RegAddress address = 0x07;

        uint16_t theta_bias:11;
        uint16_t :5;
    };

    struct R32_ThetaKi:public Reg32<>{
        scexpr RegAddress address = 0x08;

        using Reg32::operator =;
        uint32_t :32;
    };

    struct R16_ErrLimitH:public Reg16<>{
        scexpr RegAddress address = 0x0a;

        using Reg16::operator =;
        uint16_t :16;
    };

    struct R16_ZP1A:public Reg16<>{
        scexpr RegAddress address = 0x0b;

        using Reg16::operator =;
        uint16_t :16;
    };

    struct R16_ZP1B:public Reg16<>{
        scexpr RegAddress address = 0x0c;

        using Reg16::operator =;
        uint16_t :16;
    };

    struct R16_ZP2A:public Reg16<>{
        scexpr RegAddress address = 0x0d;

        using Reg16::operator =;
        uint16_t :16;
    };

    struct R16_ZP2B:public Reg16<>{
        scexpr RegAddress address = 0x0e;

        using Reg16::operator =;
        uint16_t :16;
    };

    struct R16_Gain2:public Reg16<>{
        scexpr RegAddress address = 0x0f;

        uint16_t set_gain2:4;
        uint16_t set_gain1:5;
        uint16_t :7;
    };

    struct R16_IdRef:public Reg16<>{
        scexpr RegAddress address = 0x10;

        using Reg16::operator =;
        uint16_t :16;        
    };

    struct R16_IqRef:public Reg16<>{
        scexpr RegAddress address = 0x11;

        using Reg16::operator =;
        uint16_t :16;        
    };

    
    struct R16_CurrentKi:public Reg16<>{
        scexpr RegAddress address = 0x12;

        using Reg16::operator =;
        uint16_t :16;        
    };

    struct R16_CurrentKp:public Reg16<>{
        scexpr RegAddress address = 0x13;

        using Reg16::operator =;
        uint16_t :16;        
    };

    struct R16_ThetaDir:public Reg16<>{
        scexpr RegAddress address = 0x05;

        uint16_t :15;
        uint16_t theta_dir:1;
    };


    struct R16_SpiSdoMod:public Reg16<>{
        scexpr RegAddress address = 0x33;

        uint16_t spi_o_mod:1;
        uint16_t :15;
    };


};

}