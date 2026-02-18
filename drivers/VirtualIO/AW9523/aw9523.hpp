//这个驱动已经完成
//这个驱动已经测试

#pragma once

#include "aw9523_prelude.hpp"


namespace ymd::drivers{


class AW9523 final:
    public AW9523_Prelude{
public:

    explicit AW9523(const hal::I2cDrv & i2c_drv):
        i2c_drv_(i2c_drv){;}
    explicit AW9523(hal::I2cDrv && i2c_drv):
        i2c_drv_(std::move(i2c_drv)){;}
    explicit AW9523(Some<hal::I2cBase *> i2c, hal::I2cSlaveAddr<7> addr = DEFAULT_I2C_ADDR):
        i2c_drv_(hal::I2cDrv(i2c, addr)){;}

    
    [[nodiscard]] IResult<> init(const Config & cfg);
    [[nodiscard]] IResult<> reset(){
        return write_reg(RegAddr::SwRst, (uint8_t)0x00);
    }

    [[nodiscard]] IResult<> set_by_mask(const hal::PinMask mask) {
        buf_mask_ = buf_mask_ | mask;
        return write_by_mask(buf_mask_);
    }

    [[nodiscard]] IResult<> clr_by_mask(const hal::PinMask mask) {
        buf_mask_ = buf_mask_ & (~mask);
        return write_by_mask(buf_mask_);
    }

    [[nodiscard]] IResult<> write_nth(
        const Nth nth, 
        const BoolLevel data) ;
    
    [[nodiscard]] IResult<BoolLevel> read_nth(const Nth nth) ;

    [[nodiscard]] IResult<> set_mode(
        const Nth nth, 
        const hal::GpioMode mode) ;

    [[nodiscard]] IResult<> enable_irq(
        const Nth nth, 
        const Enable en);

    [[nodiscard]] IResult<> enable_led_mode(const hal::PinMask pin);

    [[nodiscard]] IResult<> set_led_current_limit(const CurrentLimit limit);

    [[nodiscard]] IResult<> set_led_current_dutycycle(
        const hal::PinMask pin, 
        const iq16 dutycycle
    );
    
    [[nodiscard]] IResult<> validate();

private:
    hal::I2cDrv i2c_drv_;
    hal::PinMask buf_mask_ = hal::PinMask::zero();
    AW9523_Regset regs_ = {};



    [[nodiscard]] static constexpr bool 
    is_index_valid(const size_t idx){
        return idx < 16;
    }


    template<typename T>
    [[nodiscard]] IResult<> write_reg(const RegCopy<T> & reg){
        if(const auto res = write_reg(T::ADDRESS, reg.to_bits());
            res.is_err()) return Err(res.unwrap_err());
        reg.apply();
        return Ok();
    }

    template<typename T>
    [[nodiscard]] IResult<> read_reg(T & reg){
        return read_reg(T::ADDRESS, reg.as_bits_mut());
    }

    [[nodiscard]] IResult<> write_reg(const RegAddr addr, const uint16_t data);

    template<typename T>
    [[nodiscard]] IResult<> read_reg(const RegAddr addr, T & data){
        if(const auto res = i2c_drv_.read_reg(uint8_t(addr), data, std::endian::little);
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }

    [[nodiscard]] IResult<> write_by_mask(const hal::PinMask mask) {
        buf_mask_ = mask;
        return write_reg(RegAddr::Out, buf_mask_.to_u16());
    }

    [[nodiscard]] IResult<hal::PinMask> read_mask() {
        auto & reg = regs_.input_reg;
        if(const auto res = read_reg(reg);
            res.is_err()) return Err(res.unwrap_err());
        return Ok(reg.mask);
    }
};

};