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

namespace details{
    enum class TM1650_Error_Kind:uint8_t{
        I2cError,
        PayloadOverlength,
        Unspecified = 0xff,
    };
}

DEF_ERROR_SUMWITH_HALERROR(TM1650_Error, details::TM1650_Error_Kind)

class TM1650_Phy final{
private:
    hal::I2cSw i2c_;
public:
    using Error = TM1650_Error;

    TM1650_Phy(hal::Gpio & scl_io, hal::Gpio & sda_io):
        i2c_{scl_io, sda_io}{;}

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

    struct AddressCommand{
        static constexpr AddressCommand from_idx(const uint8_t idx){
            return {uint8_t(0x68 + (idx << 1))};
        }

        constexpr uint8_t as_u8() const {return addr;}
        const uint8_t addr;
    };

    static_assert(sizeof(AddressCommand) == 1);

    struct DisplayCommand{
        uint8_t display_on:1;
        const uint8_t __resv1__:2 = 0b0;
        uint8_t seg7_else_sge8:1;
        uint8_t lim:3;
        const uint8_t __resv2__:1 = 0b0;

        constexpr uint8_t as_u8() const {return *reinterpret_cast<const uint8_t *>(this);}
    };

    static_assert(sizeof(DisplayCommand) == 1);

    class KeyEvent{
    public:
        constexpr Option<uint8_t> row() const {return row_;}
        constexpr Option<uint8_t> col() const {return col_;}

        static constexpr KeyEvent from_u8(const uint8_t data){
            const uint8_t high = data >> 3;
            const uint8_t low = data & 0b111;

            if(low < 4) return {None, None};
            if(high < 8 or high > 14) return {None, None};

            return KeyEvent{Some<uint8_t>(high - 8), Some<uint8_t>(low - 4)};
        }
    private:
        constexpr KeyEvent(Option<uint8_t> row, Option<uint8_t> col): 
            row_(row),
            col_(col)
        {;}

        Option<uint8_t> row_;
        Option<uint8_t> col_;
    };

    Result<void, Error> write_screen(const DisplayCommand cmd, const std::span<const uint8_t, 4> pbuf){
        auto res = write_display_cmd(cmd);

        for(size_t i = 0; i < pbuf.size(); i++){
            if(res.is_err()) return res;
            res = res | write_u8x2(
                AddressCommand::from_idx(i).as_u8(),
                pbuf[i]
            );
        }

        return Ok();
    }

    Result<KeyEvent, Error> read_key(){
        // const auto guard = bus_.create_guard();
        // uint32_t buf;
        // auto res = bus_.begin(uint8_t(DataCommand::READ_KEY))
        //     .then([&](){
        //         return bus_.read(buf, ACK);
        //     })
        // ;
        
        // if (res.wrong()) return Err(Error(res));

        // return Ok<KeyEvent>(KeyEvent::from_u8(buf));

        TODO();
        return Ok(KeyEvent::from_u8(0));
    }
private:
    Result<void, Error> write_display_cmd(const DisplayCommand cmd){
        return write_u8x2(uint8_t(DataCommand::MODE_CMD), cmd.as_u8());
    }

    Result<void, Error> write_u8x2(const uint8_t payload1, const uint8_t payload2){
        const auto guard = i2c_.create_guard();

        TODO();
        // auto res = i2c_.begin(I2cs payload1)
        //     .then([&](){return bus_.write(payload2);})
        // ;

        // return Result<void, Error>(res);
        return Ok();
    }
};

class TM1650{
public:
    using Error = TM1650_Phy::Error;
    using DisplayCommand = TM1650_Phy::DisplayCommand;
    using KeyEvent = TM1650_Phy::KeyEvent;

    static constexpr auto NAME = "TM1650";

    TM1650(hal::Gpio & scl_io, hal::Gpio & sda_io):
        phy_(scl_io, sda_io){;}

    class Display final{
    public:
        Display(TM1650 & owner):
            owner_(owner){;}

        std::span<uint8_t, 4> into_iter(){
            return std::span(buf_);
        }

        void set_brightness(const uint8_t brightness){
            display_command_.lim = brightness;
        }

        void turn_on(){
            display_command_.display_on = true;
        }

        void turn_off(){
            display_command_.display_on = false;
        }

        void enable_seg7(const bool en = true){
            display_command_.seg7_else_sge8 = en;
        }

    private:
        DisplayCommand display_command_;
        std::array<uint8_t, 4> buf_;

        TM1650 & owner_;
    };

    class Keyboard final{
    public:
        Keyboard(TM1650 & owner):
            owner_(owner){;}

    private:
        TM1650 & owner_;
    };


    Result<void, Error> write_screen(
        const DisplayCommand cmd, 
        const std::span<const uint8_t, 4> pbuf){
        
        return phy_.write_screen(cmd, pbuf);
    }

    Result<KeyEvent, Error> read_key(){
        return phy_.read_key();
    }


private:
    TM1650_Phy phy_;
    Display display{*this};
};

}

