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
struct AW9523_Prelude{
    enum class Error_Kind:uint8_t{
        WrongChipId,
        IndexOutOfRange
    };

    DEF_FRIEND_DERIVE_DEBUG(Error_Kind)
    DEF_ERROR_SUMWITH_HALERROR(Error, Error_Kind)

    template<typename T = void>
    using IResult = Result<T, Error>;

    enum class CurrentLimit{
        Max, High, Medium, Low
    };

    scexpr auto DEFAULT_I2C_ADDR = hal::I2cSlaveAddr<7>::from_u7(0b1011000);


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

};

struct AW9523_Regs:public AW9523_Prelude{
    static constexpr uint8_t VALID_CHIP_ID = 0x23;
    static constexpr size_t MAX_CHANNELS = 16;
    
    struct InputReg  : public Reg16<>{
        static constexpr auto ADDRESS = RegAddress::In;

        hal::PinMask mask;
    }DEF_R16(input_reg)

    struct OutputReg  : public Reg16<>{
        static constexpr auto ADDRESS = RegAddress::Out;

        hal::PinMask mask;
    }DEF_R16(output_reg)

    struct DirReg:public Reg16<>{
        static constexpr auto ADDRESS = RegAddress::Dir;

        hal::PinMask mask;
    }DEF_R16(dir_reg)

    struct CtlReg:Reg8<>{
        static constexpr auto ADDRESS = RegAddress::Ctl;
        uint8_t isel:2;
        uint8_t __resv1__:2;
        uint8_t p0mod:1;
        uint8_t __resv2__:3;
    }DEF_R8(ctl_reg)

    struct IntEnReg:public Reg16<>{
        static constexpr auto ADDRESS = RegAddress::Inten;

        hal::PinMask mask;
    }DEF_R16(inten_reg)


    struct LedModeReg:public Reg16<>{
        static constexpr auto ADDRESS = RegAddress::LedMode;

        hal::PinMask mask;
    }DEF_R16(led_mode_reg);


    struct ChipIdReg:public Reg8<>{
        static constexpr auto ADDRESS = RegAddress::ChipId;

        uint8_t id;
    }DEF_R8(chip_id_reg)





    struct Config{
        CurrentLimit current_limit = CurrentLimit::Low;
    };

};

class AW9523 final:
    public AW9523_Regs{
public:

    explicit AW9523(const hal::I2cDrv & i2c_drv):
        i2c_drv_(i2c_drv){;}
    explicit AW9523(hal::I2cDrv && i2c_drv):
        i2c_drv_(std::move(i2c_drv)){;}
    explicit AW9523(Some<hal::I2c *> i2c):
        i2c_drv_(hal::I2cDrv(i2c, DEFAULT_I2C_ADDR)){;}


    class AW9523Pwm:public hal::PwmIntf{
    protected:
        AW9523Pwm(AW9523 & aw9523, const hal::PinSource pin):aw9523_(aw9523), pin_(pin){;}

        DELETE_COPY_AND_MOVE(AW9523Pwm)
        
        friend class AW9523;
    public:

        void init(){
            aw9523_.enable_led_mode(pin_).examine();
        }

        void set_dutycycle(const real_t duty) {
            aw9523_.set_led_current(pin_,int(255 * duty)).examine();
        }
    private:
        AW9523 & aw9523_;
        hal::PinSource pin_;
    };

    
    [[nodiscard]] IResult<> init(const Config & cfg);
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

    [[nodiscard]] IResult<> write_by_index(
        const size_t index, 
        const BoolLevel data) ;
    
    [[nodiscard]] IResult<BoolLevel> read_by_index(const size_t index) ;

    [[nodiscard]] IResult<> set_mode(
        const size_t index, 
        const hal::GpioMode mode) ;

    [[nodiscard]] IResult<> enable_irq_by_index(
        const size_t index, 
        const Enable en = EN);

    [[nodiscard]] IResult<> enable_led_mode(const hal::PinMask pin);

    [[nodiscard]] IResult<> set_led_current_limit(const CurrentLimit limit);

    [[nodiscard]] IResult<> set_led_current(
        const hal::PinMask pin, 
        const uint8_t current);
    
    [[nodiscard]] IResult<> validate();

    AW9523Pwm operator [](const size_t index){
        return AW9523Pwm(*this, hal::PinSource(1 << index));
    }
private:
    hal::I2cDrv i2c_drv_;
    hal::PinMask buf_mask_ = hal::PinMask(0);

    static constexpr RegAddress get_dim_addr(const size_t idx){
        switch(idx){
            case 0:  return RegAddress::DimP00;
            case 1:  return RegAddress::DimP01;
            case 2:  return RegAddress::DimP02;
            case 3:  return RegAddress::DimP03;
            case 4:  return RegAddress::DimP04;
            case 5:  return RegAddress::DimP05;
            case 6:  return RegAddress::DimP06;
            case 7:  return RegAddress::DimP07;
            case 8:  return RegAddress::DimP10;
            case 9:  return RegAddress::DimP11;
            case 10: return RegAddress::DimP12;
            case 11: return RegAddress::DimP13;
            case 12: return RegAddress::DimP14;
            case 13: return RegAddress::DimP15;
            case 14: return RegAddress::DimP16;
            case 15: return RegAddress::DimP17;
            default: __builtin_unreachable();
        }
    }

    static constexpr bool is_index_valid(const size_t idx){
        return idx < 16;
    }


    template<typename T>
    [[nodiscard]] IResult<> write_reg(const RegCopy<T> & reg){
        if(const auto res = write_reg(T::ADDRESS, reg.as_val());
            res.is_err()) return Err(res.unwrap_err());
        reg.apply();
        return Ok();
    }

    template<typename T>
    [[nodiscard]] IResult<> read_reg(T & reg){
        return read_reg(T::ADDRESS, reg.as_ref());
    }

    [[nodiscard]] IResult<> write_reg(const RegAddress addr, const uint16_t data);

    template<typename T>
    [[nodiscard]] IResult<> read_reg(const RegAddress addr, T & data){
        if(const auto res = i2c_drv_.read_reg(uint8_t(addr), data, LSB);
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }

    [[nodiscard]] IResult<> write_by_mask(const hal::PinMask mask) {
        buf_mask_ = mask;
        return write_reg(RegAddress::Out, buf_mask_.as_u16());
    }

    [[nodiscard]] IResult<hal::PinMask> read_mask() {
        if(const auto res = read_reg(input_reg);
            res.is_err()) return Err(res.unwrap_err());
        return Ok(input_reg.mask);
    }
};

};