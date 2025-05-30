#pragma once


#include "core/utils/Result.hpp"
#include "core/utils/errno.hpp"

#include "hal/bus/i2c/i2csw.hpp"
#include "hal/gpio/gpio.hpp"


namespace ymd::drivers{


struct CH455_Collections{
    enum class Error_Kind:uint8_t{
        BrightnessGreaterThanMax,
        BrightnessLessThanMin,
        ReadKeyBit2VerifyFailed,
        ReadKeyBit7VerifyFailed,
    };

    DEF_ERROR_SUMWITH_HALERROR(Error, Error_Kind)

    template<typename T = void>
    using IResult = Result<T, Error>;

    static constexpr uint8_t CH455_BIT_ENABLE = 0x01;		// 开启/关闭位
    static constexpr uint8_t CH455_BIT_SLEEP = 0x04;		// 睡眠控制位
    static constexpr uint8_t CH455_BIT_7SEG = 0x08;		// 7段控制位
    static constexpr uint8_t CH455_BIT_INTENS1 = 0x10;		// 1级亮度
    static constexpr uint8_t CH455_BIT_INTENS2 = 0x20;		// 2级亮度
    static constexpr uint8_t CH455_BIT_INTENS3 = 0x30;		// 3级亮度
    static constexpr uint8_t CH455_BIT_INTENS4 = 0x40;		// 4级亮度
    static constexpr uint8_t CH455_BIT_INTENS5 = 0x50;		// 5级亮度
    static constexpr uint8_t CH455_BIT_INTENS6 = 0x60;		// 6级亮度
    static constexpr uint8_t CH455_BIT_INTENS7 = 0x70;		// 7级亮度
    static constexpr uint8_t CH455_BIT_INTENS8 = 0x00;		// 8级亮度
    
    static constexpr uint16_t CH455_SYSOFF = 0x0400;			// 关闭显示、关闭键盘
    static constexpr uint16_t CH455_SYSON	= 	( CH455_SYSOFF | CH455_BIT_ENABLE );	// 开启显示、键盘
    static constexpr uint16_t CH455_SLEEPOFF = CH455_SYSOFF;	// 关闭睡眠
    static constexpr uint16_t CH455_SLEEPON	= ( CH455_SYSOFF | CH455_BIT_SLEEP );	// 开启睡眠
    static constexpr uint16_t CH455_7SEG_ON	= ( CH455_SYSON | CH455_BIT_7SEG );	// 开启七段模式
    static constexpr uint16_t CH455_8SEG_ON	= ( CH455_SYSON | 0x00 );	// 开启八段模式
    static constexpr uint16_t CH455_SYSON_4	= ( CH455_SYSON | CH455_BIT_INTENS4 );	// 开启显示、键盘、4级亮度
    static constexpr uint16_t CH455_SYSON_8	= ( CH455_SYSON | CH455_BIT_INTENS8 );	// 开启显示、键盘、8级亮度
    
    
    // 加载字数据命令
    static constexpr uint16_t CH455_DIG0 = 0x1400;			// 数码管位0显示,需另加8位数据
    static constexpr uint16_t CH455_DIG1 = 0x1500;			// 数码管位1显示,需另加8位数据
    static constexpr uint16_t CH455_DIG2 = 0x1600;			// 数码管位2显示,需另加8位数据
    static constexpr uint16_t CH455_DIG3 = 0x1700;			// 数码管位3显示,需另加8位数据
    
    // 读取按键代码命令
    static constexpr uint16_t CH455_GET_KEY = 0x0700;					// 获取按键,返回按键代码
    
    // CH455接口定义
    static constexpr uint8_t		CH455_I2C_ADDR =		0x40;			// CH455的地址
    static constexpr uint8_t		CH455_I2C_MASK =		0x3E;			// CH455的高字节命令掩码

    static constexpr auto BCD_DEC_TABLE = std::to_array<uint8_t>({ 0X3F, 0X06, 0X5B, 0X4F, 0X66, 0X6D, 0X7D, 0X07, 0X7F, 0X6F, 
        0X77, 0X7C, 0X58, 0X5E, 0X79, 0X71 });//BCD

    static constexpr uint16_t dig2raw(const uint8_t digit, const uint8_t value){
        switch(digit & 0b11){
            case 0: return CH455_DIG0 | value;
            case 1: return CH455_DIG1 | value;
            case 2: return CH455_DIG2 | value;
            case 3: return CH455_DIG3 | value;
            default: __builtin_unreachable();
        }
    }
};


class CH455_phy final :public CH455_Collections{
public:
    CH455_phy(hal::Gpio & scl, hal::Gpio & sda):
        i2c_(scl, sda){;}

    IResult<> write_u16(const uint16_t cmd){
        if(const auto res = i2c_.begin(
                hal::LockRequest((uint8_t(cmd>>7)&CH455_I2C_MASK)|CH455_I2C_ADDR, 0)
            );
            res.is_err()) return Err(res.unwrap_err());
        
        const auto res = i2c_.write(uint8_t(cmd&0xFF));
        i2c_.end();
        if(res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }

    IResult<uint8_t> read_u8(){
        if(const auto res = i2c_.begin(
                hal::LockRequest((uint8_t(CH455_GET_KEY>>7) & CH455_I2C_MASK) | 0x01 | CH455_I2C_ADDR, 0));
            res.is_err()) return Err(res.unwrap_err());

        uint32_t ret;
        const auto res = i2c_.read(ret, NACK);
        i2c_.end();
        if(res.is_err()) return Err(res.unwrap_err());
        return Ok(uint8_t(ret));
    }

    void init(){
        i2c_.init(100000);
    }
private:
    hal::I2cSw i2c_;
};
}