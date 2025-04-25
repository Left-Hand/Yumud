#pragma once

#include <optional>
#include <array>

#include "concept/analog_channel.hpp"
#include "hal/bus/spi/spidrv.hpp"

#include "../CoilDriver.hpp"
#include "core/utils/Result.hpp"



namespace ymd::drivers{

class DRV832X:public Coil3DriverIntf{
public:
    enum class Error:uint8_t{
        Unspecified = 0xff
    };

    enum class PeakCurrent:uint8_t{
        _1_7A = 0,
        _0_7A = 1,
        _0_25A = 2,
    };

    enum class Gain:uint8_t{
        X10 = 0,
        X20 = 1,
        X40 = 2,
        X80 = 3
    };

    enum class IDriveN:uint8_t{
        // 0000b = 20 mA
        // 0001b = 60 mA
        // 0010b = 120 mA
        // 0011b = 160 mA
        // 0100b = 240 mA
        // 0101b = 280 mA
        // 0110b = 340 mA
        // 0111b = 380 mA
        // 1000b = 520 mA
        // 1001b = 660 mA
        // 1010b = 740 mA
        // 1011b = 880 mA
        // 1100b = 1140 mA
        // 1101b = 1360 mA
        // 1110b = 1640 mA
        // 1111b = 2000 mA

        _20mA = 0,
        _60mA = 1,
        _120mA = 2,
        _160mA = 3,
        _240mA = 4,
        _280mA = 5,
        _340mA = 6,
        _380mA = 7,
        _520mA = 8,
        _660mA = 9,
        _740mA= 10,
        _880mA = 11,
        _1140mA = 12,
        _1360mA = 13,
        _1640mA = 14,
        _2000mA = 15
    };

    enum class IDriveP:uint8_t{
        // 0000b = 10 mA
        // 0001b = 30 mA
        // 0010b = 60 mA
        // 0011b = 80 mA
        // 0100b = 120 mA
        // 0101b = 140 mA
        // 0110b = 170 mA
        // 0111b = 190 mA
        // 1000b = 260 mA
        // 1001b = 330 mA
        // 1010b = 370 mA
        // 1011b = 440 mA
        // 1100b = 570 mA
        // 1101b = 680 mA
        // 1110b = 820 mA
        // 1111b = 1000 mA

        _10mA = 0,
        _30mA = 1,
        _60mA = 2,
        _80mA = 3,
        _120mA = 4,
        _140mA = 5,
        _170mA = 6,
        _190mA = 7,
        _260mA = 8,
        _330mA = 9,
        _370mA= 10,
        _440mA = 11,
        _570mA = 12,
        _680mA = 13,
        _820mA = 14,
        _1000mA = 15
    };

    enum class VdsLevel{
        // 0000b = 0.06 V
        // 0001b = 0.13 V
        // 0010b = 0.2 V
        // 0011b = 0.26 V
        // 0100b = 0.31 V
        // 0101b = 0.45 V
        // 0110b = 0.53 V
        // 0111b = 0.6 V
        // 1000b = 0.68 V
        // 1001b = 0.75 V
        // 1010b = 0.94 V
        // 1011b = 1.13 V
        // 1100b = 1.3 V
        // 1101b = 1.5 V
        // 1110b = 1.7 V
        // 1111b = 1.88 V

        _0_06V = 0,
        _0_13V = 1,
        _0_2V = 2,
        _0_26V = 3,
        _0_31V = 4,
        _0_45V = 5,
        _0_53V = 6,
        _0_6V = 7,
        _0_68V = 8,
        _0_75V = 9,
        _0_94V = 10,
        _1_13V = 11,
        _1_30V = 12,
        _1_50V = 13,
        _1_70V = 14,
        _1_88V = 15
    };


    enum class OcpMode{
        CurrentLimit = 0,
        OClatchShutdown = 1,
        ReportOnly = 2,
        OCdisabled = 3
    };

    enum class OcpDeglitchTime:uint8_t{
        _2us = 0,
        _4us,
        _6us,
        _8us,
    };

    enum class PeakDriveTime:uint8_t{
        _500ns = 0,
        _1us,
        _2us,
        _4us,
    };

    enum class DeadTime:uint8_t{
        // 00b = 50-ns dead time
        // 01b = 100-ns dead time
        // 10b = 200-ns dead time
        // 11b = 400-ns dead time

        _50_ns = 0,
        _100_ns = 1,
        _200_ns = 2,
        _400_ns = 3,
    };

    enum class CsaGain:uint8_t{
        _5x = 0b00,
        _10x = 0b01,
        _20x = 0b10,
        _40x = 0b11,
    };

    enum class SenseLevel:uint8_t{
        // 00b = 0.25 V
        // 01b = 0.5 V
        // 10b = 0.75 V
        // 11b = 1.0 V

        _0_25V = 0,
        _0_5V = 1,
        _0_75V = 2,
        _1_0V = 3,
    };
protected:
    hal::SpiDrv spi_drv_;

    using RegAddress = uint8_t;

