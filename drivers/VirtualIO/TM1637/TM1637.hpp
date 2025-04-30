//这个驱动已经完成
//这个驱动正在测试
    //显示已经完成测试 需要进一步添加业务语法糖
    //按键扫描还未测试 目前尚未有硬件平台

//TM1637是天微半导体的一款LED矩阵驱动/按键矩阵扫描芯片


// 快速上手:

// (src/testbench/i2c/tm1637_tb.cpp)
// 示例代码:

// TM1637 tm1637{hal::portB[0], hal::portB[1]};

// while(true){
//     const auto res = 
//         tm1637.set(0, SegDisplayer::digit_to_seg(millis() / 1000))
//         | tm1637.set(1, SegDisplayer::digit_to_seg(millis() / 100))
//         | tm1637.set(2, SegDisplayer::digit_to_seg(millis() / 10))
//         | tm1637.set(3, SegDisplayer::digit_to_seg(millis() % 10))
//         | tm1637.flush()
//     ;
//     if(res.is_err()) PANIC();
//     DEBUG_PRINTLN(millis(), uint8_t(millis()));
//     delay(20);
// }

#pragma once

#include "core/utils/Result.hpp"
#include "core/utils/Errno.hpp"
#include "core/string/StringView.hpp"

#include "hal/bus/i2c/i2cdrv.hpp"

namespace ymd::drivers{

// DisplayBuf: 显示缓存,能够区分脏数据
template<typename T, size_t N>
class DisplayBuf {
public:
    struct Element {
        T value;
        bool dirty;

        Element(T v, bool d) : value(v), dirty(d) {}
    };

    class Iterator {
    public:
        Iterator(DisplayBuf& owner, size_t index) : owner_(owner), index_(index) {}

        Element operator*() const {
            return {owner_.buf_[index_], owner_.buf_[index_] != owner_.last_buf_[index_]};
        }

        Iterator& operator++() {
            ++index_;
            return *this;
        }

        bool operator!=(const Iterator& other) const {
            return index_ != other.index_;
        }

    private:
        DisplayBuf& owner_;
        size_t index_;
    };

    DisplayBuf() {
        last_buf_.fill(T{});
        buf_.fill(T{});
    }

    Iterator begin() {
        return Iterator(*this, 0);
    }

    Iterator end() {
        return Iterator(*this, N);
    }

    void set(size_t index, T value) {
        if (index < N) {
            buf_[index] = value;
        }
    }

    T get(size_t index) const {
        if (index < N) {
            return buf_[index];
        }
        return T{};
    }

    void flush() {
        last_buf_ = buf_;
    }

    bool changed() const {
        return last_buf_ != buf_;
    }

    std::span<const T, N> to_span() const {
        return std::span<const T, N>(buf_);
    }

private:
    std::array<T, N> last_buf_;
    std::array<T, N> buf_;
};

namespace details{
// TM1637 常用基础工具
struct _TM1637_Collections{
    enum class Error_Kind{
        KeyFormatWrong,
        DisplayLengthTooLong,
        IndexOutOfRange,
    };

    DEF_ERROR_SUMWITH_HALERROR(Error, Error_Kind)

    static constexpr uint8_t CGRAM_BEGIN_ADDR = 0;
    static constexpr uint8_t CGRAM_MAX_LEN = 6;

    enum class PulseWidth:uint8_t{
        _1_16   = 0b000,
        _2_16   = 0b001,
        _4_16   = 0b010,
        _10_16  = 0b011,
        _11_16  = 0b100,
        _12_16  = 0b101,
        _13_16  = 0b110,
        _14_16  = 0b111,
    };



    struct DataCommand{
        const uint8_t __resv1__:1 = 0;
        uint8_t read_key:1;
        uint8_t addr_inc_disen:1;
        const uint8_t __resv2__:5 = 0b01000;

        constexpr uint8_t as_u8() const {return std::bit_cast<uint8_t>(*this);}
    };

    static_assert(sizeof(DataCommand) == 1);

    struct AddressCommand{
        uint8_t addr:3;
        const uint8_t __resv2__:5 = 0b11000;

