#pragma once

#include "vce2755_prelude.hpp"

// 这个驱动适用于VCE2755和VCE2758 他们只是在封装上有差异

// 锐评：和KTH7823一样，都是和MA730 pin to pin的芯片
//!! 这款磁编码器只能在18MHz下稳定工作 在36MHz下MISO会延后一个周期输出
//!! 这款磁编码器提供了18位的原始数据，宣称有效位数为14位。 但实测18位不会叠加白噪声，目测约为16位的有效位数

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
    public VCE2755_Prelude{
public:
    struct Config{
        RotateDirection direction;
    };

    explicit VCE2755(const hal::SpiDrv & spi_drv):
        spi_drv_(spi_drv){;}
    explicit VCE2755(hal::SpiDrv && spi_drv):
        spi_drv_(std::move(spi_drv)){;}
    explicit VCE2755(Some<hal::Spi *> spi, const hal::SpiSlaveRank rank):
        spi_drv_(hal::SpiDrv(spi, rank)){;}


    [[nodiscard]] IResult<> init(const Config & cfg);
    [[nodiscard]] IResult<> update();

    [[nodiscard]] IResult<> set_zero_angle(const Angular<uq32> angle);
    [[nodiscard]] IResult<Angular<uq32>> read_lap_angle(){
        return Ok(Angular<uq32>::from_turns(lap_turns_));
    }

    [[nodiscard]] IResult<PackageCode> get_package_code();

private:
    hal::SpiDrv spi_drv_;
    VCE2755_Regset regs_ = {};
    uq32 lap_turns_ = 0;


    IResult<> write_reg(const uint8_t reg_addr, const uint8_t reg_data){
        const auto tx_u16 = uint16_t(
            0x0000 | static_cast<uint16_t>(reg_addr << 8) | reg_data);
        if(const auto res = spi_drv_.write_single<uint16_t>(tx_u16);
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }

    IResult<> read_burst(const uint8_t reg_addr, std::span<uint8_t> bytes){
        const auto tx_u8 = static_cast<uint8_t>(0x80 | reg_addr);
        if(const auto res = spi_drv_.write_single<uint8_t>(tx_u8, CONT);
            res.is_err()) return Err(res.unwrap_err());
        if(const auto res = spi_drv_.read_burst<uint8_t>(bytes);
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }

    IResult<> read_reg(const uint8_t reg_addr, uint8_t & reg_data){
        return read_burst(reg_addr, std::span(&reg_data, 1));
    }

    template<typename T>
    [[nodiscard]] IResult<> write_reg(const RegCopy<T> & reg){
        constexpr auto address = T::ADDRESS;
        const uint8_t data = reg.to_bits();
        if(const auto res = write_reg(address, data); 
            res.is_err()) return Err(res.unwrap_err());
        reg.apply();
        return Ok();
    }


    template<typename T>
    [[nodiscard]] IResult<> read_reg(T & reg){
        constexpr auto address = T::ADDRESS;
        if(const auto res = read_reg(address, reg.as_bits_mut()); 
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }


};

}