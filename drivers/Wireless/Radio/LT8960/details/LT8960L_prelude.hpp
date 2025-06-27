#pragma once

#include "hal/bus/i2c/i2csw.hpp"
#include "hal/bus/i2c/i2cdrv.hpp"

#include "hal/gpio/gpio_intf.hpp"
#include "core/utils/Errno.hpp"

namespace ymd::drivers{

namespace details{

struct LT8960L_Prelude{
    static constexpr auto MAX_RX_RETRY = 2;

    enum Error_Kind:uint8_t{
        TransmitTimeout,
        PacketOverlength,
        ChipIdMismatch,
        ReceiveTimeout,
        InvalidState,
        Unspecified = 0xff
    };

    
    enum class PacketType:uint8_t{
        NrzLaw = 0, 
        Manchester,
        Line8_10,
        Interleave
    };

    enum class Power:uint8_t{
        _n19_Db,
        _n13_Db,
        _n9_Db,
        _n7_Db,
        _n4_Db,
        _n1_5_Db,
        _0_2_Db,
        _3_4_Db,
        _5_Db,
        _6_Db,
        _8_Db,
    };

    enum class TrailerBits:uint8_t{
        _4, _6, _8, _10, _12, _14, _16, _18
    };

    enum class SyncWordBits:uint8_t{
        _16, _32, _48, _64
    };

    enum class PreambleBits:uint8_t{
        _1, _2, _3, _4, _5, _6, _7, _8
    };

    enum class BrclkSel:uint8_t{
        Low, Div1, Div2, Div4, Div8,
        Mhz1, Mhz12
    };

    enum class DataRate:uint8_t{
        _1M = 0, _250K, _125K, _62_5K
    };

    enum class Mode:uint8_t{
        Rx, Tx, CarrierWave, Sleep
    };

    DEF_ERROR_SUMWITH_HALERROR(Error, Error_Kind)

    static constexpr uint8_t DEFAULT_I2C_ADDR = 0x1A;


    template<typename T = void>
    using IResult = Result<T, Error>;



    struct States{
    public:
        enum Kind : uint8_t{
            Idle,

            Transmitting,
            TransmitFailed,
            TransmitSucceed,

            Receiving,
            ReceiveFailed,
            ReceiveSucceed,

            Sleeping,
            PowerDown
        };

        
    private:
        Kind status_ = Kind::Idle;

        uint8_t timeout_ = 0;
    public:

        States & operator = (const Kind status) {
            transition_to(status);
            return *this;
        }

        auto kind() const {return status_;}

        auto & timeout() {return timeout_;}
        void transition_to(const Kind status);
    };

    class Channel{
    public:
        constexpr explicit Channel (const uint8_t ch):ch_(ch){;}

        constexpr Channel (const Channel & other) = default;
        constexpr Channel (Channel && other) = default;
        constexpr Channel & operator = (const Channel & other) = default;
        constexpr Channel & operator = (Channel && other) = default;

        constexpr auto as_u8() const {
            return ch_;
        }
    private:    
        uint8_t ch_;
    };

    using RegAddress = uint8_t;
};


}

class LT8960L_Phy final:public details::LT8960L_Prelude{
public:


public:
    LT8960L_Phy(hal::Gpio & scl_io, hal::Gpio & sda_io):
        i2c_(hal::I2cSw(scl_io, sda_io)){};

    [[nodiscard]] IResult<> init();

    [[nodiscard]] IResult<> write_reg(uint8_t address, uint16_t data);

    [[nodiscard]] IResult<> read_reg(uint8_t address, uint16_t & data);

    [[nodiscard]] IResult<size_t> read_burst(uint8_t address, std::span<uint8_t> pbuf);

    [[nodiscard]] IResult<size_t> write_burst(uint8_t address, std::span<const uint8_t> pbuf);

    [[nodiscard]] IResult<> start_hw_listen_pkt();

    [[nodiscard]] IResult<bool> check_and_skip_hw_listen_pkt();

    [[nodiscard]] IResult<> wait_pkt_ready(const uint timeout);
private:
    hal::I2cSw i2c_;
    [[nodiscard]] IResult<> _write_reg(uint8_t address, uint16_t data);

    [[nodiscard]] IResult<> _read_reg(uint8_t address, uint16_t & data);
};
}