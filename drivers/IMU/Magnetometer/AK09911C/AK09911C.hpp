#pragma once

#include "core/io/regs.hpp"
#include "drivers/IMU/IMU.hpp"
#include "drivers/IMU/details/AsahiKaseiIMU.hpp"


namespace ymd::drivers{


struct AK09911C_Collections{
    static constexpr  auto DEFAULT_I2C_ADDR = hal::I2cSlaveAddr<7>::from_u8(0x68);

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

        PowerDown = 0b00000,
        SingleMeasurement = 0b00001,
        Cont1 = 0b00010,
        Cont2 = 0b00100,
        Cont3 = 0b00110,
        Cont4 = 0b01000,
        SelfTest = 0b10000,
        FuseRomAccess = 0b11111,
    };

};

struct AK09911C_Regs : public AK09911C_Collections{
    struct R8_WIA1:public Reg8<>{
        scexpr RegAddress address = 0x00;
        scexpr uint8_t KEY = 0x48;

        uint8_t data;
    } DEF_R8(wia1_reg)

    struct R8_WIA2:public Reg8<>{
        scexpr RegAddress address = 0x01;
        scexpr uint8_t KEY = 0x05;

        uint8_t data;
    } DEF_R8(wia2_reg)

    struct R8_INFO1:public Reg8<>{
        scexpr RegAddress address = 0x02;

        uint8_t data;
    } DEF_R8(info1_reg)

    struct R8_INFO2:public Reg8<>{
        scexpr RegAddress address = 0x03;

        uint8_t data;
    } DEF_R8(info2_reg)

    struct R8_ST1:public Reg8<>{
        scexpr RegAddress address = 0x10;

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
        scexpr RegAddress address = 0x18;

        uint8_t :3;
        uint8_t hofl:1;
        uint8_t :4;

    } DEF_R8(st2_reg)

    //cntl1 reg is a dummy reg, it is not listed here

    struct R8_CNTL2:public Reg8<>{
        scexpr RegAddress address = 0x31;

        Mode mode:5;
        uint8_t :3;
    } DEF_R8(cntl2_reg)

    struct R8_CNTL3:public Reg8<>{
        scexpr RegAddress address = 0x32;

        uint8_t srst:1;
        uint8_t :7;
    } DEF_R8(cntl3_reg)

    //test reg is a dummy reg, it is not listed here

    struct _R8_ASA:public Reg8<>{
        uint8_t data;
    };

    struct R8_ASAX:public _R8_ASA{
        scexpr RegAddress address = 0x10;
    } DEF_R8(asax_reg)

    struct R8_ASAY:public _R8_ASA{
        scexpr RegAddress address = 0x11;
    } DEF_R8(asay_reg)

    struct R8_ASAZ:public _R8_ASA{
        scexpr RegAddress address = 0x12;
    } DEF_R8(asaz_reg)

};


class AK09911C final:
    public MagnetometerIntf, 
    public AK09911C_Regs{
public:

    AK09911C(const hal::I2cDrv & i2c_drv):phy_(i2c_drv){;}
    AK09911C(hal::I2cDrv && i2c_drv):phy_(i2c_drv){;}
    AK09911C(hal::I2c & bus):phy_(hal::I2cDrv(bus, DEFAULT_I2C_ADDR)){;}
    AK09911C(const hal::SpiDrv & spi_drv):phy_(spi_drv){;}
    AK09911C(hal::SpiDrv && spi_drv):phy_(std::move(spi_drv)){;}
    AK09911C(hal::Spi & spi, const hal::SpiSlaveIndex index):phy_(hal::SpiDrv(spi, index)){;}

    [[nodiscard]] IResult<> init();
    [[nodiscard]] IResult<> update();
    [[nodiscard]] IResult<> validate();
    [[nodiscard]] IResult<bool> is_busy();
    [[nodiscard]] IResult<bool> is_stable();
    [[nodiscard]] IResult<> set_mode(const Mode mode);
    [[nodiscard]] IResult<> disable_i2c();
    [[nodiscard]] Option<Vector3_t<q24>> read_mag();

    [[nodiscard]] IResult<bool> is_data_ready();
    [[nodiscard]] IResult<bool> is_data_overrun();
    [[nodiscard]] IResult<> enable_hs_i2c(const Enable en);
    [[nodiscard]] IResult<> reset();

private:

    AsahiKaseiSensor_Phy phy_;
    Option<Vector3_t<q24>> scale_ = None; 

    [[nodiscard]] IResult<> update_adj();

    [[nodiscard]] Result<void, Error> write_reg(const uint8_t addr, const uint8_t data){
        return phy_.write_reg(addr, data);
    }

    [[nodiscard]] Result<void, Error> read_reg(const uint8_t addr, uint8_t & data){
        return phy_.read_reg(addr, data);
    }


    [[nodiscard]] Result<void, Error> write_reg(const auto & reg){
        return phy_.write_reg(reg.address, reg.as_val());
    }

    [[nodiscard]] Result<void, Error> read_reg(auto & reg){
        return phy_.read_reg(reg.address, reg.as_ref());
    }

    [[nodiscard]] IResult<Vector3_t<int8_t>> get_coeff();

    [[nodiscard]] static constexpr Vector3_t<q24> 
    transform_coeff_into_scale(const Vector3_t<int8_t> coeff){
        return Vector3_t<q24>(coeff) / 128 + Vector3_t<q24>(1, 1, 1); 
    }
    
};

};