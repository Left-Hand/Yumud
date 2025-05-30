//这个驱动已经完成
//这个驱动已经测试

#pragma once

#include "core/io/regs.hpp"
#include "core/utils/Result.hpp"
#include "core/utils/Errno.hpp"

#include "hal/gpio/vport.hpp"
#include "hal/bus/i2c/i2cdrv.hpp"

#include "concept/pwm_channel.hpp"



namespace ymd::drivers{
struct AW9523_Collections{
    enum class Error_Kind:uint8_t{
        WrongChipId,
        IndexOutOfRange
    };

    DEF_ERROR_SUMWITH_HALERROR(Error, Error_Kind)

    template<typename T = void>
    using IResult = Result<T, Error>;

    enum class CurrentLimit{
        Max, High, Medium, Low
    };

    scexpr auto DEFAULT_I2C_ADDR = hal::I2cSlaveAddr<7>::from_u8(0b10110000);
};

struct AW9523_Regs:public AW9523_Collections{
    static constexpr uint8_t VALID_CHIP_ID = 0x23;
    
    struct CtlReg:Reg8<>{
        uint8_t isel:2;
        uint8_t __resv1__:2;
        uint8_t p0mod:1;
        uint8_t __resv2__:3;
    };

    enum class RegAddress:uint8_t{
        In = 0x00,
        Out = 0x02,
        Dir = 0x04,
        Inten = 0x06,
        ChipId = 0x10,
        Ctl = 0x11,
        LedMode = 0x12,
        DimP10 = 0x20,
        DimP11 = 0x21,
        DimP12 = 0x22,
        DimP13 = 0x23,
        DimP00 = 0x24,
        DimP01 = 0x25,
        DimP02 = 0x26,
        DimP03 = 0x27,
        DimP04 = 0x28,
        DimP05 = 0x29,
        DimP06  = 0x2a,
        DimP07  = 0x2b,
        DimP14  = 0x2c,
        DimP15  = 0x2d,
        DimP16  = 0x2e,
        DimP17  = 0x2f,
        SwRst = 0x7f
    };

    uint16_t dir_reg;
    uint16_t inten;
    CtlReg ctl;
    hal::PinMask led_mode_ = hal::PinMask(0);


};

class AW9523 final:
    public AW9523_Regs{
public:
public:

    class AW9523Pwm:public hal::PwmIntf{
    protected:
        AW9523 & aw9523;
        hal::PinSource pin;

        AW9523Pwm(AW9523 & _aw9523, const hal::PinSource _pin):aw9523(_aw9523), pin(_pin){;}

        DELETE_COPY_AND_MOVE(AW9523Pwm)
        
        friend class AW9523;
    public:

        void init(){
            aw9523.enable_led_mode(pin).examine();
        }

        void set_duty(const real_t duty) {
            aw9523.set_led_current(pin,int(255 * duty)).examine();
        }
    };

    class AW9523Port{

    };


    AW9523(const hal::I2cDrv & i2c_drv):i2c_drv_(i2c_drv){;}
    AW9523(hal::I2cDrv && i2c_drv):i2c_drv_(i2c_drv){;}
    AW9523(hal::I2c & bus):i2c_drv_(hal::I2cDrv(bus, DEFAULT_I2C_ADDR)){;}

    IResult<> init();
    [[nodiscard]] IResult<> reset(){
        return write_reg(RegAddress::SwRst, (uint8_t)0x00);
    }

    [[nodiscard]] IResult<> set_by_mask(const hal::PinMask mask) {
        buf_mask_ = buf_mask_ | mask;
        return write_by_mask(buf_mask_);
    }

    [[nodiscard]] IResult<> clr_by_mask(const hal::PinMask mask) {
        buf_mask_ = buf_mask_ & (~mask);
        return write_by_mask(buf_mask_);
    }

    [[nodiscard]] IResult<> write_by_index(const size_t index, const BoolLevel data) ;
    
    [[nodiscard]] IResult<BoolLevel> read_by_index(const size_t index) ;

    [[nodiscard]] IResult<> set_mode(const size_t index, const hal::GpioMode mode) ;

    [[nodiscard]] IResult<> enable_irq_by_index(const size_t index, const Enable en = EN);

    [[nodiscard]] IResult<> enable_led_mode(const hal::PinSource pin, const Enable en = EN);

    [[nodiscard]] IResult<> set_led_current_limit(const CurrentLimit limit);

    [[nodiscard]] IResult<> set_led_current(const hal::PinSource pin, const uint8_t current);
    
    [[nodiscard]] IResult<> validate();

    AW9523Pwm operator [](const size_t index){
        return AW9523Pwm(*this, hal::PinSource(1 << index));
    }
private:
    hal::I2cDrv i2c_drv_;
    hal::PinMask buf_mask_ = hal::PinMask(0);

    static constexpr RegAddress get_dim_addr(const size_t idx){
        switch(idx){
            case 0 ... 7:
                return std::bit_cast<RegAddress>(uint8_t(uint8_t(RegAddress::DimP00) + idx));
            case 8 ... 11:
                return std::bit_cast<RegAddress>(uint8_t(uint8_t(RegAddress::DimP10) + idx));
            case 12 ... 16:
                return std::bit_cast<RegAddress>(uint8_t(uint8_t(RegAddress::DimP14) + idx));
            default:
                __builtin_unreachable();
        }
    }

    static constexpr bool is_index_valid(const size_t idx){
        return idx < 16;
    }


    [[nodiscard]] IResult<> write_reg(const RegAddress addr, const uint8_t data){
        if(const auto res = i2c_drv_.write_reg(uint8_t(addr), data);
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    };

    [[nodiscard]] IResult<> write_reg(const RegAddress addr, const uint16_t data){
        if(const auto res = i2c_drv_.write_reg(uint8_t(addr), data, LSB);
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }

    [[nodiscard]] IResult<> read_reg(const RegAddress addr, uint8_t & data){
        if(const auto res = i2c_drv_.read_reg(uint8_t(addr), data);
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }

    [[nodiscard]] IResult<> read_reg(const RegAddress addr, uint16_t & data){
        if(const auto res = i2c_drv_.read_reg(uint8_t(addr), data, LSB);
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }

    [[nodiscard]] IResult<> write_by_mask(const hal::PinMask mask) {
        buf_mask_ = mask;
        return write_reg(RegAddress::Out, buf_mask_.as_u16());
    }

    [[nodiscard]] IResult<hal::PinMask> read_mask() {
        uint16_t buf;
        if(const auto res = read_reg(RegAddress::In, buf);
            res.is_err()) return Err(res.unwrap_err());
        return Ok(hal::PinMask(buf));
    }
};

};