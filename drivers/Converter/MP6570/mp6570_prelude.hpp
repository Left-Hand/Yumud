#pragma once

#include <optional>

#include "core/utils/reg_base.hpp"
#include "core/utils/Errno.hpp"

#include "hal/conn/i2c/i2cdrv.hpp"
#include "hal/conn/spi/spidrv.hpp"


namespace ymd::drivers::mp6570{


struct [[nodiscard]] WriteConfig final{
    uint8_t slave_addr;
    uint8_t reg_addr;
    bool pen;
    uint16_t reg_val;
};

struct [[nodiscard]] ReadConfig final{
    uint8_t slave_addr;
    uint8_t reg_addr;
    bool pen;
};



struct [[nodiscard]] TxPacket final{
    using Self = TxPacket;

    std::array<uint8_t, 2> header_bytes;
    std::array<uint8_t, 3> payload_bytes;



    static constexpr TxPacket from_write(const WriteConfig & cfg){
        return TxPacket{
            .header_bytes = {
                uint8_t(uint8_t(cfg.slave_addr << 1) | uint8_t(0b1)),

                uint8_t(cfg.reg_addr | uint8_t(cfg.pen)), 
            },
            .payload_bytes = {

                uint8_t((cfg.reg_val >> 12) & 0b1111), 

                uint8_t(cfg.reg_val >> 4),

                uint8_t(uint8_t((cfg.reg_val & 0xff) << 4) | 
                    uint8_t(cfg.pen ? 
                        (is_odd(cfg.reg_val) ? uint8_t(0b10) : uint8_t(0b00)) 
                        : uint8_t(0b0))),
            }
        };
    }


    static constexpr TxPacket from_read(const ReadConfig & cfg){
        return TxPacket{
            .header_bytes = {
                uint8_t(uint8_t(cfg.slave_addr << 1) | uint8_t(0b0)),

                uint8_t(cfg.reg_addr | uint8_t(cfg.pen)), 
            },
            .payload_bytes = {
                0,0,0
            }
        };
    }

    [[nodiscard]] static constexpr bool is_odd(const uint16_t val){
        return __builtin_popcount(val) & 0b1;
    }

};

using SlaveAddress = uint8_t;

enum class Error_Kind:uint8_t{

};

DEF_ERROR_SUMWITH_HALERROR(Error, Error_Kind)

using RegAddr = uint8_t;



struct [[nodiscard]] MP6570_Regset{


struct [[nodiscard]] R16_ThetaCmdL:public Reg16{
    static constexpr RegAddr REG_ADDR = RegAddr{0x01};

    uint16_t bits;
};

struct [[nodiscard]] R16_ThetaCmdM:public Reg16{
    static constexpr RegAddr REG_ADDR = RegAddr{0x02};

    uint16_t bits;
};

struct [[nodiscard]] R16_ThetaCmH:public Reg16{
    static constexpr RegAddr REG_ADDR = RegAddr{0x03};

    uint16_t bits;
};

struct [[nodiscard]] R16_CycleReg:public Reg16{
    static constexpr RegAddr REG_ADDR = RegAddr{0x04};

    uint8_t pole_pair;
    uint8_t cycle_p;
};

struct [[nodiscard]] R16_Ctrl1:public Reg16{
    static constexpr RegAddr REG_ADDR = RegAddr{0x05};
    uint16_t nstep:9;
    uint16_t fgsel:5;
    uint16_t theta_cmd_type:1;
    uint16_t theta_dir:1;
};

struct [[nodiscard]] R16_Gain1:public Reg16{
    static constexpr RegAddr REG_ADDR = RegAddr{0x06};

    uint16_t iq_limit:11;
    uint16_t gain_coe:4;
    uint16_t __resv__:1;
};

struct [[nodiscard]] R16_ThetaBias:public Reg16{
    static constexpr RegAddr REG_ADDR = RegAddr{0x07};

    uint16_t theta_bias:11;
    uint16_t :5;
};

struct [[nodiscard]] R32_ThetaKi:public Reg32{
    static constexpr RegAddr REG_ADDR = RegAddr{0x08};

    uint32_t bits;
};

struct [[nodiscard]] R16_ErrLimitH:public Reg16{
    static constexpr RegAddr REG_ADDR = RegAddr{0x0a};

    uint16_t bits;
};

struct [[nodiscard]] R16_ZP1A:public Reg16{
    static constexpr RegAddr REG_ADDR = RegAddr{0x0b};

    uint16_t bits;
};

struct [[nodiscard]] R16_ZP1B:public Reg16{
    static constexpr RegAddr REG_ADDR = RegAddr{0x0c};

    uint16_t bits;
};

struct [[nodiscard]] R16_ZP2A:public Reg16{
    static constexpr RegAddr REG_ADDR = RegAddr{0x0d};

    uint16_t bits;
};

struct [[nodiscard]] R16_ZP2B:public Reg16{
    static constexpr RegAddr REG_ADDR = RegAddr{0x0e};

    uint16_t bits;
};

struct [[nodiscard]] R16_Gain2:public Reg16{
    static constexpr RegAddr REG_ADDR = RegAddr{0x0f};

    uint16_t set_gain2:4;
    uint16_t set_gain1:5;
    uint16_t :7;
};

struct [[nodiscard]] R16_IdRef:public Reg16{
    static constexpr RegAddr REG_ADDR = RegAddr{0x10};

    uint16_t bits;        
};

struct [[nodiscard]] R16_IqRef:public Reg16{
    static constexpr RegAddr REG_ADDR = RegAddr{0x11};

    uint16_t bits;        
};


struct [[nodiscard]] R16_CurrentKi:public Reg16{
    static constexpr RegAddr REG_ADDR = RegAddr{0x12};

    uint16_t bits;        
};

struct [[nodiscard]] R16_CurrentKp:public Reg16{
    static constexpr RegAddr REG_ADDR = RegAddr{0x13};

    uint16_t bits;        
};

struct [[nodiscard]] R16_ThetaDir:public Reg16{
    static constexpr RegAddr REG_ADDR = RegAddr{0x05};

    uint16_t :15;
    uint16_t theta_dir:1;
};


struct [[nodiscard]] R16_SpiSdoMod:public Reg16{
    static constexpr RegAddr REG_ADDR = RegAddr{0x33};

    uint16_t spi_o_mod:1;
    uint16_t :15;
};
};


}