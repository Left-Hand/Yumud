//这个驱动已经在初级试用阶段

#pragma once

#include "core/utils/Result.hpp"

#include "details/LT8960L_prelude.hpp"
#include "details/LT8960L_Regs.hpp"

namespace ymd::drivers{

class LT8960L final:public details::LT8960L_Prelude{
public:

    using Error = LT8960L_Phy::Error;

    using Regs = _LT8960L_Regs;
public:
    struct Config{
        Power tx_power;
        uint8_t preamble_bytes;
        uint8_t syncword_bytes;
        PacketType packet_type;
        uint8_t retrans_times;
        uint32_t syncword;
        DataRate datarate;

        static constexpr Config from_default(){
            return Config{
                .tx_power = Power::_8_Db,
                .preamble_bytes = 3,
                .syncword_bytes = 4,
                .packet_type = PacketType::Manchester,
                .retrans_times = 3,
                .syncword = 0xAABBCCDD,
                .datarate = DataRate::_62_5K
            };
        }
    };

    LT8960L(Some<hal::Gpio *> scl, Some<hal::Gpio *> sda):
        phy_(scl, sda){;}


    [[nodiscard]] IResult<> set_rf_freq_mhz(const size_t freq);

    [[nodiscard]] IResult<> set_syncword(const uint32_t syncword);

    [[nodiscard]] IResult<> set_retrans_time(const uint8_t times);

    [[nodiscard]] IResult<> enable_autoack(const Enable en);

    [[nodiscard]] IResult<> init(const Config & cfg);

    [[nodiscard]] IResult<> init_rf();
    
    [[nodiscard]] IResult<> init_ble(const Power power);

    [[nodiscard]] IResult<> set_preamble_bytes(const size_t bytes);

    [[nodiscard]] IResult<> set_syncword_bytes(const size_t bytes);
    
    [[nodiscard]] IResult<> set_trailer_bits(const size_t bits);

    [[nodiscard]] IResult<> set_pack_type(const PacketType ptype);

    [[nodiscard]] IResult<> reset();
    [[nodiscard]] IResult<> wakeup(){return reset();}

    [[nodiscard]] IResult<> sleep();
    
    [[nodiscard]] IResult<> wake();

    [[nodiscard]] IResult<> validate();

    [[nodiscard]] IResult<> set_tx_power(const Power power);

    [[nodiscard]] IResult<size_t> transmit_rf(std::span<const uint8_t> buf);

    [[nodiscard]] IResult<size_t> receive_rf(std::span<uint8_t> buf);

    [[nodiscard]] IResult<size_t> transmit_ble(std::span<const uint8_t> buf);
    
    [[nodiscard]] IResult<size_t> receive_ble(std::span<uint8_t> buf);

    [[nodiscard]] IResult<> set_datarate(LT8960L::DataRate rate);

    [[nodiscard]] IResult<> enable_gain_weaken(const Enable en);

    [[nodiscard]] IResult<bool> is_pkt_ready();
    
    [[nodiscard]] IResult<bool> is_receiving();

    [[nodiscard]] IResult<bool> is_rst_done();

    [[nodiscard]] IResult<> wait_pkt_ready(const size_t timeout);

    [[nodiscard]] IResult<> wait_rst_done(const size_t timeout);

    [[nodiscard]] IResult<> set_rf_channel(const Channel ch){
        curr_channel_ = ch; return Ok();}

    [[nodiscard]] IResult<> enable_use_hw_pkt(const Enable en){
        use_hw_pkt_ = en == EN; return Ok();}

    [[nodiscard]] IResult<> tick();
    

    [[nodiscard]] IResult<> write(const std::span<const uint8_t> pbuf);

    [[nodiscard]] IResult<> read(const std::span<uint8_t> pbuf);

    [[nodiscard]] size_t available() const;

    [[nodiscard]] size_t pending() const;

    [[nodiscard]] IResult<> on_interrupt();

    [[nodiscard]] IResult<> set_syncword_tolerance_bits(const size_t bits);

private:

    Regs regs_ = {};
    States states_ = {};

    LT8960L_Phy phy_;

    bool use_hw_pkt_ = false;//使能通过监听引脚判断数据是否发送完成

    DataRate datarate_;
    bool on_ble_ = false;

    Channel curr_channel_ = Channel(0);

    [[nodiscard]] __fast_inline
    IResult<> write_reg(const RegAddress address, const uint16_t reg){
        return phy_.write_reg(address, reg);
    }

    template<typename T>
    [[nodiscard]] __fast_inline
    IResult<> write_reg(const RegCopy<T> & reg){
        if(const auto res = write_reg(reg.address, reg.as_val());
            res.is_err()) return Err(res.unwrap_err());
        reg.apply();
        return Ok();
    }


    [[nodiscard]] __fast_inline
    IResult<> read_reg(const RegAddress address, uint16_t & reg){
        return phy_.read_reg(address, reg);
    }


    template<typename ... Ts>
    [[nodiscard]] __fast_inline
    IResult<> read_regs(Ts & ... reg) {
        return (phy_.read_reg(reg.address, reg.as_ref()) | ...);
    }

    template<typename T>
    [[nodiscard]] __fast_inline
    IResult<> read_reg(T & reg){
        return phy_.read_reg(reg.address, reg.as_ref());
    }


    [[nodiscard]] __fast_inline IResult<size_t> write_fifo(std::span<const uint8_t> buf){
        return phy_.write_burst(Regs::R16_Fifo::address, buf);
    }

    [[nodiscard]] IResult<size_t> read_fifo(std::span<uint8_t> buf);

    [[nodiscard]] IResult<> set_pa_current(const uint8_t current);

    [[nodiscard]] IResult<> set_pa_gain(const uint8_t gain);

    [[nodiscard]] IResult<> enable_analog(Enable en = EN);

    [[nodiscard]] IResult<> change_carrier(const Channel ch);

    [[nodiscard]] IResult<> set_rf_channel(const Channel ch, const bool tx, const bool rx);
    [[nodiscard]] IResult<> set_rf_channel_and_enter_tx(const Channel ch){return set_rf_channel(ch, 1, 0);}
    [[nodiscard]] IResult<> set_rf_channel_and_enter_rx(const Channel ch){return set_rf_channel(ch, 0, 1);}
    [[nodiscard]] IResult<> set_rf_channel_and_exit_tx_rx(const Channel ch){return set_rf_channel(ch, 0, 0);}

    [[nodiscard]] IResult<> enter_tx(){return set_rf_channel(curr_channel_, 1, 0);}
    [[nodiscard]] IResult<> enter_rx(){return set_rf_channel(curr_channel_, 0, 1);}
    [[nodiscard]] IResult<> exit_tx_rx(){return set_rf_channel(curr_channel_, 0, 0);}

    [[nodiscard]] IResult<> clear_fifo_write_and_read_ptr();

    [[nodiscard]] IResult<> ensure_correct_0x08();

    [[nodiscard]] IResult<> begin_receive();

    [[nodiscard]] IResult<> begin_transmit();

    [[nodiscard]] IResult<> start_listen_pkt();

    [[nodiscard]] IResult<> set_radio_mode(const bool isRx);

    [[nodiscard]] IResult<> set_brclk_sel(const BrclkSel brclkSel);

    [[nodiscard]] IResult<> clear_fifo_write_ptr();

    [[nodiscard]] IResult<> clear_fifo_read_ptr();

    [[nodiscard]] IResult<bool> is_rfsynth_locked();

    [[nodiscard]] IResult<> set_fifo_full_threshold(const size_t thd);

    [[nodiscard]] IResult<> set_fifo_empty_threshold(const size_t thd);
};

}
