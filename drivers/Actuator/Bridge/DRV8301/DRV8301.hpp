#pragma once

#include "drivers/device_defs.h"
#include "../CoilDriver.hpp"
#include "types/range/range.hpp"
#include "hal/adc/analog_channel.hpp"

#include <optional>
#include <array>


namespace ymd::drivers{

class DRV8301:public Coil3DriverIntf{
public:
    enum class PeakCurrent:uint8_t{
        _1_7A = 0,
        _0_7A = 1,
        _0_25A = 2,
    };

    enum class OcpMode:uint8_t{
        CurrentLimit = 0,
        OClatchShutdown = 1,
        ReportOnly = 2,
        OCdisabled = 3
    };

    enum class OctwMode:uint8_t{
        OTandOC = 0,
        OTonly = 1,
        OConly = 2,
    };

    enum class Gain:uint8_t{
        X10 = 0,
        X20 = 1,
        X40 = 2,
        X80 = 3
    };

    enum class OcAdTable:uint8_t{
        _060mA = 0,
        _068mA,
        _076mA,
        _086mA,
        _097mA,
        _109mA,
        _123mA,
        _138mA,

        _155mA = 8,
        _175mA,
        _197mA,
        _222mA,
        _250mA,
        _282mA,
        _317mA,
        _358mA,

        _403mA = 16,
        _454mA,
        _511mA,
        _576mA,
        _648mA,
        _730mA,
        _822mA,
        _926mA,

        _1043mA = 24,
        _1175mA,
        _1324mA,
        _1491mA,
        _1679mA,
        _1892mA,
        _2131mA,
        _2400mA,
    };

protected:
    hal::SpiDrv spi_drv_;

    using RegAddress = uint8_t;

    struct Status1Reg:public Reg16<>{
        scexpr RegAddress address = 0x00;

        uint16_t fetlc_oc:1;
        uint16_t fethc_oc:1;
        uint16_t fetlb_oc:1;
        uint16_t fettb_oc:1;
        uint16_t fetla_oc:1;
        uint16_t fetha_oc:1;

        uint16_t otw:1;
        uint16_t otsd:1;
        uint16_t pvdd_uv:1;
        uint16_t gvdd_uw:1;
        uint16_t fault:1;

        uint16_t :6;
    };

    struct Status2Reg:public Reg16<>{
        scexpr RegAddress address = 0x01;

        uint16_t device_id:4;
        uint16_t :3;
        uint16_t gvdd_ov:1;
        uint16_t :9;
    };

    struct Ctrl1Reg:public Reg16<>{
        scexpr RegAddress address = 0x02;

        uint16_t gate_current:2;
        uint16_t gate_reset:1;
        uint16_t pwm3_en:1;
        uint16_t ocp_mode:3;
        uint16_t oc_adj_set:5;
        uint16_t :6;
    };

    struct Ctrl2Reg:public Reg16<>{
        scexpr RegAddress address = 0x03;

        uint16_t octw_mode:2;
        uint16_t gain:2;
        uint16_t dc_cal_ch1:1;
        uint16_t dc_cal_ch2:1;

        uint16_t oc_toff:1;
        uint16_t :10;
    };

    Status1Reg status1_reg = {};
    Status2Reg status2_reg = {};
    Ctrl1Reg ctrl1_reg = {};
    Ctrl2Reg ctrl2_reg = {};

    BusError write_reg(const RegAddress addr, const uint16_t reg);
    BusError read_reg(const RegAddress addr, uint16_t & reg);

public:
    DRV8301(const hal::SpiDrv & spi_drv):spi_drv_(spi_drv){;}
    DRV8301(hal::SpiDrv && spi_drv):spi_drv_(std::move(spi_drv)){;}
    DRV8301(hal::Spi & spi, const uint8_t index):spi_drv_(hal::SpiDrv(spi, index)){;}


    void init();

    void setPeakCurrent(const PeakCurrent peak_current);
    void setOcpMode(const OcpMode ocp_mode);
    void setOctwMode(const OctwMode octw_mode);
    void setGain(const Gain gain);
    void setOcAdTable(const OcAdTable oc_ad_table);
    void enablePwm3(const bool en = true);
};

};