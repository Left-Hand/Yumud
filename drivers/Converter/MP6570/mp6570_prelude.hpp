#pragma once

#include <optional>

#include "core/io/regs.hpp"
#include "core/utils/Errno.hpp"

#include "hal/bus/i2c/i2cdrv.hpp"
#include "hal/bus/spi/spidrv.hpp"


namespace ymd::drivers::mp6750{






struct [[nodiscard]] WriteConfig{
    const uint8_t slave_addr;
    const uint8_t reg_addr;
    const bool pen;
    const uint16_t data;
};

struct [[nodiscard]] ReadConfig{
    const uint8_t slave_addr;
    const uint8_t reg_addr;
    const bool pen;
};

[[nodiscard]] static constexpr size_t count_ones(const uint16_t val){
    return __builtin_popcount(val);
}

[[nodiscard]] static constexpr bool is_odd(const uint16_t val){
    return count_ones(val) & 0b1;
}


struct [[nodiscard]] TxPacket{
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

                uint8_t((cfg.data >> 12) & 0b1111), 

                uint8_t(cfg.data >> 4),

                uint8_t(uint8_t((cfg.data & 0xff) << 4) | 
                    uint8_t(cfg.pen ? 
                        (is_odd(cfg.data) ? uint8_t(0b10) : uint8_t(0b00)) 
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
};
static constexpr Option<uint16_t> rx_bytes_to_data(std::span<const uint8_t, 3> bytes){

    //fuck you big endian!!!
    const uint32_t buf = uint32_t(bytes[2]) | uint32_t(bytes[1] << 8) | uint32_t(bytes[0] << 16);   
    const uint16_t data = uint16_t(buf >> 4);

    if((bytes[0] & 0xf0) != 0xf0) return None;
    if((bytes[2] & 0x0e) != 0x0e) return None;

    bool assume_is_odd = is_odd(data);
    bool is_odd_bit = (bytes[2] & 0x01);

    if(assume_is_odd != is_odd_bit) return None;

    return Some(data);

}

using SlaveAddress = uint8_t;

enum class Error_Kind:uint8_t{

};

DEF_ERROR_SUMWITH_HALERROR(Error, Error_Kind)

using RegAddr = uint8_t;

struct [[nodiscard]] MP6570_Transport{
public: 

    MP6570_Transport(const hal::I2cDrv & i2c_drv) : 
        i2c_drv_(i2c_drv) {}
    MP6570_Transport(hal::I2cDrv && i2c_drv) : 
        i2c_drv_(std::move(i2c_drv)) {}

    MP6570_Transport(const SlaveAddress spi_slave_addr, const hal::SpiDrv & spi_drv)
        : spi_slave_addr_(spi_slave_addr), spi_drv_(spi_drv) {}
    MP6570_Transport(const SlaveAddress spi_slave_addr, hal::SpiDrv && spi_drv)
        : spi_slave_addr_(spi_slave_addr), spi_drv_(std::move(spi_drv)) {}

    
    // hal::HalResult write_reg(const uint8_t reg_addr, const uint16_t data);
    
    // hal::HalResult read_reg(const uint8_t reg_addr, uint16_t & data);



    hal::HalResult write_reg(const uint8_t reg_addr, const uint16_t data){
        if(i2c_drv_){
            return i2c_drv_->write_reg(reg_addr, data, std::endian::big);
        }else if(spi_drv_){

            const TxPacket packet = TxPacket::from_write({
                .slave_addr = spi_slave_addr_,
                .reg_addr = reg_addr,
                .pen = true,
                .data = data,
            });

            return spi_drv_->write_burst<uint8_t>(
                std::span(packet.payload_bytes),
                DISC);
        }else{
            PANIC("No phy");
            __builtin_unreachable();
        }
    }

    hal::HalResult read_reg(const uint8_t reg_addr, uint16_t & data){
        if(i2c_drv_){
            return i2c_drv_->read_reg(reg_addr, data, std::endian::big);
        }else if(spi_drv_){
            const TxPacket packet = TxPacket::from_read({
                .slave_addr = spi_slave_addr_,
                .reg_addr = reg_addr,
                .pen = true,
            });

            if(const auto res = spi_drv_->write_burst<uint8_t>(
                std::span(packet.payload_bytes),
                CONT); res.is_err()) return res;

            std::array<uint8_t, 3> rx_bytes;

            if(const auto res = spi_drv_->read_burst<uint8_t>(
                std::span(rx_bytes),
                DISC); res.is_err()) return res;

            if(const auto may_data = rx_bytes_to_data(std::span(rx_bytes)); 
                may_data.is_none()
            ){
                PANIC{};
            } else{
                data = may_data.unwrap();
            }
            
        }else{
            PANIC("No phy");
            __builtin_unreachable();
        }
    }

private:
    std::optional<hal::I2cDrv> i2c_drv_;
    SlaveAddress spi_slave_addr_ = 0;
    std::optional<hal::SpiDrv> spi_drv_;
};


struct [[nodiscard]] MP6570_Regs{


    struct [[nodiscard]] R16_ThetaCmdL:public Reg16<>{
        static constexpr RegAddr REG_ADDR = 0x01;

        uint16_t bits;
    };

    struct [[nodiscard]] R16_ThetaCmdM:public Reg16<>{
        static constexpr RegAddr REG_ADDR = 0x02;

        uint16_t bits;
    };

    struct [[nodiscard]] R16_ThetaCmH:public Reg16<>{
        static constexpr RegAddr REG_ADDR = 0x03;

        uint16_t bits;
    };

    struct [[nodiscard]] R16_CycleReg:public Reg16<>{
        static constexpr RegAddr REG_ADDR = 0x04;

        uint8_t pole_pair;
        uint8_t cycle_p;
    };

    struct [[nodiscard]] R16_Ctrl1:public Reg16<>{
        static constexpr RegAddr REG_ADDR = 0x05;
        uint16_t nstep:9;
        uint16_t fgsel:5;
        uint16_t theta_cmd_type:1;
        uint16_t theta_dir:1;
    };

    struct [[nodiscard]] R16_Gain1:public Reg16<>{
        static constexpr RegAddr REG_ADDR = 0x06;

        uint16_t iq_limit:11;
        uint16_t gain_coe:4;
        uint16_t :1;
    };

    struct [[nodiscard]] R16_ThetaBias:public Reg16<>{
        static constexpr RegAddr REG_ADDR = 0x07;

        uint16_t theta_bias:11;
        uint16_t :5;
    };

    struct [[nodiscard]] R32_ThetaKi:public Reg32<>{
        static constexpr RegAddr REG_ADDR = 0x08;

        using Reg32::operator =;
        uint32_t :32;
    };

    struct [[nodiscard]] R16_ErrLimitH:public Reg16<>{
        static constexpr RegAddr REG_ADDR = 0x0a;

        using Reg16::operator =;
        uint16_t bits;
    };

    struct [[nodiscard]] R16_ZP1A:public Reg16<>{
        static constexpr RegAddr REG_ADDR = 0x0b;

        using Reg16::operator =;
        uint16_t bits;
    };

    struct [[nodiscard]] R16_ZP1B:public Reg16<>{
        static constexpr RegAddr REG_ADDR = 0x0c;

        using Reg16::operator =;
        uint16_t bits;
    };

    struct [[nodiscard]] R16_ZP2A:public Reg16<>{
        static constexpr RegAddr REG_ADDR = 0x0d;

        using Reg16::operator =;
        uint16_t bits;
    };

    struct [[nodiscard]] R16_ZP2B:public Reg16<>{
        static constexpr RegAddr REG_ADDR = 0x0e;

        using Reg16::operator =;
        uint16_t bits;
    };

    struct [[nodiscard]] R16_Gain2:public Reg16<>{
        static constexpr RegAddr REG_ADDR = 0x0f;

        uint16_t set_gain2:4;
        uint16_t set_gain1:5;
        uint16_t :7;
    };

    struct [[nodiscard]] R16_IdRef:public Reg16<>{
        static constexpr RegAddr REG_ADDR = 0x10;

        using Reg16::operator =;
    uint16_t bits;        
    };

    struct [[nodiscard]] R16_IqRef:public Reg16<>{
        static constexpr RegAddr REG_ADDR = 0x11;

        using Reg16::operator =;
    uint16_t bits;        
    };

    
    struct [[nodiscard]] R16_CurrentKi:public Reg16<>{
        static constexpr RegAddr REG_ADDR = 0x12;

        using Reg16::operator =;
    uint16_t bits;        
    };

    struct [[nodiscard]] R16_CurrentKp:public Reg16<>{
        static constexpr RegAddr REG_ADDR = 0x13;

        using Reg16::operator =;
    uint16_t bits;        
    };

    struct [[nodiscard]] R16_ThetaDir:public Reg16<>{
        static constexpr RegAddr REG_ADDR = 0x05;

        uint16_t :15;
        uint16_t theta_dir:1;
    };


    struct [[nodiscard]] R16_SpiSdoMod:public Reg16<>{
        static constexpr RegAddr REG_ADDR = 0x33;

        uint16_t spi_o_mod:1;
        uint16_t :15;
    };
};


}