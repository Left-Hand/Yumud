#pragma once

#include "core/io/regs.hpp"
#include "core/utils/result.hpp"
#include "core/utils/Errno.hpp"

#include "primitive/pwm_channel.hpp"


#include "hal/conn/i2c/i2cdrv.hpp"


namespace ymd::drivers{


struct MP2980_Prelude{
    static constexpr auto DEFAULT_I2C_ADDR = hal::I2cSlaveAddr<7>::from_u7(0b01100000 >> 1);

    using RegAddr = uint8_t;

    enum class Error_Kind:uint8_t{

    };

    DEF_ERROR_SUMWITH_HALERROR(Error, Error_Kind)

    template<typename T = void>
    using IResult = Result<T, Error>;

    enum class [[nodiscard]] VrefSlewRate:uint8_t{
        _38_V_S,
        _50_V_S,
        _75_V_S,
        _150_V_S
    };

    enum class [[nodiscard]] OvpMode:uint8_t{
        NoProtect,
        Discharge, // default
        LatchOff
    };

    enum class [[nodiscard]] OcpMode:uint8_t{
        CycleByCycle,
        Hiccup, // default
        LatchOff
    };

    enum class [[nodiscard]] BuckBoostFsw:uint8_t{
        _40_P,
        _80_P
    };

    enum class [[nodiscard]] Fsw:uint8_t{
        _200_kHz,
        _300_kHz,
        _400_kHz,
        _600_kHz
    };

    enum class [[nodiscard]] CurrLimitThreshold:uint8_t{
        _27_9_mV,
        _33_3_mV,
        _39_3_mV,
        _45_1_mV,
        _51_2_mV,
        _56_8_mV,
        _62_8_mV,
        _68_7_mV,
    };

    struct [[nodiscard]] Interrupts final{
        uint8_t png:1;
        uint8_t ocp:1;
        uint8_t ovp:1;
        uint8_t __resv__:1;
        uint8_t otp:1;
        uint8_t :3;
    };


    struct [[nodiscard]] VoltageCode final{
        std::array<uint8_t, 2> bytes;

        static constexpr VoltageCode from_mv(const uint16_t mv){
            return VoltageCode{
                .bytes = {
                    uint8_t(mv & 0b111),
                    uint8_t(mv >> 3)
                }
            };
        }

        [[nodiscard]] constexpr uint16_t to_mv() const noexcept {
            auto & self = *this;
            return (uint8_t(self.bytes[1]) << 3) 
                | uint8_t(self.bytes[0]);
        }
    };

    struct RefReg:public Reg16<>{
        static constexpr RegAddr REG_ADDR = RegAddr{0x00};

        VoltageCode code;
    };

};

struct MP2980_Regs:public MP2980_Prelude{



    struct Ctrl1Reg:public Reg8<>{
        static constexpr RegAddr REG_ADDR = RegAddr{0x02};

        uint8_t en_pwr:1;
        uint8_t go_bit:1;
        uint8_t __resv__:1;
        uint8_t png_latch:1;
        uint8_t dither:1;
        uint8_t dischg:1;
        uint8_t sr:2;
    };

    struct Ctrl2Reg:public Reg8<>{
        static constexpr RegAddr REG_ADDR = RegAddr{0x03};

        uint8_t ovp_mode:1;
        uint8_t ocp_mode:1;
        uint8_t bb_fsw:1;
        uint8_t __resv__:1;
        uint8_t fsw:2;
    };

    struct IlimReg:public Reg8<>{
        static constexpr RegAddr REG_ADDR = RegAddr{0x04};

        uint8_t ilim:3;
        uint8_t :5;
    };


    struct StatusReg:public Reg8<>{
        static constexpr RegAddr REG_ADDR = RegAddr{0x05};
        Interrupts interrupts;
    };

    struct MaskReg:public Reg8<>{
        static constexpr RegAddr REG_ADDR = RegAddr{0x06};
        Interrupts interrupts_mask;
    };

    RefReg ref_reg = {};
    Ctrl1Reg ctrl1_reg = {};
    Ctrl2Reg ctrl2_reg = {};
    IlimReg ilim_reg = {};
    MaskReg mask_reg = {};
    StatusReg status_reg = {};
};

}