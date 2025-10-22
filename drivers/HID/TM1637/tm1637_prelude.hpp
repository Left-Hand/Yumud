//这个驱动已经完成
//这个驱动正在测试
    //显示已经完成测试 需要进一步添加业务语法糖
    //按键扫描还未测试 目前尚未有硬件平台

//TM1637是天微半导体的一款LED矩阵驱动/按键矩阵扫描芯片


#pragma once

#include "core/utils/Result.hpp"
#include "core/utils/Errno.hpp"
#include "core/string/string_view.hpp"
#include "core/stream/ostream.hpp"

#include "hal/bus/i2c/i2cdrv.hpp"
#include "drivers/HID/Event.hpp"

#include "core/magic/enum_traits.hpp"

namespace ymd::drivers{


// TM1637 常用基础工具
struct _TM1637_Prelude{
    enum class Error_Kind:uint8_t{
        KeyFormatWrong,
        DisplayLengthTooLong,
        IndexOutOfRange,
        DutyGreatThanOne,
        DutyLessThanZero
    };

    DEF_ERROR_SUMWITH_HALERROR(Error, Error_Kind)
    DEF_FRIEND_DERIVE_DEBUG(Error_Kind)

    template<typename T = void>
    using IResult = Result<T, Error>;

    static constexpr uint8_t CGRAM_BEGIN_ADDR = 0;
    static constexpr uint8_t CGRAM_MAX_LEN = 6;

    class PulseWidth{
    public:
        enum class Kind:uint8_t{
            _1_16   = 0b000,
            _2_16   = 0b001,
            _4_16   = 0b010,
            _10_16  = 0b011,
            _11_16  = 0b100,
            _12_16  = 0b101,
            _13_16  = 0b110,
            _14_16  = 0b111
        };
        constexpr PulseWidth(const Kind kind):kind_(kind){;}

        using enum Kind;
        static constexpr Option<PulseWidth> from_duty(const real_t duty){
            if(duty < DUTY_TABLE[0]) return None;
            else return Some(PulseWidth(duty2kind(duty)));
        }

        constexpr auto kind() const { return kind_; }
    private:
        Kind kind_;

        static constexpr std::array DUTY_TABLE = {
            real_t(1.0 / 16),
            real_t(2.0 / 16),
            real_t(4.0 / 16),
            real_t(10.0 / 16),
            real_t(11.0 / 16),
            real_t(12.0 / 16),
            real_t(13.0 / 16),
            real_t(14.0 / 16)
        };

        static constexpr Kind duty2kind(const real_t duty){
            const auto it = std::lower_bound(DUTY_TABLE.begin(), DUTY_TABLE.end(), duty);
            const auto idx = std::distance(DUTY_TABLE.begin(), it);
            return std::bit_cast<Kind>(uint8_t(idx));
        }
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
        PulseWidth::Kind pulse_width:3;
        uint8_t display_en:1;
        const uint8_t __resv2__:4 = 0b1000;

        constexpr uint8_t as_u8() const {return std::bit_cast<uint8_t>(*this);}
    };

    static_assert(sizeof(DisplayCommand) == 1);
};

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


//TM1637 物理层接口
//由于TM1637使用了另类的I2C接口 故特化
class TM1637_Phy final:public _TM1637_Prelude{
public:
    TM1637_Phy(hal::Gpio & scl_gpio, hal::Gpio & sda_gpio):
        scl_gpio_(scl_gpio),
        sda_gpio_(sda_gpio)
    {;}


    IResult<> write_sram(const std::span<const uint8_t> pbuf);

    Result<uint8_t, Error> read_key();
    IResult<> set_display(const DisplayCommand);
    IResult<> set_data_mode(const DataCommand);
private:
    IResult<> write_byte(const uint8_t data);
    IResult<> read_byte(uint8_t & data);
    IResult<> wait_ack();
    IResult<> iic_start(const uint8_t data);
    IResult<> iic_stop();
    hal::Gpio & scl_gpio_;
    hal::Gpio & sda_gpio_;
};


}