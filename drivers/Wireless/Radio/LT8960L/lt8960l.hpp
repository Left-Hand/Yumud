//这个驱动已经在初级试用阶段

#pragma once

#include "core/utils/Result.hpp"

#include "details/LT8960L_prelude.hpp"
#include "details/LT8960L_Regs.hpp"

namespace ymd::drivers{

class LT8960L final:public details::LT8960L_Prelude{
public:

    using Error = LT8960L_Transport::Error;

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

        [[nodiscard]] static constexpr Config from_default(){
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

    explicit LT8960L(const hal::Gpio & scl, const hal::Gpio & sda):
        transport_(scl, sda){;}


    IResult<> set_rf_freq_mhz(const size_t freq);

    IResult<> set_syncword(const uint32_t syncword);

    IResult<> set_retrans_time(const uint8_t times);

    IResult<> enable_autoack(const Enable en);

    IResult<> init(const Config & cfg);

    IResult<> init_rf();
    
    IResult<> init_ble(const Power power);

    IResult<> set_preamble_bytes(const size_t bytes);

    IResult<> set_syncword_bytes(const size_t bytes);
    
    IResult<> set_trailer_bits(const size_t bits);

    IResult<> set_pack_type(const PacketType ptype);

    IResult<> reset();
    IResult<> wakeup(){return reset();}

    IResult<> sleep();
    
    IResult<> wake();

    IResult<> validate();

    IResult<> set_tx_power(const Power power);

    IResult<size_t> transmit_rf(std::span<const uint8_t> buf);

    IResult<size_t> receive_rf(std::span<uint8_t> buf);

    IResult<size_t> transmit_ble(std::span<const uint8_t> buf);
    
    IResult<size_t> receive_ble(std::span<uint8_t> buf);

    IResult<> set_datarate(LT8960L::DataRate rate);

    IResult<> enable_gain_weaken(const Enable en);

    IResult<bool> is_pkt_ready();
    
    IResult<bool> is_receiving();

    IResult<bool> is_rst_done();

    IResult<> set_rf_channel(const Channel ch){
        now_channel_ = ch; return Ok();}

    IResult<> enable_use_hw_pkt(const Enable en){
        use_hw_pkt_ = (en == EN); return Ok();}

    IResult<> tick();
    

    IResult<> write(const std::span<const uint8_t> pbuf);

    IResult<> read(const std::span<uint8_t> pbuf);

    [[nodiscard]] size_t available() const;

    [[nodiscard]] size_t pending() const;

    IResult<> on_interrupt();

    IResult<> set_syncword_tolerance_bits(const size_t bits);

private:

    Regs regs_ = {};
    States states_ = {};

    LT8960L_Transport transport_;

    bool use_hw_pkt_ = false;//使能通过监听引脚判断数据是否发送完成

    DataRate datarate_;
    bool on_ble_ = false;

    Channel now_channel_ = Channel(0);

    IResult<> write_reg(const RegAddr address, const uint16_t reg){
        return transport_.write_reg(address, reg);
    }

    template<typename T>
    IResult<> write_reg(const RegCopy<T> & reg){
        if(const auto res = write_reg(T::REG_ADDR, reg.to_bits());
            res.is_err()) return Err(res.unwrap_err());
        reg.apply();
        return Ok();
    }


    IResult<> read_reg(const RegAddr address, uint16_t & reg){
        return transport_.read_reg(address, reg);
    }


    template<typename ... Ts>
    IResult<> read_regs(Ts & ... reg) {
        return (transport_.read_reg(reg.REG_ADDR, reg.as_bits_mut()) | ...);
    }

    template<typename T>
    IResult<> read_reg(T & reg){
        return transport_.read_reg(reg.REG_ADDR, reg.as_bits_mut());
    }


    IResult<size_t> write_fifo(std::span<const uint8_t> buf){
        return transport_.write_burst(Regs::R16_Fifo::REG_ADDR, buf);
    }

    IResult<size_t> read_fifo(std::span<uint8_t> buf);

    IResult<> set_pa_current(const uint8_t nowent);

    IResult<> set_pa_gain(const uint8_t gain);

    IResult<> enable_analog(Enable en);

    IResult<> change_carrier(const Channel ch);

    IResult<> set_rf_channel(const Channel ch, const bool tx, const bool rx);
    IResult<> set_rf_channel_and_enter_tx(const Channel ch){
        return set_rf_channel(ch, 1, 0);
    }
    IResult<> set_rf_channel_and_enter_rx(const Channel ch){
        return set_rf_channel(ch, 0, 1);
    }
    IResult<> set_rf_channel_and_exit_tx_rx(const Channel ch){
        return set_rf_channel(ch, 0, 0);
    }

    IResult<> enter_tx(){
        return set_rf_channel(now_channel_, 1, 0);
    }

    IResult<> enter_rx(){
        return set_rf_channel(now_channel_, 0, 1);
    }

    IResult<> exit_tx_rx(){
        return set_rf_channel(now_channel_, 0, 0);
    }

    IResult<> clear_fifo_write_and_read_ptr();

    IResult<> ensure_correct_0x08();

    IResult<> begin_receive();

    IResult<> begin_transmit();

    IResult<> start_listen_pkt();

    IResult<> set_radio_mode(const bool isRx);

    IResult<> set_brclk_sel(const BrclkSel brclkSel);

    IResult<> clear_fifo_write_ptr();

    IResult<> clear_fifo_read_ptr();

    IResult<bool> is_rfsynth_locked();

    IResult<> set_fifo_full_threshold(const size_t thd);

    IResult<> set_fifo_empty_threshold(const size_t thd);
};

}
