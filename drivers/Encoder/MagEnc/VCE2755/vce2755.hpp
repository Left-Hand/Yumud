#pragma once

#include "vce2755_prelude.hpp"

// 这个驱动适用于VCE2755和VCE2758 他们只是在封装上有差异

// 锐评：和KTH7823一样，都是和MA730 pin to pin的芯片

// VCE2755是一款基于各向异性磁阻（AMR）技术，高度集成的旋转磁编码器芯片，它在一个小型封装内集成了AMR
// 磁传感器和高精度CMOS处理电路，实现14bit分辨率平行于封装表面的平面360°磁场角度检测。基于AMR在饱
// 和工作模式下对磁场强度变化不敏感的优势，VCE2755具备优异的抗震动和低温漂特性，适用于各种使用环境严
// 苛的场合。由于使用了SAR结构的ADC，VCE2755磁编码器具备极低的信号延迟（<2μs），支持高达18000rpm
// 的高转速，内置的校准算法能对传感器及电路的零偏、幅度和温度做实时补偿，并同时提供了不同角度信号的输出
// 方式：SPI、SSI、ABZ、UVW和PWM，方便用户根据不同需求而选用，适用于各种典型的需要角度位置反馈和速
// 度检测的应用场景。 


// VCE2758是一款基于各向异性磁阻（AMR）技术，高度集成的旋转磁编码器芯片，它在一个小型封装内集成了AMR
// 磁传感器和高精度CMOS处理电路，实现14bit分辨率平行于封装表面的平面360°磁场角度检测。基于AMR在饱
// 和工作模式下对磁场强度变化不敏感的优势，VCE2758具备优异的抗震动和低温漂特性，适用于各种使用环境严
// 苛的场合。由于使用了SAR结构的ADC，VCE2758磁编码器具备极低的信号延迟（<2μs），支持高达18000rpm
// 的高转速，内置的校准算法能对传感器及电路的零偏、幅度和温度做实时补偿，并同时提供了不同角度信号的输出
// 方式：SPI、SSI、ABZ、UVW和PWM，方便用户根据不同需求而选用，适用于各种典型的需要角度位置反馈和速
// 度检测的应用场景。 

namespace ymd::drivers{

class VCE2755 final:
    public VCE2755_Prelude,
    public MagEncoderIntf{
public:
    struct Config{
        RotateDirection direction;
    };

    explicit VCE2755(const hal::SpiDrv & spi_drv):
        spi_drv_(spi_drv){;}
    explicit VCE2755(hal::SpiDrv && spi_drv):
        spi_drv_(std::move(spi_drv)){;}
    explicit VCE2755(Some<hal::SpiBase *> spi, const hal::SpiSlaveRank rank):
        spi_drv_(hal::SpiDrv(spi, rank)){;}


    [[nodiscard]] IResult<> init(const Config & cfg);
    [[nodiscard]] IResult<> update();

    [[nodiscard]] IResult<> set_zero_angle(const Angular<uq32> angle);
    [[nodiscard]] IResult<Angular<uq32>> read_lap_angle(){
        return Ok(Angular<uq32>::from_turns(lap_turns_));
    }

private:
    hal::SpiDrv spi_drv_;
    VCE2755_Regset regs_ = {};
    uq32 lap_turns_ = 0;

    template<typename T>
    [[nodiscard]] IResult<> write_reg(const RegCopy<T> & reg){
        const auto address = T::ADDRESS;
        const uint8_t data = reg.to_bits();
        const auto tx = uint16_t(
            0x8000 | (std::bit_cast<uint8_t>(address) << 8) | data);
        if(const auto res = spi_drv_.write_single<uint16_t>(tx);
            res.is_err()) return Err(Error(res.unwrap_err()));
        reg.apply();
        return Ok();
    }


    template<typename T>
    [[nodiscard]] IResult<> read_reg(T & reg){
        uint16_t dummy;
        const auto addr = std::bit_cast<uint8_t>(T::ADDRESS);
        const auto tx = uint16_t(0x4000 | ((uint8_t)addr << 8));
        if(const auto res = spi_drv_.write_single<uint16_t>(tx); 
            res.is_err()) return Err(Error(res.unwrap_err()));
        if(const auto res = spi_drv_.read_single<uint16_t>(dummy);
            res.is_err()) return Err(Error(res.unwrap_err()));
        if((dummy & 0xff) != 0x00) 
            return Err(Error(Error::Kind::InvalidRxFormat));
        reg.as_bits_mut() = (dummy >> 8);
        return Ok();
    }

    [[nodiscard]] IResult<Packet> read_packet() {
        #if 1
        static constexpr std::array<uint8_t, 4> tx = {0x83, 0x00, 0x00, 0x00};
        std::array<uint8_t, 4> rx;
        if(const auto res = spi_drv_.transceive_burst<uint8_t>(rx, tx);
            res.is_err()) return Err(Error(res.unwrap_err()));
        // DEBUG_PRINTLN(rx);
        return Ok(Packet::from_bytes(rx[1], rx[2], rx[3]));
        #else
        //exprimental
        static constexpr std::array<uint16_t, 2> tx = {0x8300, 0x0000};
        std::array<uint16_t, 2> rx;
        if(const auto res = spi_drv_.transceive_burst<uint16_t>(rx, tx);
            res.is_err()) return Err(Error(res.unwrap_err()));
        return Ok(Packet::from_u24(static_cast<uint32_t>(rx[0] >> 8) | static_cast<uint32_t>(rx[1] << 8)));
        #endif
    }


};

}