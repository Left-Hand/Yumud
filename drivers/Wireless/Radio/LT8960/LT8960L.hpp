//这个驱动已经在初级试用阶段

#pragma once

#include "core/utils/Result.hpp"

#include "details/LT8960L_Phy.hpp"
#include "details/LT8960L_Regs.hpp"

namespace ymd::drivers{



class LT8960L{
public:
    static constexpr auto DEFAULT_I2C_ADDR = LT8960L_Phy::DEFAULT_I2C_ADDR;
    static constexpr auto MAX_RX_RETRY = 2;

    using Error = LT8960L_Error;
    using Regs = _LT8960L_Regs;

    using RegAddress = Regs::RegAddress;
    using PacketType = Regs::PacketType;
    using Power = Regs::Power;
    using TrailerBits = Regs::TrailerBits;

    using SyncWordBits = Regs::SyncWordBits;
    using PreambleBits = Regs::PreambleBits;
    using BrclkSel = Regs::BrclkSel;
    using DataRate = Regs::DataRate;
    using Mode = Regs::Mode;

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
        constexpr Channel (const uint8_t ch):ch_(ch){;}

        constexpr Channel (const Channel & other) = default;
        constexpr Channel (Channel && other) = default;
        constexpr Channel & operator = (const Channel & other) = default;
        constexpr Channel & operator = (Channel && other) = default;

        constexpr auto into_code() const {
            return ch_;
        }
    private:    
        uint8_t ch_;
    };


protected:
    Regs regs_ = {};
    States states_ = {};

    LT8960L_Phy phy_;

    bool use_hw_pkt_ = false;//使能通过监听引脚判断数据是否发送完成

    DataRate datarate_;
    bool on_ble_ = false;

    Channel curr_channel_ = Channel(0);

    [[nodiscard]] __fast_inline
    Result<void, Error> write_reg(const RegAddress address, const uint16_t reg){
        return phy_.write_reg(address, reg);
    }


    [[nodiscard]] __fast_inline
    Result<void, Error> read_reg(const RegAddress address, uint16_t & reg){
        return phy_.read_reg(address, reg);
    }


    template<typename ... Ts>
    [[nodiscard]] __fast_inline
    Result<void, Error> write_regs(Ts const & ... reg) {
        return (phy_.write_reg(reg.address, reg.as_val()) | ...);
    }

    template<typename ... Ts>
    [[nodiscard]] __fast_inline
    Result<void, Error> read_regs(Ts & ... reg) {
        return (phy_.read_reg(reg.address, reg.as_ref()) | ...);
    }

    template<typename T>
    [[nodiscard]] __fast_inline
    Result<void, Error> read_reg(T & reg){
        return phy_.read_reg(reg.address, reg);
    }


    [[nodiscard]] __fast_inline
    Result<size_t, Error> write_fifo(std::span<const std::byte> buf){
        return phy_.write_burst(Regs::R16_Fifo::address, buf);
    }

    [[nodiscard]]Result<size_t, Error> read_fifo(std::span<std::byte> buf);

    [[nodiscard]] Result<void, Error> set_pa_current(const uint8_t current);

    [[nodiscard]] Result<void, Error> set_pa_gain(const uint8_t gain);

    [[nodiscard]] Result<void, Error> enable_analog(bool en = true);

    [[nodiscard]] Result<void, Error> change_carrier(const Channel ch);

    [[nodiscard]] Result<void, Error> set_rf_channel(const Channel ch, const bool tx, const bool rx);
    [[nodiscard]] Result<void, Error> set_rf_channel_and_enter_tx(const Channel ch){return set_rf_channel(ch, 1, 0);}
    [[nodiscard]] Result<void, Error> set_rf_channel_and_enter_rx(const Channel ch){return set_rf_channel(ch, 0, 1);}
    [[nodiscard]] Result<void, Error> set_rf_channel_and_exit_tx_rx(const Channel ch){return set_rf_channel(ch, 0, 0);}

    [[nodiscard]] Result<void, Error> enter_tx(){return set_rf_channel(curr_channel_, 1, 0);}
    [[nodiscard]] Result<void, Error> enter_rx(){return set_rf_channel(curr_channel_, 0, 1);}
    [[nodiscard]] Result<void, Error> exit_tx_rx(){return set_rf_channel(curr_channel_, 0, 0);}

    

