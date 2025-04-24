//这个驱动还未完成
//这个驱动还未测试

//TM1650是天微半导体的一款LED矩阵驱动/按键矩阵扫描芯片


#pragma once

#include "core/utils/Result.hpp"
#include "hal/bus/i2c/i2cdrv.hpp"

namespace ymd::drivers{
class TM1650_Error{
    public:
        enum Kind:uint8_t{
            I2cError,
            Unspecified = 0xff,
        };
        constexpr TM1650_Error(const Kind kind): kind_(kind){;}
        constexpr TM1650_Error(const BusError err)
            {

            }
        constexpr Kind kind() const {return kind_;}
        constexpr bool operator ==(const TM1650_Error &rhs) const {return kind_ == rhs.kind_;}
    private:
        Kind kind_ = Kind::Unspecified;
    };
}


namespace ymd::custom{
    template<typename T>
    struct result_converter<T, drivers::TM1650_Error, BusError> {
        scexpr Result<T, drivers::TM1650_Error> convert(const BusError berr){
            using Error = drivers::TM1650_Error;
            using BusError = BusError;
            
            if constexpr(std::is_void_v<T>)
                if(berr.ok()) return Ok();
            
            Error err = [](const BusError berr_){
                switch(berr_.type){
                    default: return Error::Unspecified;
                }
            }(berr);

            return Err(err); 
        }
    };
}
    

namespace ymd::drivers{
class TM1650_Phy final:private hal::ProtocolBusDrv<hal::I2c> {
public:
    using Error = TM1650_Error;

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

    // template<typename Dummy = void>
    // class DataCommand{
    // public:
    //     static constexpr DataCommand<Dummy> MODE_CMD = DataCommand<Dummy>(0b0100'1000); 
    //     static constexpr DataCommand<Dummy> READ_KEY = DataCommand<Dummy>(0b0100'1001); 

    //     constexpr uint8_t as_u8() const {return data_;}
    // private:    
    //     constexpr DataCommand(const uint8_t data): data_(data){;}

    //     uint8_t data_;
    // };

    // static_assert(sizeof(DataCommand<void>) == 1);

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
        const auto guard = create_guard();
        uint32_t buf;
        auto res = bus_.begin(uint8_t(DataCommand::READ_KEY))
            .then([&](){
                return bus_.read(buf, ACK);
            })
        ;
        
        if (res.wrong()) return Err(Error(res));

        return Ok<KeyEvent>(KeyEvent::from_u8(buf));
    }
private:
    Result<void, Error> write_display_cmd(const DisplayCommand cmd){
        return write_u8x2(uint8_t(DataCommand::MODE_CMD), cmd.as_u8());
    }

    Result<void, Error> write_u8x2(const uint8_t payload1, const uint8_t payload2){
        const auto guard = create_guard();

        auto res = bus_.begin(payload1)
            .then([&](){
                return bus_.write(payload2);
            })
        ;

        return Result<void, Error>(res);
    }
};

class TM1650{
public:
    using Error = TM1650_Phy::Error;
    using DisplayCommand = TM1650_Phy::DisplayCommand;
    using KeyEvent = TM1650_Phy::KeyEvent;

    static constexpr auto NAME = "TM1650";

    TM1650(TM1650_Phy phy):
        phy_(phy){;}

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

