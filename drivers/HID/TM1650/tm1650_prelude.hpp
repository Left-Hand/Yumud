//这个驱动还未完成
//这个驱动还未测试

//TM1650是天微半导体的一款LED矩阵驱动/按键矩阵扫描芯片


#pragma once

#include "core/utils/Result.hpp"
#include "core/utils/Errno.hpp"

#include "hal/gpio/gpio_intf.hpp"
#include "hal/bus/i2c/i2cdrv.hpp"

#include "hal/bus/i2c/i2csw.hpp"



namespace ymd::drivers{

struct [[nodiscard]] KeyEvent{
public:

    constexpr KeyEvent(Option<uint8_t> row, Option<uint8_t> col): 
        row_(row),
        col_(col)
    {;}
    [[nodiscard]] static constexpr KeyEvent from_none() {
        return KeyEvent{None, None};
    }

    [[nodiscard]] static constexpr KeyEvent from_row_and_col(uint8_t row, uint8_t col) {
        return KeyEvent{Some(row), Some(col)};
    }

    [[nodiscard]] constexpr Option<uint8_t> row() const {return row_;}
    [[nodiscard]] constexpr Option<uint8_t> col() const {return col_;}
    
private:


    Option<uint8_t> row_;
    Option<uint8_t> col_;
};


struct TM1650_Prelude{

    enum class Error_Kind:uint8_t{
        I2cError,
        PayloadOverlength,
        Unspecified = 0xff,
    };

    DEF_ERROR_SUMWITH_HALERROR(Error, Error_Kind)

    template<typename T = void>
    using IResult = Result<T, Error>;

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


    enum class DataCommand:uint8_t{
        MODE_CMD = 0b0100'1000,
        READ_KEY = 0b1000'1001
    };

    static_assert(sizeof(DataCommand) == 1);

    struct [[nodiscard]] AddressCommand{
        static constexpr AddressCommand from_idx(const uint8_t idx){
            return {uint8_t(0x68 + (idx << 1))};
        }

        constexpr uint8_t as_bits() const {return addr;}
        const uint8_t addr;
    };

    static_assert(sizeof(AddressCommand) == 1);

    struct [[nodiscard]] DisplayCommand{
        uint8_t display_on:1;
        const uint8_t __resv1__:2 = 0b0;
        uint8_t seg7_else_sge8:1;
        uint8_t lim:3;
        const uint8_t __resv2__:1 = 0b0;

        constexpr uint8_t as_bits() const {return *reinterpret_cast<const uint8_t *>(this);}
    };

    static_assert(sizeof(DisplayCommand) == 1);


    [[nodiscard]] static constexpr KeyEvent key_event_from_bits(const uint8_t data){
        const uint8_t high = data >> 3;
        const uint8_t low = data & 0b111;

        if(low < 4) return {None, None};
        if(high < 8 or high > 14) return {None, None};

        return KeyEvent{Some<uint8_t>(high - 8), Some<uint8_t>(low - 4)};
    }

};


class TM1650_Phy final:public TM1650_Prelude{
private:
    hal::I2cSw i2c_;
public:

    TM1650_Phy(Some<hal::Gpio *> scl_io, Some<hal::Gpio *> sda_io):
        i2c_{scl_io, sda_io}{;}

    IResult<> write_screen(const DisplayCommand cmd, const std::span<const uint8_t, 4> pbuf){
        if(const auto res = write_display_cmd(cmd);
            res.is_err()) return Err(res.unwrap_err());

        for(size_t i = 0; i < pbuf.size(); i++){
            if(const auto res = write_u8x2(
                AddressCommand::from_idx(i).as_bits(),
                pbuf[i]
            ); res.is_err()) return Err(res.unwrap_err());
        }

        return Ok();
    }

    IResult<KeyEvent> read_key(){
        // const auto guard = bus_.create_guard();
        // uint32_t buf;
        // auto res = bus_.begin(uint8_t(DataCommand::READ_KEY))
        //     .then([&](){
        //         return bus_.read(buf, ACK);
        //     })
        // ;
        
        // if (res.wrong()) return Err(Error(res));

        // return Ok<KeyEvent>(KeyEvent::from_bits(buf));

        TODO();
        return Ok(key_event_from_bits(0));
    }
private:
    [[nodiscard]] IResult<> write_display_cmd(const DisplayCommand cmd){
        return write_u8x2(uint8_t(DataCommand::MODE_CMD), cmd.as_bits());
    }

    [[nodiscard]] IResult<> write_u8x2(const uint8_t payload1, const uint8_t payload2){
        const auto guard = i2c_.create_guard();

        TODO();
        // auto res = i2c_.begin(I2cs payload1)
        //     .then([&](){return bus_.write(payload2);})
        // ;

        // return IResult<>(res);
        return Ok();
    }
};


}