    [[nodiscard]] Result<void, Error> clear_fifo_write_and_read_ptr();

    [[nodiscard]] Result<void, Error> ensure_correct_0x08();

    [[nodiscard]] Result<void, Error> begin_receive();

    [[nodiscard]] Result<void, Error> begin_transmit();

    [[nodiscard]] Result<void, Error> start_listen_pkt();

    [[nodiscard]] Result<void, Error> set_radio_mode(const bool isRx);

    [[nodiscard]] Result<void, Error> set_brclk_sel(const BrclkSel brclkSel);

    [[nodiscard]] Result<void, Error> clear_fifo_write_ptr();

    [[nodiscard]] Result<void, Error> clear_fifo_read_ptr();

    [[nodiscard]] Result<bool, Error> is_rfsynth_locked();

    [[nodiscard]] Result<void, Error> set_fifo_full_threshold(const uint thd);

    [[nodiscard]] Result<void, Error> set_fifo_empty_threshold(const uint thd);



    
public:

    LT8960L(hal::Gpio & scl, hal::Gpio & sda):
        phy_(scl, sda){;}


    [[nodiscard]] Result<void, Error> set_rf_freq_mhz(const uint freq);

    [[nodiscard]] Result<void, Error> set_syncword(const uint32_t syncword);

    [[nodiscard]] Result<void, Error> set_retrans_time(const uint8_t times);

    [[nodiscard]] Result<void, Error> enable_autoack(const bool en = true);

    [[nodiscard]] Result<void, Error> init(const Power power, uint32_t syncword);

    [[nodiscard]] Result<void, Error> init_rf();
    
    [[nodiscard]] Result<void, Error> init_ble(const Power power);

    [[nodiscard]] Result<void, Error> set_preamble_bytes(const uint bytes);

    [[nodiscard]] Result<void, Error> set_syncword_bytes(const uint bytes);
    
    [[nodiscard]] Result<void, Error> set_trailer_bits(const uint bits);

    [[nodiscard]] Result<void, Error> set_pack_type(const PacketType ptype);

    [[nodiscard]] Result<void, Error> wake();

    [[nodiscard]] Result<void, Error> reset();
    [[nodiscard]] Result<void, Error> wakup(){return reset();}

    [[nodiscard]] Result<void, Error> sleep();

    [[nodiscard]] Result<void, Error> verify();

    [[nodiscard]] Result<void, Error> set_tx_power(const Power power);

    [[nodiscard]] Result<size_t, Error> transmit_rf(std::span<const std::byte> buf);

    [[nodiscard]] Result<size_t, Error> receive_rf(std::span<std::byte> buf);

    [[nodiscard]] Result<size_t, Error> transmit_ble(std::span<const std::byte> buf);
    
    [[nodiscard]] Result<size_t, Error> receive_ble(std::span<std::byte> buf);

    [[nodiscard]] Result<void, Error> set_datarate(LT8960L::DataRate rate);

    [[nodiscard]] Result<void, Error> enable_gain_weaken(const bool en);

    [[nodiscard]] Result<bool, Error> is_pkt_ready();
    
    [[nodiscard]] Result<bool, Error> is_receiving();

    [[nodiscard]] Result<bool, Error> is_rst_done();

    [[nodiscard]] Result<void, Error> wait_pkt_ready(const uint timeout);

    [[nodiscard]] Result<void, Error> wait_rst_done(const uint timeout);

    [[nodiscard]] Result<void, Error> set_rf_channel(const Channel ch){curr_channel_ = ch; return Ok();}

    [[nodiscard]] Result<void, Error> enable_use_hw_pkt(const bool en){use_hw_pkt_ = en; return Ok();}

    [[nodiscard]] Result<void, Error> tick();
    

    [[nodiscard]] Result<void, Error> write(const std::span<const std::byte> pdata);

    [[nodiscard]] Result<void, Error> read(const std::span<std::byte> pdata);

    [[nodiscard]] size_t available() const;

    [[nodiscard]] size_t pending() const;

    [[nodiscard]] Result<void, Error> on_interrupt();

    [[nodiscard]] Result<void, Error> set_syncword_tolerance_bits(const uint bits);
};

}
