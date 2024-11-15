#pragma once

#include "hal/bus/spi/spidrv.hpp"

#include "drivers/Encoder/MagEncoder.hpp"
#include "drivers/device_defs.h"

namespace ymd::drivers{

class MT6835:public MagEncoder{
protected:

    using RegAddress = uint8_t;

    struct UserIdReg:public Reg8{
        scexpr RegAddress address = 0x001;
        uint8_t id:8;
    };

    struct AngleHReg:public Reg8{
        scexpr RegAddress address = 0x003;
        uint8_t angle:8;
    };

    struct AngleMReg:public Reg8{
        scexpr RegAddress address = 0x004;
        uint8_t angle:8;
    };

    struct AngleLReg:public Reg8{
        scexpr RegAddress address = 0x005;
        uint8_t over_speed:1;
        uint8_t mag_weak:1;
        uint8_t under_voltage:1;
        uint8_t angle:5;
    };

    struct CrcReg:public Reg8{
        scexpr RegAddress address = 0x006;
        uint8_t crc:8;
    };

    struct AbzResHReg:public Reg8{
        scexpr RegAddress address = 0x007;
        uint8_t resolution:8;
    };

    struct AbzResLReg:public Reg8{
        scexpr RegAddress address = 0x008;
        uint8_t ab_swap:1;
        uint8_t abz_off:1;
        uint8_t resolution:6;
    };

    struct ZeroPosHReg:public Reg8{
        scexpr RegAddress address = 0x009;
        uint8_t zero_pos:8;
    };

    struct ZeroPosLReg:public Reg8{
        scexpr RegAddress address = 0x00A;
        uint8_t z_pul_wid:3;
        uint8_t z_falling_on_0edge:1;
        uint8_t zero_pos:4;
    };

    enum class UVWPoles:uint8_t{
        _1 = 0,
        _2, _3, _4, _5, _6, _7, _8, _9, 
        _10, _11, _12, _13, _14, _15, _16
    };

    struct UvwReg:public Reg8{
        scexpr RegAddress address = 0x00B;
        uint8_t uvw_res:4;
        uint8_t uvw_off:1;
        uint8_t uvw_mux:1;
        uint8_t z_phase:2;
    };


    struct PwmReg:public Reg8{
        scexpr RegAddress address = 0x00C;
        uint8_t pwm_sel:3;
        uint8_t pwm_pol:1;
        uint8_t pwm_fq:1;
        uint8_t nlc_en:1;
    };

    struct RoatationReg:public Reg8{
        scexpr RegAddress address = 0x00D;
        uint8_t hyst:3;
        uint8_t rot_dir:1;
    };

    struct CaliReg:public Reg8{
        scexpr RegAddress address = 0x00E;

        uint8_t :4;
        uint8_t autocal_freq:3;
        uint8_t gpio_ds:1;
    };

    struct BandWidthReg:public Reg8{
        scexpr RegAddress address = 0x011;
        uint8_t bw:3;
        uint8_t :5;
    };

    struct NlcReg:public Reg8{
        scexpr RegAddress address = 0x013;
        scexpr RegAddress address_end = 0x0D2;

        uint8_t :8;
    };


    SpiDrv spi_drv_;

    real_t lap_position;
    size_t errcnt = 0;
    bool fast_mode = true;


    uint16_t getPositionData();


    void writeReg(const RegAddress addr, const uint8_t data);
    void readReg(const RegAddress addr, uint8_t & data);
public:
    MT6835(const SpiDrv & spi_drv):spi_drv_(spi_drv){;}
    MT6835(SpiDrv && spi_drv):spi_drv_(spi_drv){;}
    MT6835(Spi & spi, const uint8_t index):spi_drv_(SpiDrv{spi, index}){;}

    void init() override;

    void update() override;
    real_t getLapPosition() override{return lap_position;}
    uint32_t getErrCnt() const {return errcnt;}

};

};