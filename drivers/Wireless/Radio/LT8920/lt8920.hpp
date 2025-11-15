#pragma once

#include "lt8920_prelude.hpp"

namespace ymd::drivers{


class LT8920 final: public LT8920_Regs{
public:
    explicit LT8920(const hal::SpiDrv & spi_drv) : 
        spi_drv_(spi_drv) {;}
    explicit LT8920(hal::SpiDrv && spi_drv) : 
        spi_drv_(std::move(spi_drv)) {;}
    explicit LT8920(Some<hal::Spi *> spi, const hal::SpiSlaveRank rank) : 
        spi_drv_(hal::SpiDrv(spi, rank)) {;}

    [[nodiscard]] IResult<> bind_nrst_gpio(hal::GpioIntf & gpio){
        nrst_gpio = &gpio;
        return Ok();
    }
    [[nodiscard]] IResult<> bind_pkt_gpio(hal::GpioIntf & gpio){
        pkt_status_gpio = &gpio;
        return Ok();
    }
    [[nodiscard]] IResult<bool> is_rf_synth_locked();
    [[nodiscard]] IResult<uint8_t> get_rssi();
    [[nodiscard]] IResult<> set_rf_channel(const uint8_t ch);
    [[nodiscard]] IResult<> set_rf_freq_mhz(const uint32_t freq);
    [[nodiscard]] IResult<> start_listen(){return set_role(Role::LISTENER);}
    [[nodiscard]] IResult<> set_pa_current(const uint8_t current);
    [[nodiscard]] IResult<> set_pa_gain(const uint8_t gain);
    [[nodiscard]] IResult<> enable_rssi(const uint16_t open = true);
    [[nodiscard]] IResult<> reset();

    [[nodiscard]] IResult<> set_brclk_sel(const BrclkSel brclkSel);

    [[nodiscard]] IResult<> set_sync_word_bitsgth(const SyncWordBits len);
    [[nodiscard]] IResult<> set_retrans_time(const uint8_t times);

    [[nodiscard]] IResult<> enable_auto_cali(const uint16_t open);
    [[nodiscard]] IResult<> enable_auto_ack(const Enable en);
    [[nodiscard]] IResult<> enable_crc(const Enable en);
    [[nodiscard]] IResult<> init();
    [[nodiscard]] IResult<> validate();
    [[nodiscard]] IResult<> set_sync_word(const uint64_t syncword);
    [[nodiscard]] IResult<> set_err_bits_tolerance(uint8_t errbits);
    [[nodiscard]] IResult<> set_data_rate(const DataRate dr);
    [[nodiscard]] IResult<> set_data_rate(const uint32_t dr);
    [[nodiscard]] IResult<bool> received_ack();


    [[nodiscard]] IResult<> write_block(std::span<const uint8_t> pbuf);
    [[nodiscard]] IResult<> read_block(std::span<uint8_t> pbuf);

    [[nodiscard]] IResult<> tick();

    [[nodiscard]] IResult<bool> is_idle(){
        return Ok(State::IDLE == state_);}
protected:
    [[nodiscard]] IResult<> set_role(const Role _role);
    [[nodiscard]] IResult<> clear_fifo_write_ptr();
    [[nodiscard]] IResult<> clear_fifo_read_ptr();
    [[nodiscard]] IResult<> clear_fifo_ptr();

    [[nodiscard]] IResult<> on_fifo_interrupt();
    [[nodiscard]] IResult<> on_pkt_interrupt();
    [[nodiscard]] IResult<> on_rx_timeout_interrupt();
 
    [[nodiscard]] IResult<bool> get_fifo_status();
    [[nodiscard]] IResult<bool> get_pkt_status();

    std::optional<hal::SpiDrv> spi_drv_;
    std::optional<hal::I2cDrv> i2c_drv_;
    hal::GpioIntf * pkt_status_gpio = nullptr;
    hal::GpioIntf * fifo_status_gpio = nullptr;
    hal::GpioIntf * nrst_gpio = nullptr;

    State state_ = State::OFF;
    Role role_ = Role::IDLE;

    bool first_as_len_en_ = true;
    bool auto_ack_en_ = true;

    __no_inline void delayT3();
    __no_inline void delayT5();



    template<typename T>
    [[nodiscard]] IResult<> write_reg(const RegCopy<T> & reg){
        if(const auto res = write_reg(T::ADDRESS, reg.as_bits());
            res.is_err()) return Err(res.unwrap_err());
        reg.apply();
        return Ok();
    }

    [[nodiscard]] IResult<> write_reg(const uint8_t address, const uint16_t reg){
        if(spi_drv_){
            if(const auto res = 
                spi_drv_->transceive_single(
                    reinterpret_cast<uint8_t &>(flag_reg), 
                    uint8_t(address), CONT);
                res.is_err()) return Err(res.unwrap_err());
            delayT3();

            if(const auto res = 
                spi_drv_->write_single<uint16_t>(reg);
                res.is_err()) return Err(res.unwrap_err());
        }else if(i2c_drv_){
            if(const auto res = i2c_drv_->write_reg(uint8_t(address), reg, std::endian::big);
                res.is_err()) return Err(res.unwrap_err());
        }
        return Ok();
    }

    template<typename T>
    [[nodiscard]] IResult<> read_reg(T & reg){
        return read_reg(uint8_t(T::ADDRESS), reg.as_mut_bits());
    }


    [[nodiscard]] IResult<> read_reg(const uint8_t address, uint16_t & data){
        if(spi_drv_){
            if(const auto res = spi_drv_->transceive_single(
                (flag_reg.as_mut_bytes()[0]), 
                uint8_t(address | 0x80), CONT); 
            res.is_err()) return Err(res.unwrap_err());
            if(const auto res = spi_drv_->read_single<uint16_t>(data);
                res.is_err()) return Err(res.unwrap_err());
        }else if(i2c_drv_){
            if(const auto res = i2c_drv_->read_reg(uint8_t(address), data, std::endian::big);
                res.is_err()) return Err(res.unwrap_err());
        }
        return Ok();

    }


    [[nodiscard]] IResult<> write_fifo(std::span<const uint8_t> pbuf);
    [[nodiscard]] IResult<> read_fifo(std::span<uint8_t> pbuf);

    [[nodiscard]] IResult<> update_fifo_status();
};

}