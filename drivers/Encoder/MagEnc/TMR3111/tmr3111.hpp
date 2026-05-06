#pragma once


// TMR3111 是多维科技推出的一款非接触式、兼
// 容对轴和离轴应用的高速、高精度、小封装磁性编码
// 器芯片，通过感应芯片上方磁铁的旋转，采集磁场信
// 号并计算旋转角度。芯片内部集成了隧道磁阻传感器
// (TMR) 及 23 位分辨率信号处理 ASIC，提供丰富的工
// 作参数配置、多种输出接口供客户选择。
// TMR3111 支持 3 线、4 线 SPI 工作模式，客户
// 端 MCU 可通过 SPI 协议读取 23 位绝对位置信息。
// 同时芯片也提供 12 位分辨率 PWM 输出接口，PWM
// 频率可按照客户需求分 4 档配置。
// TMR3111 提 供 ABZ 增 量 模 式 输 出， 最 高 支 持
// 4096 线，A/B 相位正反转可编程，Z 脉冲的宽度及初
// 始相位可编程。
// TMR3111 提供便捷的客户端自校准模式，方便
// 客户自行补偿由结构安装带来的非线性误差。

#include "core/io/regs.hpp"
#include "core/utils/Result.hpp"
#include "primitive/arithmetic/angular.hpp"

#include "hal/conn/spi/spidrv.hpp"


#include "drivers/encoder/encoder.hpp"


namespace ymd::drivers{

struct TMR3111_Prelude{
    using Error = EncoderError;

    template<typename T = void>
    using IResult = Result<T, Error>;

    using RegAddr = uint8_t;

    enum class [[nodiscard]] OpCode:uint8_t{
        ProgramEE = 0b001,
        WriteReg = 0b101,
        ReadReg = 0b110,
        ChangeMode = 0b111,
        ReadAngle = 0b011
    };

    static constexpr uint32_t make_txcode(
        const OpCode op_code, 
        const uint8_t arg1, 
        const uint16_t arg2
    ){
        uint32_t sum = 0;
        sum |= static_cast<uint32_t>(op_code) << (32 - 3);
        sum |= static_cast<uint32_t>(arg1) << (32 - 11);
        sum |= static_cast<uint32_t>(arg2);
        return sum;
    }

    struct [[nodiscard]] AngleCode final{
        uint32_t bits;

        [[nodiscard]] constexpr bool is_error() const noexcept {return bits & 0x01;}
        [[nodiscard]] constexpr uint32_t angle23() const noexcept {
            constexpr uint32_t MASK = ((1 << 23) - 1);
            return (bits >> 5) & MASK;
        }


        [[nodiscard]] constexpr uint8_t checksum4() const noexcept {
            constexpr uint32_t MASK = ((1 << 4) - 1);
            return (bits >> 1) & MASK;
        }

        [[nodiscard]] constexpr uint8_t calc_actual_checksum() const noexcept {
            // 4-bit CRC 检验码，校验范围为 1-bit “0” 后接 23-bit 角度值（总共 24-bit 数据）,CRC 校验多
            // 项式为 x^4 + x^3 + x^2 + 1,初始值为 4'b0011

            const uint32_t data = angle23(); // 23-bit value; bit23 is treated as 0
            uint8_t crc = 0b0011; // initial value
            constexpr uint8_t poly = 0b1101; // x^3 + x^2 + 1 (for polynomial x^4 + x^3 + x^2 + 1)

            for (int i = 23; i >= 0; --i) {
                const uint8_t bit = (data >> i) & 1U;
                crc ^= (bit << 3);
                crc = (crc & 0x08) ? ((crc << 1) ^ poly) : (crc << 1);
                crc &= 0x0F;
            }
            return crc;
        }

        [[nodiscard]] constexpr bool is_crc_valid(){
            //todo
            return true;
        }
    };

};

struct TMR3111_Regset{

};

class TMR3111 final:
    public TMR3111_Prelude{
public:
    struct Config{
        RotateDirection direction;
    };

    explicit TMR3111(const hal::SpiDrv & spi_drv):
        spi_drv_(spi_drv){;}
    explicit TMR3111(hal::SpiDrv && spi_drv):
        spi_drv_(std::move(spi_drv)){;}
    explicit TMR3111(Some<hal::Spi *> spi, const hal::SpiSlaveRank rank):
        spi_drv_(hal::SpiDrv(spi, rank)){;}


private:
    hal::SpiDrv spi_drv_;
    TMR3111_Regset regs_ = {};
    uq32 lap_turns_ = 0;

    IResult<> transceive32(uint32_t & rx, const uint32_t tx){
        std::array<uint16_t, 2> rxbuf;

        if(const auto res = spi_drv_.transceive_single<uint16_t>(
            rxbuf[1], static_cast<uint16_t>(tx >> 16), CONT
        ); res.is_err()) return Err(res.unwrap_err());

        if(const auto res = spi_drv_.transceive_single<uint16_t>(
            rxbuf[0], static_cast<uint16_t>(tx), CONT
        ); res.is_err()) return Err(res.unwrap_err());

        rx = std::bit_cast<uint32_t>(rxbuf);

        return Ok();
    }

    IResult<> write_reg(const uint8_t reg_addr, const uint16_t reg_val){
        uint32_t dummy_rx;
        return transceive32(dummy_rx, make_txcode(
            OpCode::WriteReg,
            reg_addr,
            reg_val
        ));
    }

    template<typename T>
    IResult<> write_reg(const RegCopy<T> & reg){
        const uint8_t bits = reg.to_bits();
        if(const auto res = write_reg(T::REG_ADDR, bits); 
            res.is_err()) return Err(res.unwrap_err());
        reg.apply();
        return Ok();
    }


    template<typename T>
    IResult<> read_reg(T & reg){
        if(const auto res = read_reg(T::REG_ADDR, reg.as_bits_mut()); 
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }


};

}

