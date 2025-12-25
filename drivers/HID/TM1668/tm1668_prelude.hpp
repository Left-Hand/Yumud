//这个驱动还未完成
//这个驱动还未测试

//TM1668是天微半导体的一款LED矩阵驱动/按键矩阵扫描芯片


#pragma once

#include "core/utils/Result.hpp"
#include "core/utils/errno.hpp"

#include "hal/gpio/gpio_intf.hpp"
#include "hal/bus/i2c/i2cdrv.hpp"

namespace ymd::drivers{

struct TM1668_Prelude{
    enum class Error_Kind{

    };

    DEF_ERROR_SUMWITH_HALERROR(Error, Error_Kind)

    template<typename T = void>
    using IResult = Result<T, Error>;
};

class TM1668_Transport final:public TM1668_Prelude{
public:

    explicit TM1668_Transport(hal::I2cBase & i2c, hal::GpioIntf & scb_io):
        i2c_(i2c), scb_io_(scb_io){;}
    
    enum class PulseWidth:uint8_t{
        _1_16 = 0,
        _2_16,
        _4_16,
        _10_16,
        _11_16,
        _12_16,
        _13_16,
        _14_16,
    };

    struct [[nodiscard]] DataCommand{
        const uint8_t __resv1__:1 = 0;
        uint8_t write_or_read:1;
        uint8_t addr_inc_en:1;
        const uint8_t __resv2__:5 = 0b01001;
    };

    static_assert(sizeof(DataCommand) == 1);

    struct [[nodiscard]] AddressCommand{
        static constexpr AddressCommand from_idx(const uint8_t idx){
            return {uint8_t(0xC0 | idx)};
        }

        constexpr uint8_t to_u8() const {return addr;}
        const uint8_t addr;
    };

    static_assert(sizeof(AddressCommand) == 1);

    struct [[nodiscard]] DisplayCommand{
        PulseWidth pulse_width:3;
        uint8_t display_on:1;
        const uint8_t __resv2__:4 = 0b1000;

        constexpr uint8_t to_u8() const {return std::bit_cast<uint8_t>(*this);}
    };

    static_assert(sizeof(DisplayCommand) == 1);

    struct [[nodiscard]] KeyCode{
    public:
        constexpr Option<uint8_t> row() const {return row_;}
        constexpr Option<uint8_t> col() const {return col_;}

        static constexpr KeyCode from_u8(const uint8_t data){
            const uint8_t high = data >> 3;
            const uint8_t low = data & 0b111;

            if(low < 4) return {None, None};
            if(high < 8 or high > 14) return {None, None};

            return KeyCode{Some<uint8_t>(high - 8), Some<uint8_t>(low - 4)};
        }
    private:
        constexpr KeyCode(Option<uint8_t> row, Option<uint8_t> col): 
            row_(row),
            col_(col)
        {;}

        Option<uint8_t> row_;
        Option<uint8_t> col_;
    };

    IResult<> write_screen(const DisplayCommand cmd, const std::span<const uint8_t, 4> pbuf){
        if(const auto res = write_display_cmd(cmd);
            res.is_err()) return Err(res.unwrap_err());

        for(size_t i = 0; i < pbuf.size(); i++){
            if(const auto res = write_u8x2(
                AddressCommand::from_idx(i).to_u8(),
                pbuf[i]
            ); res.is_err()) return Err(res.unwrap_err());
        }

        return Ok();
    }

    Result<KeyCode, Error> read_key(){
        const auto guard = i2c_.create_guard();
        uint32_t buf;
        TODO();
        // auto res = i2c_.borrow(uint8_t(DataCommand::READ_KEY))
        //     .then([&](){
        //         return i2c_.read(buf, ACK);
        //     })
        // ;
        
        // if (res.wrong()) return Err(Error(res));

        return Ok<KeyCode>(KeyCode::from_u8(buf));
    }
private:
    IResult<> write_display_cmd(const DisplayCommand cmd){
        // return write_u8x2(uint8_t(DataCommand::MODE_CMD), cmd.to_u8());
        return Ok();
    }
private:
    hal::I2cBase & i2c_;
    hal::GpioIntf & scb_io_;

    void set_scb(){scb_io_.set_high();}
    void clr_scb(){scb_io_.set_low();}

    IResult<> write_u8x2(const uint8_t payload1, const uint8_t payload2){
        const auto guard = i2c_.create_guard();

        auto res = i2c_
            .borrow(hal::I2cSlaveAddrWithRw::from_8bits(payload1))
            .then([&](){return i2c_.write(payload2);})
        ;

        if(res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }
};


}

