#pragma once

#include "core/io/regs.hpp"
#include "drivers/IMU/IMU.hpp"

#include "hal/bus/i2c/i2cdrv.hpp"

namespace ymd::drivers{

struct HMC5883L_Prelude{
    using Error = ImuError;
    template<typename T = void>
    using IResult = Result<T, Error>;

    enum class [[nodiscard]] Odr:uint8_t{
        DR0_75, DR1_5, DR3, DR7_5, DR15, DR30, DR75
    };

    enum class [[nodiscard]] SampleNumber:uint8_t{
        SN1, SN2, SN4, SN8
    };

    enum class [[nodiscard]] Gain:uint8_t{
        GL0_73, GL0_92, GL1_22, GL1_52, GL2_27, GL2_56, GL3_03, GL4_35
    };

    enum class [[nodiscard]] Mode:uint8_t{
        Continuous = 0, 
        Single = 1
    };

    static constexpr auto DEFAULT_I2C_ADDR = hal::I2cSlaveAddr<7>::from_u7(0x3d >> 1);

    enum class [[nodiscard]] RegAddr:uint8_t{
        ConfigA = 0x00,
        ConfigB = 0x01,
        Mode = 0x02,
        MagX = 0x03,
        MagY = 0x05,
        MagZ = 0x07,
        Status = 0x09,
        IDA = 10,
        IDB = 11,
        IDC = 12
    };

};

struct HMC5883L_Regset:public HMC5883L_Prelude{


    struct R8_ConfigA:public Reg8<>{
        static constexpr RegAddr ADDRESS = RegAddr::ConfigA;
        uint8_t measureMode:3;
        uint8_t dataRate:2;
        uint8_t sampleNumber:2;
        uint8_t __resv__:1;
    }DEF_R8(config_a_reg)

    struct R8_ConfigB:public Reg8<>{
        static constexpr RegAddr ADDRESS = RegAddr::ConfigB;
        uint8_t __resv__:5;
        Gain gain:3;
    }DEF_R8(config_b_reg)

    struct R8_Mode:public Reg8<>{
        static constexpr RegAddr ADDRESS = RegAddr::Mode;
        Mode mode:2;
        uint8_t __resv__:5;
        uint8_t hs:1;
    }DEF_R8(mode_reg)

    struct R8_Status:public Reg8<>{
        static constexpr RegAddr ADDRESS = RegAddr::Status;
        uint8_t ready:1;
        uint8_t lock:1;
        uint8_t __resv__:6;
    }DEF_R8(status_reg)

    struct R8_IdA:public Reg8<>{
        static constexpr RegAddr ADDRESS = RegAddr::IDA;
        uint8_t data;
    }DEF_R8(id_a_reg)

    struct R8_IdB:public Reg8<>{
        static constexpr RegAddr ADDRESS = RegAddr::IDB;
        uint8_t data;
    }DEF_R8(id_b_reg)

    struct R8_IdC:public Reg8<>{
        static constexpr RegAddr ADDRESS = RegAddr::IDC;
        uint8_t data;
    }DEF_R8(id_c_reg)

    int16_t mag_x_reg = {};
    int16_t mag_y_reg = {};
    int16_t mag_z_reg = {};
};

};