#pragma once

#include <optional>
#include <array>

#include "hal/bus/spi/spidrv.hpp"
#include "hal/gpio/gpio.hpp"

#include "core/utils/Result.hpp"
#include "core/utils/errno.hpp"

#include "concept/analog_channel.hpp"



namespace ymd::drivers{

struct DRV832X_Prelude{
    enum class Error_Kind{

    };

    DEF_ERROR_SUMWITH_HALERROR(Error, Error_Kind)

    template<typename T = void>
    using IResult = Result<T, Error>;

    using RegAddr = uint8_t;

    enum class PeakCurrent:uint16_t{
        _1_7A = 0,
        _0_7A = 1,
        _0_25A = 2,
    };

    enum class Gain:uint16_t{
        X10 = 0,
        X20 = 1,
        X40 = 2,
        X80 = 3
    };

    enum class IDriveN:uint16_t{
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

    enum class IDriveP:uint16_t{
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

    enum class VdsLevel:uint16_t{
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

    enum class OcpMode:uint16_t{
        CurrentLimit = 0,
        OClatchShutdown = 1,
        ReportOnly = 2,
        OCdisabled = 3
    };

    enum class OcpDeglitchTime:uint16_t{
        _2us = 0,
        _4us,
        _6us,
        _8us,
    };

    enum class PeakDriveTime:uint16_t{
        _500ns = 0,
        _1us,
        _2us,
        _4us,
    };

    enum class DeadTime:uint16_t{
        // 00b = 50-ns dead time
        // 01b = 100-ns dead time
        // 10b = 200-ns dead time
        // 11b = 400-ns dead time

        _50_ns = 0,
        _100_ns = 1,
        _200_ns = 2,
        _400_ns = 3,
    };

    enum class CsaGain:uint16_t{
        _5x = 0b00,
        _10x = 0b01,
        _20x = 0b10,
        _40x = 0b11,
    };

    enum class SenseLevel:uint16_t{
        // 00b = 0.25 V
        // 01b = 0.5 V
        // 10b = 0.75 V
        // 11b = 1.0 V

        _0_25V = 0,
        _0_5V = 1,
        _0_75V = 2,
        _1_0V = 3,
    };

    enum class PwmMode:uint16_t{
        _6x = 0b00,
        _3x = 0b01,
        _1x = 0b10,
        Independent = 0b11,
    };


    struct Config{

    };
};

struct DRV832X_Regs:public DRV832X_Prelude{
    struct R16_Status1{
        static constexpr RegAddr ADDRESS = 0x00;

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

        uint16_t :5;

        uint16_t & as_ref(){
            return *reinterpret_cast<uint16_t*>(this);
        }

        std::bitset<11> as_bitset() const {
            return std::bitset<11>(*reinterpret_cast<const uint16_t*>(this));
        }
    }DEF_R16(status1_reg)

    struct R16_Status2{
        static constexpr RegAddr ADDRESS = 0x01;

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

        uint16_t :5;

        uint16_t & as_ref(){
            return *reinterpret_cast<uint16_t*>(this);
        }

        std::bitset<11> as_bitset() const {
            return std::bitset<11>(*reinterpret_cast<const uint16_t*>(this));
        }
    }DEF_R16(status2_reg)

    struct R16_Ctrl1:public Reg16<>{
        static constexpr RegAddr ADDRESS = 0x02;

        uint16_t clr_flt:1;
        uint16_t brake:1;
        uint16_t coast:1;
        uint16_t pwm1_dir:1;
        uint16_t pwm1_com:1;
        PwmMode pwm_mode:2;
        uint16_t otw_rep:1;
        uint16_t dis_gdf:1;
        uint16_t dis_cpuv:1;

        uint16_t :6;
    }DEF_R16(ctrl1_reg)

    struct R16_GateDriveHs:public Reg16<>{
        static constexpr RegAddr ADDRESS = 0x03;

        IDriveN idrive_n_hs:4;
        IDriveP idrive_p_hs:4;
        uint16_t lock:3;

        uint16_t :5;
    }DEF_R16(gate_drv_hs_reg)

    struct R16_GateDriveLs:public Reg16<>{
        static constexpr RegAddr ADDRESS = 0x04;

        IDriveN idrive_n_ls:4;
        IDriveP idrive_p_ls:4;
        PeakDriveTime tdrive:2;
        uint16_t cbc:1;

        uint16_t :5;
    }DEF_R16(gate_drv_ls_reg)

    struct R16_OcpCtrl:public Reg16<>{
        static constexpr RegAddr ADDRESS = 0x05;

        VdsLevel vds_lvl:4;
        OcpDeglitchTime ocp_deg:2;
        OcpMode ocp_mode:2;
        DeadTime dead_time:2;
        uint16_t tretry:1;

        uint16_t :5;
    }DEF_R16(ocp_ctrl_reg)

    struct R16_CsaCtrl{
        static constexpr RegAddr ADDRESS = 0x06;

        SenseLevel sen_lvl:2;
        uint16_t csa_cal_c:1;
        uint16_t csa_cal_b:1;
        uint16_t csa_cal_a:1;
        uint16_t dis_sen:1;
        uint16_t csa_gain:2;
        uint16_t ls_ref:1;
        uint16_t vref_div:1;
        uint16_t csa_fet:1;

        uint16_t :5;
    }DEF_R16(csa_ctrl_reg)

};


class DRV8323R_Phy final:public DRV832X_Prelude{
public:
    DRV8323R_Phy(const hal::SpiDrv & spi_drv):
        spi_drv_(spi_drv){;}
    DRV8323R_Phy(hal::SpiDrv && spi_drv):
        spi_drv_(std::move(spi_drv)){;}
    DRV8323R_Phy(Some<hal::Spi *> spi, const hal::SpiSlaveRank index):
        spi_drv_(hal::SpiDrv(spi, index)){;}

    [[nodiscard]] IResult<> write_reg(const RegAddr addr, const uint16_t reg);
    [[nodiscard]] IResult<> read_reg(const RegAddr addr, uint16_t & reg);
private:
    hal::SpiDrv spi_drv_;
};

class DRV8323H_Phy final:public DRV832X_Prelude{
public:
    struct Params{
        hal::Gpio & gain_gpio;
        hal::Gpio & vds_gpio;
        hal::Gpio & idrive_gpio;
        hal::Gpio & mode_gpio;
    };

    DRV8323H_Phy(const Params & params):
        gain_gpio_(params.gain_gpio),
        vds_gpio_(params.vds_gpio),
        idrive_gpio_(params.idrive_gpio),
        mode_gpio_(params.mode_gpio){;}

    void set_pwm_mode(const PwmMode mode){
        // _6x = GND,
        // _3x = 47K to GND,
        // _1x = HiZ,
        // Independent = VDD,

        switch(mode){
            case PwmMode::_6x:
                mode_gpio_.outpp(LOW);
                break;
            case PwmMode::_3x:
                mode_gpio_.inpd();
                break;
            case PwmMode::_1x:
                mode_gpio_.inflt();
                break;
                // mode_gpio_.outpp(HIGH);
                break;
            case  PwmMode::Independent:
                mode_gpio_.outpp(HIGH);
                break;
        }
    }

    void set_idrive(const IDriveP drive){
        // switch(drive){
        //     case IDriveP::
        // }
        idrive_gpio_.inflt();
    }

private:
    hal::Gpio & gain_gpio_;
    hal::Gpio & vds_gpio_;
    hal::Gpio & idrive_gpio_;
    hal::Gpio & mode_gpio_;
};


};
