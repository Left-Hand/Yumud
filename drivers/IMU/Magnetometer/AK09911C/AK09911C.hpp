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

    using RegAddress = uint8_t;
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
        // respectively. After magnetic sensor measurement and signal processing is finished, measurement magnetic data is stored to 
        // measurement data registers (HXL to HZH) and all circuits except for the minimum circuit required for counting cycle 
        // length are turned off (PD). When the next measurement timing comes, AK09911 wakes up automatically from PD and 
        // starts measurement again.
        // Continuous measurement mode ends when Power-down mode (MODE[4:0]=“00000”) is set. It repeats measurement until 
        // Power-down mode is set.
        // When Continuous measurement mode 1 (MODE[4:0]=“00010”), 2 (MODE[4:0]=“00100”), 3 (MODE[4:0]=“00110”) or 4 
        // (MODE[4:0]=“01000”) is set again while AK09911 is already in Continuous measurement mode, a new measurement starts. 
        // ST1, ST2 and measurement data registers (HXL to TMPS) will not be initialized by this.

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

struct AK09911C_Regs : public AK09911C_Prelude{
    struct R8_WIA1:public Reg8<>{
        static constexpr RegAddress address = 0x00;
        static constexpr uint8_t KEY = 0x48;

        uint8_t data;
    } DEF_R8(wia1_reg)

    struct R8_WIA2:public Reg8<>{
        static constexpr RegAddress address = 0x01;
        static constexpr uint8_t KEY = 0x05;

        uint8_t data;
    } DEF_R8(wia2_reg)

    struct R8_INFO1:public Reg8<>{
        static constexpr RegAddress address = 0x02;

        uint8_t data;
    } DEF_R8(info1_reg)

    struct R8_INFO2:public Reg8<>{
        static constexpr RegAddress address = 0x03;

        uint8_t data;
    } DEF_R8(info2_reg)

    struct R8_ST1:public Reg8<>{
        static constexpr RegAddress address = 0x10;

        uint8_t drdy:1;
        uint8_t dor:1;
        uint8_t :5;
        uint8_t hsm:1;

        bool is_data_ready() const {return drdy;}
        bool is_data_overrun() const {return dor;}
    } DEF_R8(st1_reg)

    REG16I_QUICK_DEF(0x11, MagXReg, mag_x_reg);
    REG16I_QUICK_DEF(0x13, MagYReg, mag_y_reg);
    REG16I_QUICK_DEF(0x15, MagZReg, mag_z_reg);

    struct R8_ST2:public Reg8<>{
        static constexpr RegAddress address = 0x18;

        uint8_t :3;
        uint8_t hofl:1;
        uint8_t :4;

    } DEF_R8(st2_reg)

    //cntl1 reg is a dummy reg, it is not listed here

    struct R8_CNTL2:public Reg8<>{
        static constexpr RegAddress address = 0x31;

        Mode mode:5;
        uint8_t :3;
    } DEF_R8(cntl2_reg)

    struct R8_CNTL3:public Reg8<>{
        static constexpr RegAddress address = 0x32;

        uint8_t srst:1;
        uint8_t :7;
    } DEF_R8(cntl3_reg)

    //test reg is a dummy reg, it is not listed here

    struct _R8_ASA:public Reg8<>{
        uint8_t data;
    };

    struct R8_ASAX:public _R8_ASA{
        static constexpr RegAddress address = 0x10;
    } DEF_R8(asax_reg)

    struct R8_ASAY:public _R8_ASA{
        static constexpr RegAddress address = 0x11;
    } DEF_R8(asay_reg)

    struct R8_ASAZ:public _R8_ASA{
        static constexpr RegAddress address = 0x12;
    } DEF_R8(asaz_reg)

};


class AK09911C final:
    public MagnetometerIntf, 
    private AK09911C_Regs{
public:

    AK09911C(const hal::I2cDrv & i2c_drv):phy_(i2c_drv){;}
    AK09911C(hal::I2cDrv && i2c_drv):phy_(i2c_drv){;}
    AK09911C(Some<hal::I2c *> i2c):phy_(hal::I2cDrv(i2c, DEFAULT_I2C_ADDR)){;}
    AK09911C(const hal::SpiDrv & spi_drv):phy_(spi_drv){;}
    AK09911C(hal::SpiDrv && spi_drv):phy_(std::move(spi_drv)){;}
    AK09911C(Some<hal::Spi *> spi, const hal::SpiSlaveRank index):phy_(hal::SpiDrv(spi, index)){;}

    [[nodiscard]] IResult<> init();
    [[nodiscard]] IResult<> update();
    [[nodiscard]] IResult<> validate();
    [[nodiscard]] IResult<bool> is_busy();
    [[nodiscard]] IResult<bool> is_stable();
    [[nodiscard]] IResult<> set_mode(const Mode mode);
    [[nodiscard]] IResult<> disable_i2c();
    [[nodiscard]] IResult<Vec3<q24>> read_mag();

    [[nodiscard]] IResult<bool> is_data_ready();
    [[nodiscard]] IResult<bool> is_data_overrun();
    [[nodiscard]] IResult<> enable_hs_i2c(const Enable en);
    [[nodiscard]] IResult<> reset();
    [[nodiscard]] IResult<> set_odr(const Odr odr);

    using AK09911C_Regs::Mode;
private:
    
    AsahiKaseiSensor_Phy phy_;
    Option<Vec3<q24>> scale_ = None; 
    
    [[nodiscard]] IResult<> selftest();
    [[nodiscard]] IResult<> blocking_update();
    [[nodiscard]] IResult<> update_adj();

    [[nodiscard]] IResult<> write_reg(const uint8_t addr, const uint8_t data){
        return phy_.write_reg(addr, data);
    }

    [[nodiscard]] IResult<> read_reg(const uint8_t addr, uint8_t & data){
        return phy_.read_reg(addr, data);
    }

    [[nodiscard]] IResult<> write_reg(const auto & reg){
        return phy_.write_reg(reg.address, reg.as_val());
    }

    [[nodiscard]] IResult<> read_reg(auto & reg){
        return phy_.read_reg(reg.address, reg.as_ref());
    }

    
    [[nodiscard]] IResult<> read_burst(const RegAddress addr, std::span<int16_t> pbuf){
        return phy_.read_burst(addr, pbuf.data(), pbuf.size());
    }

    [[nodiscard]] IResult<Vec3<int8_t>> get_coeff();

    [[nodiscard]] static constexpr Vec3<q24> 
    transform_coeff_into_scale(const Vec3<int8_t> coeff){
        return Vec3<q24>(coeff) / 128 + Vec3<q24>(1, 1, 1); 
    }
    
};

};