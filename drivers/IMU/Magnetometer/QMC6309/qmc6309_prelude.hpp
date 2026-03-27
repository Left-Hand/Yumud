#pragma once

#include "core/io/regs.hpp"
#include "core/utils/enum/enum_array.hpp"
#include "drivers/IMU/IMU.hpp"
#include <tuple>

#include "hal/conn/i2c/i2cdrv.hpp"


namespace ymd::drivers{

struct QMC6309_Prelude{

    static constexpr auto DEFAULT_I2C_ADDR = hal::I2cSlaveAddr<7>::from_u7(0b1111'100);

    using Error = ImuError;
    template<typename T = void>
    using IResult = Result<T, Error>;


    enum class [[nodiscard]] Mode:uint8_t{
        Suspend = 0b00,
        Normal = 0b01,
        Single = 0b10,
        Continuous = 0b11
    };

    enum class [[nodiscard]] OverSampleRatio:uint8_t{
        _8 = 0b00,
        _4 = 0b01,
        _2 = 0b10,
        _1 = 0b11
    };

    enum class [[nodiscard]] Lpf:uint8_t{
        _1 = 0b000,
        _2 = 0b001,
        _4 = 0b010,
        _8 = 0b011,
        _16 = 0b100,
    };

    enum class Range:uint8_t {
        _32G = 0b00,
        _16G = 0b01,
        _8G = 0b10,
    };

    enum class [[nodiscard]] RegAddr:uint8_t{
        ChipId = 0x00,
        Status1 = 0x09,
        Control1 = 0x0A,
        Control2 = 0x0B,
        Control3 = 0x0e,
        SelftestX = 0x13,
        SelftestY = 0x14,
        SelftestZ = 0x15
    };
};

struct QMC6309_Regs:public QMC6309_Prelude{


    struct NestedXyz{
        int16_t x;
        int16_t y;
        int16_t z;
    };

    struct R8_Status1:public Reg8<>{
        static constexpr RegAddr REG_ADDR = RegAddr::Status1;

        uint8_t ready:1;
        uint8_t ovl:1;
        uint8_t st_rdy:1;
        uint8_t nvm_rdy:1;
        uint8_t nvm_load_done:1;
        uint8_t :3;
    };

    struct R8_Control1:public Reg8<>{
        static constexpr RegAddr REG_ADDR = RegAddr::Control1;

        uint8_t mode:2;
        uint8_t __resv__:1;
        uint8_t osr:2;
        uint8_t osr2:3;
    };

    struct R8_Control2:public Reg8<>{
        static constexpr RegAddr REG_ADDR = RegAddr::Control2;

        uint8_t mode:2;
        uint8_t rng:2;
        uint8_t odr:3;
        uint8_t soft_rst:1;
    };


    struct R8_Control3:public Reg8<>{
        static constexpr RegAddr REG_ADDR = RegAddr::Control3;

        uint8_t __resv__:7;
        uint8_t selftest:1;
    };

    

};



}