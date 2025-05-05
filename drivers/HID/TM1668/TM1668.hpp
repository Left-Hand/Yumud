//这个驱动还未完成
//这个驱动还未测试

//TM1668是天微半导体的一款LED矩阵驱动/按键矩阵扫描芯片


#pragma once

#include "core/utils/Result.hpp"

#include "hal/gpio/gpio_intf.hpp"
#include "hal/bus/i2c/i2cdrv.hpp"

namespace ymd::drivers{
class TM1668_Error{
    public:
        enum Kind:uint8_t{
            I2cError,
            Unspecified = 0xff,
        };
        constexpr TM1668_Error(const Kind kind): kind_(kind){;}
        constexpr TM1668_Error(const hal::HalResult err)
            {

            }
        constexpr Kind kind() const {return kind_;}
        constexpr bool operator ==(const TM1668_Error &rhs) const {return kind_ == rhs.kind_;}
    private:
        Kind kind_ = Kind::Unspecified;
    };
}


namespace ymd::custom{
    template<typename T>
    struct result_converter<T, drivers::TM1668_Error, hal::HalResult> {
        scexpr Result<T, drivers::TM1668_Error> convert(const hal::HalResult res){
            if constexpr(std::is_void_v<T>)
                if(res.is_ok()) return Ok();

            return Err(drivers::TM1668_Error(res.unwrap_err())); 
        }
    };
}


namespace ymd::drivers{
class TM1668_Phy final{
public:

    TM1668_Phy(hal::I2c & i2c, hal::GpioIntf & scb_io):
        i2c_(i2c), scb_io_(scb_io){;}
    
    using Error = TM1668_Error;

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

    struct DataCommand{
        const uint8_t __resv1__:1 = 0;
        uint8_t write_or_read:1;
        uint8_t addr_inc_en:1;
        const uint8_t __resv2__:5 = 0b01001;
    };

    static_assert(sizeof(DataCommand) == 1);

    struct AddressCommand{
        static constexpr AddressCommand from_idx(const uint8_t idx){
            return {uint8_t(0xC0 | idx)};
        }

        constexpr uint8_t as_u8() const {return addr;}
        const uint8_t addr;
    };

    static_assert(sizeof(AddressCommand) == 1);

    struct DisplayCommand{
        PulseWidth pulse_width:3;
        uint8_t display_on:1;
        const uint8_t __resv2__:4 = 0b1000;

        constexpr uint8_t as_u8() const {return std::bit_cast<uint8_t>(*this);}
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
        const auto guard = i2c_.create_guard();
        uint32_t buf;
        TODO();
        // auto res = i2c_.begin(uint8_t(DataCommand::READ_KEY))
        //     .then([&](){
        //         return i2c_.read(buf, ACK);
        //     })
        // ;
        
        // if (res.wrong()) return Err(Error(res));

        return Ok<KeyEvent>(KeyEvent::from_u8(buf));
    }
private:
    Result<void, Error> write_display_cmd(const DisplayCommand cmd){
        // return write_u8x2(uint8_t(DataCommand::MODE_CMD), cmd.as_u8());
        return Ok();
    }
private:
    hal::I2c & i2c_;
    hal::GpioIntf & scb_io_;

    void set_scb(){scb_io_.set();}
    void clr_scb(){scb_io_.clr();}

    Result<void, Error> write_u8x2(const uint8_t payload1, const uint8_t payload2){
        const auto guard = i2c_.create_guard();

        auto res = i2c_
            .begin(hal::LockRequest{payload1, 0})
            .then([&](){return i2c_.write(payload2);})
        ;

        return Result<void, Error>(res);
    }
};

class TM1668{
public:
    using Error = TM1668_Phy::Error;
    using DisplayCommand = TM1668_Phy::DisplayCommand;
    using KeyEvent = TM1668_Phy::KeyEvent;

    static constexpr auto NAME = "TM1668";

    TM1668(TM1668_Phy && phy):
        phy_(std::move(phy)){;}

    class Display final{
    public:
        Display(TM1668 & owner):
            owner_(owner){;}

        std::span<uint8_t, 4> into_iter(){
            return std::span(buf_);
        }

        void turn_on(){
            display_command_.display_on = true;
        }

        void turn_off(){
            display_command_.display_on = false;
        }


    private:
        DisplayCommand display_command_;
        std::array<uint8_t, 4> buf_;

        TM1668 & owner_;
    };

    class Keyboard final{
    public:
        Keyboard(TM1668 & owner):
            owner_(owner){;}

    private:
        TM1668 & owner_;
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
    TM1668_Phy phy_;
    Display display{*this};
};

}

