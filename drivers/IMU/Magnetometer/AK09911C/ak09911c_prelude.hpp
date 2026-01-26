#pragma once

//这款磁传感器不建议使用 原因如下：
// 1.I2c通信频率低
// 2.经常发生读写故障 需要多次重试才能正常通信
// 3.数据抖动大 直接转为姿态能有10度左右的抖动
// 4.很容易受环境磁场干扰

//注意
//市售的AK09911模块没有给RSTN接上拉电阻 导致模块默认处于复位状态

#include "core/io/regs.hpp"
#include "drivers/IMU/IMU.hpp"
#include "drivers/IMU/details/AsahiKaseiIMU.hpp"


namespace ymd::drivers{


struct AK09911C_Prelude{
    // 0b 0 0 0 1 1 0 1 (CAD)
    static constexpr auto DEFAULT_I2C_ADDR = hal::I2cSlaveAddr<7>::from_u7(0b0001101);
    // static constexpr auto DEFAULT_I2C_ADDR = hal::I2cSlaveAddr<7>::from_u7(0b0001100);

    using RegAddr = uint8_t;
    using Error = ImuError;

    template<typename T = void>
    using IResult = Result<T, Error>;

    // enum class Mode:uint8_t{
    //     PowerDown = 0b0000,
    //     SingleMeasurement = 0b0001,
    //     SelfTest = 0b1000,
    //     FuseRomAccess = 0b1111,
    // };

    enum class Mode:uint8_t{
        // “00000”: Power-down mode
        // “00001”: Single measurement mode
        // “00010”: Continuous measurement mode 1
        // “00100”: Continuous measurement mode 2
        // “00110”: Continuous measurement mode 3
        // “01000”: Continuous measurement mode 4
        // “10000”: Self-test mode
        // “11111”: Fuse ROM access mode

        // @page 16 6.4.3

        // When Continuous measurement mode 1 (MODE[4:0]=“00010”), 2 (MODE[4:0]=“00100”), 3 (MODE[4:0]=“00110”) or 4 
        // (MODE[4:0]=“01000”) is set, magnetic sensor measurement is started periodically at 10 Hz, 20 Hz, 50 Hz or 100 Hz 
        // respectively. After magnetic sensor measurement and signal processing is finished, measurement magnetic bits is stored to 
        // measurement bits registers (HXL to HZH) and all circuits except for the minimum circuit required for counting cycle 
        // length are turned off (PD). When the next measurement timing comes, AK09911 wakes up automatically from PD and 
        // starts measurement again.
        // Continuous measurement mode ends when Power-down mode (MODE[4:0]=“00000”) is set. It repeats measurement until 
        // Power-down mode is set.
        // When Continuous measurement mode 1 (MODE[4:0]=“00010”), 2 (MODE[4:0]=“00100”), 3 (MODE[4:0]=“00110”) or 4 
        // (MODE[4:0]=“01000”) is set again while AK09911 is already in Continuous measurement mode, a new measurement starts. 
        // ST1, ST2 and measurement bits registers (HXL to TMPS) will not be initialized by this.

        PowerDown = 0b00000,
        SingleMeasurement = 0b00001,
        Cont1 = 0b00010,
        Cont2 = 0b00100,
        Cont3 = 0b00110,
        Cont4 = 0b01000,
        SelfTest = 0b10000,
        FuseRomAccess = 0b11111,
    };

    enum class Odr:uint8_t{
        Single = 0b00001,
        _10Hz = 0b00010,
        _20Hz = 0b00100,
        _50Hz = 0b00110,
        _100Hz = 0b01000,
    };

};

struct AK09911C_Regset : public AK09911C_Prelude{
    struct R8_WIA1:public Reg8<>{
        static constexpr RegAddr ADDRESS = 0x00;
        static constexpr uint8_t KEY = 0x48;

        uint8_t bits;
    } DEF_R8(wia1_reg)

    struct R8_WIA2:public Reg8<>{
        static constexpr RegAddr ADDRESS = 0x01;
        static constexpr uint8_t KEY = 0x05;

        uint8_t bits;
    } DEF_R8(wia2_reg)

    struct R8_INFO1:public Reg8<>{
        static constexpr RegAddr ADDRESS = 0x02;

        uint8_t bits;
    } DEF_R8(info1_reg)

    struct R8_INFO2:public Reg8<>{
        static constexpr RegAddr ADDRESS = 0x03;

        uint8_t bits;
    } DEF_R8(info2_reg)

    struct R8_ST1:public Reg8<>{
        static constexpr RegAddr ADDRESS = 0x10;

        uint8_t drdy:1;
        uint8_t dor:1;
        uint8_t :5;
        uint8_t hsm:1;

        [[nodiscard]] constexpr bool is_data_ready() const {return drdy;}
        [[nodiscard]] constexpr bool is_data_overrun() const {return dor;}
    } DEF_R8(st1_reg)

    REG16_QUICK_DEF(0x11, MagXReg, mag_x_reg);
    REG16_QUICK_DEF(0x13, MagYReg, mag_y_reg);
    REG16_QUICK_DEF(0x15, MagZReg, mag_z_reg);

    struct R8_ST2:public Reg8<>{
        static constexpr RegAddr ADDRESS = 0x18;

        uint8_t :3;
        uint8_t hofl:1;
        uint8_t :4;

    } DEF_R8(st2_reg)

    //cntl1 reg is a dummy reg, it is not listed here

    struct R8_CNTL2:public Reg8<>{
        static constexpr RegAddr ADDRESS = 0x31;

        Mode mode:5;
        uint8_t :3;
    } DEF_R8(cntl2_reg)

    struct R8_CNTL3:public Reg8<>{
        static constexpr RegAddr ADDRESS = 0x32;

        uint8_t srst:1;
        uint8_t :7;
    } DEF_R8(cntl3_reg)

    //test reg is a dummy reg, it is not listed here

    struct _R8_ASA:public Reg8<>{
        uint8_t bits;
    };

    struct R8_ASAX:public _R8_ASA{
        static constexpr RegAddr ADDRESS = 0x10;
    } DEF_R8(asax_reg)

    struct R8_ASAY:public _R8_ASA{
        static constexpr RegAddr ADDRESS = 0x11;
    } DEF_R8(asay_reg)

    struct R8_ASAZ:public _R8_ASA{
        static constexpr RegAddr ADDRESS = 0x12;
    } DEF_R8(asaz_reg)

};


};