        constexpr uint8_t as_u8() const {return std::bit_cast<uint8_t>(*this);}
    };

    static_assert(sizeof(AddressCommand) == 1);

    struct DisplayCommand{
        PulseWidth pulse_width:3;
        uint8_t display_en:1;
        const uint8_t __resv2__:4 = 0b1000;

        constexpr uint8_t as_u8() const {return std::bit_cast<uint8_t>(*this);}
    };

    static_assert(sizeof(DisplayCommand) == 1);

    class KeyEvent{
    public:
        constexpr Option<uint8_t> row() const {
            //no key pressed
            if(raw_ == 0xff) return None;

            const uint8_t key = raw_ >> 3;
            switch(key){
                case 0b11101: return Some<uint8_t>(0);
                case 0b01001: return Some<uint8_t>(0);
                case 0b10101: return Some<uint8_t>(0);
                case 0b00101: return Some<uint8_t>(0);
                case 0b11111: return Some<uint8_t>(1);
                case 0b01011: return Some<uint8_t>(1);
                case 0b10111: return Some<uint8_t>(1);
                case 0b00111: return Some<uint8_t>(1);
                case 0b11010: return Some<uint8_t>(2);
                case 0b01010: return Some<uint8_t>(2);
                case 0b10010: return Some<uint8_t>(2);
                case 0b00010: return Some<uint8_t>(2);
                case 0b11110: return Some<uint8_t>(1);
                case 0b01110: return Some<uint8_t>(1);
                case 0b10110: return Some<uint8_t>(1);
                case 0b00110: return Some<uint8_t>(1);
                // default: while(true);
                default: return None;
            }
        }
        constexpr Option<uint8_t> col() const {
            //no key pressed
            if(raw_ == 0xff) return None;

            const uint8_t key = raw_ >> 3;
            switch(key){
                case 0b11101: return Some<uint8_t>(0);
                case 0b01001: return Some<uint8_t>(1);
                case 0b10101: return Some<uint8_t>(2);
                case 0b00101: return Some<uint8_t>(3);
                case 0b11111: return Some<uint8_t>(0);
                case 0b01011: return Some<uint8_t>(1);
                case 0b10111: return Some<uint8_t>(2);
                case 0b00111: return Some<uint8_t>(3);
                case 0b11010: return Some<uint8_t>(0);
                case 0b01010: return Some<uint8_t>(1);
                case 0b10010: return Some<uint8_t>(2);
                case 0b00010: return Some<uint8_t>(3);
                case 0b11110: return Some<uint8_t>(0);
                case 0b01110: return Some<uint8_t>(1);
                case 0b10110: return Some<uint8_t>(2);
                case 0b00110: return Some<uint8_t>(3);
                // default: while(true);
                default: return None;
                // default: return {None, None};
            }
        }

        static constexpr Result<KeyEvent, Error> from_u8(const uint8_t data){
            //low 3bit must be 111
            if((data & 0b111) != 0b111) return Err(Error::KeyFormatWrong);
            return Ok(KeyEvent{uint8_t(data >> 3)});
        }
    private:
        constexpr KeyEvent(uint8_t raw): 
            raw_(raw)
        {;}

        // Option<uint8_t> row_;
        // Option<uint8_t> col_;
        uint8_t raw_;
    };
};
}

//TM1637 物理层接口
//由于TM1637使用了另类的I2C接口 故特化
class TM1637_Phy final:public details::_TM1637_Collections{
public:
    TM1637_Phy(hal::Gpio & scl_gpio, hal::Gpio & sda_gpio):
        scl_gpio_(scl_gpio),
        sda_gpio_(sda_gpio)
    {;}


    Result<void, Error> write_sram(const std::span<const uint8_t> pbuf);