    struct Status1Reg:public Reg16<>{
        scexpr RegAddress address = 0x00;

        uint16_t vds_lc:1;
        uint16_t vds_hc:1;
        uint16_t vds_lb:1;
        uint16_t vds_hb:1;
        uint16_t vds_la:1;
        uint16_t vds_ha:1;
        uint16_t otsd:1;
        uint16_t uvlo:1;
        uint16_t gdf:1;
        uint16_t vds_ocp:1;
        uint16_t fault:1;

        uint16_t :6;
    };

    struct Status2Reg:public Reg16<>{
        scexpr RegAddress address = 0x01;

        uint16_t vgs_lc:1;
        uint16_t vgs_hc:1;
        uint16_t vgs_lb:1;
        uint16_t vgs_hb:1;
        uint16_t vgs_la:1;
        uint16_t vgs_ha:1;
        uint16_t cpuv:1;
        uint16_t otw:1;
        uint16_t sc_oc:1;
        uint16_t sb_oc:1;
        uint16_t sa_oc:1;

        uint16_t :6;
    };

    struct Ctrl1Reg:public Reg16<>{
        scexpr RegAddress address = 0x02;

        uint16_t clr_flt:1;
        uint16_t brake:1;
        uint16_t coast:1;
        uint16_t pwm1_dir:1;
        uint16_t pwm1_com:1;
        uint16_t pwm_mode:2;
        uint16_t otw_rep:2;
        uint16_t dis_gdf:2;
        uint16_t dis_cpuv:2;

        uint16_t :7;
    };

    struct R16_GateDriveHs:public Reg16<>{
        scexpr RegAddress address = 0x03;

        uint16_t idrive_n_hs:4;
        uint16_t idrive_p_hs:4;
        uint16_t lock:3;

        uint16_t :5;
    };

    struct R16_GateDriveLs:public Reg16<>{
        scexpr RegAddress address = 0x04;

        uint16_t idrive_n_ls:4;
        uint16_t idrive_p_ls:4;
        uint16_t tdrive:2;
        uint16_t cbc:1;

        uint16_t :5;
    };

    struct R16_OcpCtrl{
        scexpr RegAddress address = 0x05;

        uint16_t vds_lvl:4;
        uint16_t ocp_deg:2;
        uint16_t ocp_mode:2;
        uint16_t dead_time:2;
        uint16_t tretry:1;

        uint16_t :5;
    };

    struct R16_CsaCtrl{
        scexpr RegAddress address = 0x06;

        uint16_t sen_lvl:2;
        uint16_t csa_cal_c:1;
        uint16_t csa_cal_b:1;
        uint16_t csa_cal_a:1;
        uint16_t dis_sen:1;
        uint16_t csa_gain:2;
        uint16_t ls_ref:1;
        uint16_t vref_div:1;
        uint16_t csa_fet:1;
    };

    struct Regs{
        Status1Reg status1;
        Status2Reg status2;
        Ctrl1Reg ctrl1;
        R16_GateDriveHs gate_drv_hs;
        R16_GateDriveLs gate_drv_ls;
        R16_OcpCtrl ocp_ctrl;
        R16_CsaCtrl csa_ctrl;
    }regs_;

    hal::BusError write_reg(const RegAddress addr, const uint16_t reg);
    hal::BusError read_reg(const RegAddress addr, uint16_t & reg);
    

    hal::BusError write_reg(const auto & reg){
        return write_reg(reg.address, reg);
    }

    hal::BusError read_reg(auto & reg){
        return read_reg(reg.address, reg);
    }



public:
    DRV832X(const hal::SpiDrv & spi_drv):spi_drv_(spi_drv){;}
    DRV832X(hal::SpiDrv && spi_drv):spi_drv_(std::move(spi_drv)){;}
    DRV832X(hal::Spi & spi, const hal::SpiSlaveIndex index):spi_drv_(hal::SpiDrv(spi, index)){;}


    void init();

    Result<void, Error> set_peak_current(const PeakCurrent peak_current);
    Result<void, Error> set_ocp_mode(const OcpMode ocp_mode);
    // Result<void, Error> set_octw_mode(const OctwMode octw_mode);
    Result<void, Error> set_gain(const Gain gain);
    // Result<void, Error> set_oc_ad_table(const OcAdTable oc_ad_table);
    Result<void, Error> enable_pwm3(const bool en = true);

    Result<void, Error> set_drive_hs(const IDriveP pdrive, const IDriveN ndrive);
    Result<void, Error> set_drive_ls(const IDriveP pdrive, const IDriveN ndrive);
    Result<void, Error> set_drive_time(const PeakDriveTime ptime);

};

};

namespace ymd::custom{
    template<>
    struct result_converter<void, drivers::DRV832X::Error, hal::BusError> {
        static Result<void, drivers::DRV832X::Error> convert(const hal::BusError & res){
            if(res.ok()) return Ok();
            else return Err(drivers::DRV832X::Error::Unspecified); 
        }
    };
}