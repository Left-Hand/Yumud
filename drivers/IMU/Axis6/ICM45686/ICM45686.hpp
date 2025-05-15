#pragma once

#include "drivers/IMU/details/InvensenseIMU.hpp"


// https://github.com/NOKOLat/STM32_ICM45686/blob/master/ICM45686.h

namespace ymd::drivers{

struct ICM45686_Collections{
    using Error = ImuError;

    template<typename T = void>
    using IResult = Result<T, Error>;

    static constexpr auto DEFAULT_I2C_ADDR = hal::I2cSlaveAddr<7>::from_u8(0x68);
    // enum class RegAddress{
    //     Config0 = 0x16
    // };

    using RegAddress = uint8_t;


    enum class REGISTER: uint8_t{
        ACCEL_DATA_X1_UI = 0x00,
        PWR_MGMT0        = 0x10,
        ACCEL_CONFIG     = 0x1B,
        GYRO_CONFIG 	 = 0x1C,
        WHO_AM_I   		 = 0x72
    };

    enum class Mode: uint8_t{

        Off = 0x00,
        Standby,
        LowPower,
        LowNoise
    };

    enum class AccelScale: uint8_t{

        _32G = 0x00,
        _16G,
        _08G,
        _04G,
        _02G
    };

    enum class GyroScale: uint8_t{

        _4000_Dps = 0x00,
        _2000_Dps,
        _1000_Dps,
        _0500_Dps,
        _0250_Dps,
        _0125_Dps,
        _0062_Dps,
        _0031_Dps,
        _0015_Dps,
        _0006_Dps

    };

    enum class ODR: uint8_t{

        _6400_Hz = 3,
        _3200_Hz,
        _1600_Hz,
        _800_Hz,
        _400_Hz,
        _200_Hz,
        _100_Hz,
        _50_Hz,
        _25_Hz,
        _12_Hz,
        _6_Hz,
        _3_Hz,
        _1_Hz
    };
};

struct ICM45686_Regs:public ICM45686_Collections{

    struct R8_Int1Config0:public Reg8<>{
        static constexpr auto address = RegAddress{0x16};

        uint8_t en_fifo_full:1;
        uint8_t en_fifo_ths:1;
        uint8_t en_drdy:1;
        uint8_t en_aux1_drdy:1;
        uint8_t en_ap_fsync:1;
        uint8_t en_ap_agc_rdy:1;
        uint8_t en_aux1_agc_rdy:1;
        uint8_t en_reset_done:1;
    };

    struct R8_Int1Config1:public Reg8<>{
        static constexpr auto address = RegAddress{0x17};

        uint8_t en_pll_rdy:1;
        uint8_t en_wom_x:1;
        uint8_t en_wom_y:1;
        uint8_t en_wom_z:1;
        uint8_t en_i3c_protocol_err:1;
        uint8_t en_i2cm_done:1;
        uint8_t en_apex_event:1;
    };

    Vector3_t<int16_t> acc_data_;
    Vector3_t<int16_t> gyr_data_;
};

class ICM45686 final:
    public AccelerometerIntf, 
    public GyroscopeIntf,
    private ICM45686_Regs{
public:
    using Error = ICM45686_Collections::Error;

    ICM45686(hal::I2c & i2c, 
        const hal::I2cSlaveAddr<7> i2c_addr = DEFAULT_I2C_ADDR):
        phy_(i2c, i2c_addr){;}

    [[nodiscard]] IResult<> init();
    [[nodiscard]] IResult<> update();

    [[nodiscard]] IResult<> validate();
    [[nodiscard]] IResult<> set_mode(Mode mode);

    [[nodiscard]] IResult<> reset();

    [[nodiscard]] IResult<Vector3_t<q24>> read_acc();
    [[nodiscard]] IResult<Vector3_t<q24>> read_gyr();
private:
    InvensenseSensor_Phy phy_;
    


    
    [[nodiscard]] IResult<> write_reg(const uint8_t addr, const uint8_t data){
        return phy_.write_reg(addr, data);
    }

    template<typename T>
    [[nodiscard]] IResult<> write_reg(const T & reg){return write_reg(reg.address, reg);}

    [[nodiscard]] IResult<> read_reg(const uint8_t addr, uint8_t & data){
        return phy_.read_reg(addr, data);
    }

    template<typename T>
    [[nodiscard]] IResult<> read_reg(T & reg){return read_reg(reg.address, reg);}

};

}
