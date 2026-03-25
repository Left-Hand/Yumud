#pragma once

#include <optional>

#include "core/io/regs.hpp"

#include "drivers/Encoder/encoder.hpp"

#include "hal/conn/i2c/i2cdrv.hpp"
#include "hal/conn/spi/spidrv.hpp"

namespace ymd::drivers{

struct [[nodiscard]] MT6701_Prelude{
static constexpr auto DEFAULT_I2C_ADDR = hal::I2cSlaveAddr<7>::from_u7(0b000110);
using Error = EncoderError;
template<typename T = void>
using IResult = Result<T, Error>;

enum class [[nodiscard]] Hysteresis:uint8_t{
    LSB1, LSB2, LSB4, LSB8,
    LSB0, LSB0_25, LSB00_5
};

enum class [[nodiscard]] ZeroPulseWidth:uint16_t{
    LSB1, LSB2, LSB4, LSB8,LSB12, LSB16, HALF
};

enum class [[nodiscard]] PwmFreq:uint8_t{
    HZ994_4,
    HZ497_2
};


enum class [[nodiscard]] RegAddr:uint8_t{
    RawAngle = 0x03,
    UVWMux = 0x25,
    ABZMux = 0x29,
    Resolution = 0x30,
    ZeroConfig = 0x32,
    Hystersis = 0x34,
    WireConfig = 0x38,
    StartStop = 0x3e,
    Start = 0x3f,
    Stop = 0x40
};


struct [[nodiscard]] alignas(4) Packet final{
    union{
        struct [[nodiscard]] { 
            uint32_t crc:6;
            union{
                struct [[nodiscard]] { 
                    uint32_t stat:2;
                    uint32_t pushed:1;
                    uint32_t overspd:1;
                };
                uint32_t mg:4;
            };
            uint16_t data_14bit:14;
        };
        struct [[nodiscard]] { 
            uint8_t data8;
            uint16_t data16;
        };
    };
    
    static constexpr Packet from_u16(const uint16_t bits){
        Packet self;
        self.data16 = bits;
        return self;
    }

    static constexpr Packet from_u8u16(const uint8_t bits8, const uint16_t bits16){
        Packet self;
        self.data8 = bits8;
        self.data16 = bits16;
        return self;
    }

    static constexpr Packet zero(){
        return Packet::from_u8u16(0, 0);
    }

    __inline constexpr IResult<> validate_fast() const {
        return Ok();
    }

    __inline constexpr IResult<> validate() const {
        if(is_crc_valid() == false) return Err(Error::InvalidCrc);
        return Ok();
    }

private:
    [[nodiscard]] constexpr bool is_crc_valid() const{
        //TODO 添加crc校验算法
        return true;
    }
};

};


class MT6701_TransportIntf:public MT6701_Prelude{
public:
    virtual IResult<> write_reg(const RegAddr reg_addr, const void * p_reg_data, const size_t data_size) = 0;
    virtual IResult<> read_reg(const RegAddr reg_addr, void * p_reg_data, const size_t data_size) = 0;
};


class MT6701_SpiTransport final:public MT6701_TransportIntf{
public:

    MT6701_SpiTransport(Some<hal::Spi *> spi, const hal::SpiSlaveRank rank):
        spi_drv_(hal::SpiDrv(spi, rank)){;}

    IResult<> write_reg(const RegAddr reg_addr, const void * p_reg_data, const size_t data_size);

    IResult<> read_reg(const RegAddr reg_addr, void * p_reg_data, const size_t data_size);
private:

    hal::SpiDrv spi_drv_;
};


class MT6701_I2cTransport final:public MT6701_TransportIntf{
public:

    MT6701_I2cTransport(Some<hal::I2cBase *> i2c, const hal::I2cSlaveAddr<7> i2c_addr):
        MT6701_I2cTransport(hal::I2cDrv(i2c, i2c_addr)){;}

    IResult<> write_reg(const RegAddr reg_addr, const void * p_reg_data, const size_t data_size){
        switch(data_size){
            case 1: return write_reg8(reg_addr, *reinterpret_cast<const uint8_t *>(p_reg_data));
            case 2: return write_reg16(reg_addr, *reinterpret_cast<const uint16_t *>(p_reg_data));
        }
        __builtin_unreachable();
    }

    IResult<> write_reg8(const RegAddr reg_addr, const uint8_t reg_val){
        if(const auto res = i2c_drv_.write_reg(
            std::bit_cast<uint8_t>(reg_addr), 
            reg_val, 
            std::endian::big
        );
            res.is_err()) return Err(res.unwrap_err());

        return Ok();
    }