    Result<uint8_t, Error> read_key();
    Result<void, Error> set_display(const DisplayCommand);
    Result<void, Error> set_data_mode(const DataCommand);
private:
    Result<void, Error> write_byte(const uint8_t data);
    Result<void, Error> read_byte(uint8_t & data);
    Result<void, Error> wait_ack();
    Result<void, Error> iic_start(const uint8_t data);
    Result<void, Error> iic_stop();
    hal::Gpio & scl_gpio_;
    hal::Gpio & sda_gpio_;
};

//TM1637本体
class TM1637 final:public details::_TM1637_Collections{
public:
    using Phy = TM1637_Phy;

    TM1637(hal::Gpio & scl_gpio, hal::Gpio & sda_gpio): 
        phy_(TM1637_Phy(scl_gpio, sda_gpio)){;}


    [[nodiscard]]
    Result<void, Error> flush();
    
    [[nodiscard]]
    Result<KeyEvent, Error> read_key();
    [[nodiscard]]
    Result<void, Error> set(const size_t pos, const uint8_t val){
        if(pos > CGRAM_MAX_LEN) return Err(Error::IndexOutOfRange);
        buf_.set(pos, val);
        return Ok();
    }
private:
    Phy phy_;
    using Buf = DisplayBuf<uint8_t, CGRAM_MAX_LEN>;
    Buf buf_;
    bool is_on_display_else_readkey_ = true;

    [[nodiscard]] Result<void, Error> switch_to_display();
    [[nodiscard]] Result<void, Error> switch_to_readkey();
};

//段显示器
struct SegDisplayer final{
    static constexpr std::array<uint8_t, 22> SEG_TABLE = {
        uint8_t(~0xc0),   //0
        uint8_t(~0xf9),   //1
        uint8_t(~0xa4),   //2
        uint8_t(~0xb0),   //3
        uint8_t(~0x99),   //4
        uint8_t(~0x92),   //5
        uint8_t(~0x82),   //6
        uint8_t(~0xf8),   //7
        uint8_t(~0x80),   //8
        uint8_t(~0x90),   //9
        uint8_t(~0x88),   //A
        uint8_t(~0x83),   //B
        uint8_t(~0xc6),   //C
        uint8_t(~0xa1),   //D
        uint8_t(~0x86),   //E
        uint8_t(~0x8e),   //F

        uint8_t(~0x8c),   //P
        uint8_t(~0xc1),   //U
        uint8_t(~0x91),   //Y
        uint8_t(~0x7c),   //L
        uint8_t(~0x00),   //全亮
        uint8_t(~0xff)    //熄灭
    };

    // static constexpr uint8_t SEG_MINUS = 0x4f;
    static constexpr uint8_t SEG_MINUS = 0xC0;

    static constexpr uint8_t DOT = 0x80;
    static constexpr uint8_t MINUS = 0x40;

    static constexpr uint8_t char_to_seg(const char c){
        switch(c){
            case '0' ... '9': return uint8_t(SEG_TABLE[c - '0']);
            case 'A' ... 'F': return uint8_t(SEG_TABLE[c - 'A' + 10]);
            case 'a' ... 'f': return uint8_t(SEG_TABLE[c - 'a' + 10]);
            case 'P': return uint8_t(SEG_TABLE[16]);
            case 'U': return uint8_t(SEG_TABLE[17]);
            case 'Y': return uint8_t(SEG_TABLE[18]);
            case 'L': return uint8_t(SEG_TABLE[19]);
            default: __builtin_abort();
        }
    }

    static constexpr uint8_t digit_to_seg(const uint8_t digit){
        return char_to_seg((digit % 10) + '0');
    }

    [[nodiscard]] static constexpr 
    Result<void, void> render_str(const std::span<uint8_t> context, const StringView str){
        if(str.size() > context.size()) return Err();
        for(uint8_t i = 0; i < context.size(); ++i){
            if(i >= str.size()) break;
            context[i] = char_to_seg(str[i]);
        }
        return Ok();
    }

    [[nodiscard]] static constexpr 
    Result<void, void> render_digit(const std::span<uint8_t> context, const int num){
        const auto len = context.size();
        if(len >= 2){
            context[0] = digit_to_seg(num % 10);
            context[1] = digit_to_seg(num / 10);

        }
        return Ok();
    }
};


}