    IResult<> write_reg16(const RegAddr reg_addr, const uint16_t reg_val){
        if(const auto res = i2c_drv_.write_reg(
            std::bit_cast<uint8_t>(reg_addr), 
            reg_val, 
            std::endian::big
        );
            res.is_err()) return Err(res.unwrap_err());

        return Ok();
    }

    IResult<> read_reg(const RegAddr reg_addr, void * p_reg_data, const size_t data_size){
        switch(data_size){
            case 1: return read_reg8(reg_addr, *reinterpret_cast<uint8_t *>(p_reg_data));
            case 2: return read_reg16(reg_addr, *reinterpret_cast<uint16_t *>(p_reg_data));
        }
        __builtin_unreachable();
    }


    IResult<> read_reg8(const RegAddr reg_addr, uint8_t & reg_val){ 
        if(const auto res = i2c_drv_.read_reg(
            std::bit_cast<uint8_t>(reg_addr), 
            reg_val, 
            std::endian::big
        );
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }

    IResult<> read_reg16(const RegAddr reg_addr, uint16_t & reg_val){ 
        if(const auto res = i2c_drv_.read_reg(
            std::bit_cast<uint8_t>(reg_addr), 
            reg_val, 
            std::endian::big
        );
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }

private:
    MT6701_I2cTransport(
        hal::I2cDrv && i2c_drv
    ):
        i2c_drv_(std::move(i2c_drv)){;}

    hal::I2cDrv i2c_drv_;
};

struct [[nodiscard]] MT6701_Regs:public MT6701_Prelude{
    struct [[nodiscard]] AngleCodeLeftAligned14bit final{
        static constexpr RegAddr REG_ADDR = RegAddr::RawAngle;
        uint16_t bits;
    };

    struct [[nodiscard]] R16_RawAngle : public Reg16<>{
        static constexpr RegAddr REG_ADDR = RegAddr::RawAngle;
        uint16_t bits;
    }DEF_R16(raw_angle_reg)

    struct [[nodiscard]] R8_UVWMux : public Reg8<>{
        static constexpr RegAddr REG_ADDR = RegAddr::UVWMux;
        uint8_t __resv__:7;
        uint8_t uvw_mux:1;
    };

    struct [[nodiscard]] R8_ABZMux : public Reg8<>{
        static constexpr RegAddr REG_ADDR = RegAddr::ABZMux;

        uint8_t __resv1__:1;
        uint8_t is_clockwise:1;
        uint8_t __resv2__:4;
        uint8_t abz_mux:1;
        uint8_t __resv3__:1;
    };

    struct [[nodiscard]] R16_Resolution : public Reg16<>{
        static constexpr RegAddr REG_ADDR = RegAddr::Resolution;
        uint16_t abz_resolution:10;
        uint16_t __resv__:2;
        uint16_t pole_pairs:4;
    };

    struct [[nodiscard]] R16_ZeroConfig : public Reg16<>{
        static constexpr RegAddr REG_ADDR = RegAddr::ZeroConfig;
        uint16_t zero_position:12;
        ZeroPulseWidth zero_pulse_width:3;
        uint16_t hysteresis:1;
    };

    struct [[nodiscard]] R8_Hystersis : public Reg8<>{
        static constexpr RegAddr REG_ADDR = RegAddr::Hystersis;
        uint8_t __resv__:6;
        uint8_t hysteresis:2;
    };

    struct [[nodiscard]] R8_WireConfig : public Reg8<>{
        static constexpr RegAddr REG_ADDR = RegAddr::WireConfig;
        uint8_t __resv__:5;
        uint8_t pwm_en:1;
        uint8_t pwm_polarity_low:1;
        PwmFreq pwm_freq:1;
    };

    struct [[nodiscard]] R8_StartStop : public Reg8<>{
        static constexpr RegAddr REG_ADDR = RegAddr::StartStop;
        uint8_t start:4;
        uint8_t stop:4;
    };

    struct [[nodiscard]] R8_Start:public  Reg8<>{
        static constexpr RegAddr REG_ADDR = RegAddr::Start;
        uint8_t bits;
    };

    struct [[nodiscard]] R8_Stop:public  Reg8<>{
        static constexpr RegAddr REG_ADDR = RegAddr::Stop;
        uint8_t bits;
    };

    R8_UVWMux uvw_mux_reg = {};
    R8_ABZMux abz_mux_reg = {};
    R16_Resolution resolution_reg = {};
    R16_ZeroConfig zero_config_reg = {};
    R8_Hystersis hystersis_reg = {};
    R8_WireConfig wire_config_reg = {};
    R8_StartStop start_stop_reg = {};
    R8_Start start_reg = {};
    R8_Stop stop_reg = {};


};


}
