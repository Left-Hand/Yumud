#pragma once

#include "lt8920_prelude.hpp"

namespace ymd::drivers{

class LT8920_TransportIntf: public LT8920_Prelude{
public:
    using FlagReg = LT8920_Regset::R16_Flag;
    virtual IResult<> write_reg(FlagReg & flag_reg, const uint8_t reg_addr, const uint16_t reg_val) = 0;
    virtual IResult<> read_reg(FlagReg & flag_reg, const uint8_t reg_addr, uint16_t &reg_val) = 0;

    virtual IResult<> write_fifo(std::span<const uint8_t> bytes) = 0;
    virtual IResult<> read_fifo(std::span<uint8_t> bytes) = 0;
    virtual IResult<> update_fifo_status(FlagReg & flag_reg) = 0;
};


class LT8920 final: public LT8920_Prelude{
public:
    explicit LT8920(LT8920_TransportIntf & transport) : 
        transport_(transport) {;}

    IResult<> bind_nrst_gpio(hal::GpioIntf & gpio){
        nrst_gpio = &gpio;
        return Ok();
    }
    IResult<> bind_pkt_gpio(hal::GpioIntf & gpio){
        pkt_status_gpio = &gpio;
        return Ok();
    }

    IResult<bool> is_rf_synth_locked();
    IResult<uint8_t> get_rssi();
    IResult<> set_rf_channel(const uint8_t ch);
    IResult<> set_rf_freq_mhz(const uint32_t freq);
    IResult<> start_listen(){return set_role(Role::LISTENER);}
    IResult<> set_pa_current(const uint8_t current);
    IResult<> set_pa_gain(const uint8_t gain);
    IResult<> enable_rssi(const uint16_t open = true);
    IResult<> reset();

    IResult<> set_brclk_sel(const BrclkSel brclkSel);

    IResult<> set_sync_word_bitsgth(const SyncWordBits len);
    IResult<> set_retrans_time(const uint8_t times);

    IResult<> enable_auto_cali(const uint16_t open);
    IResult<> enable_auto_ack(const Enable en);
    IResult<> enable_crc(const Enable en);
    IResult<> init();
    IResult<> validate();
    IResult<> set_sync_word(const uint64_t syncword);
    IResult<> set_err_bits_tolerance(uint8_t errbits);
    IResult<> set_datarate(const DataRate dr);
    IResult<bool> received_ack();


    IResult<> write_block(std::span<const uint8_t> bytes);
    IResult<> read_block(std::span<uint8_t> bytes);

    IResult<> tick();

    IResult<bool> is_idle(){
        return Ok(State::IDLE == state_);}
private:
    IResult<> set_role(const Role _role);
    IResult<> clear_fifo_write_ptr();
    IResult<> clear_fifo_read_ptr();
    IResult<> clear_fifo_ptr();

    IResult<> on_fifo_interrupt();
    IResult<> on_pkt_interrupt();
    IResult<> on_rx_timeout_interrupt();

    IResult<bool> get_fifo_status();
    IResult<bool> get_pkt_status();

    LT8920_TransportIntf & transport_;
    LT8920_Regset regs_ = {};

    Option<hal::GpioIntf &> pkt_status_gpio = None;
    Option<hal::GpioIntf &> fifo_status_gpio = None;
    Option<hal::GpioIntf &> nrst_gpio = None;

    State state_ = State::OFF;
    Role role_ = Role::IDLE;

    bool first_as_len_en_ = true;
    bool auto_ack_en_ = true;

    template<typename T>
    IResult<> write_reg(const RegCopy<T> & reg){
        if(const auto res = write_reg(T::REG_ADDR, reg.to_bits());
            res.is_err()) return Err(res.unwrap_err());
        reg.apply();
        return Ok();
    }

    IResult<> write_reg(const uint8_t reg_addr, const uint16_t reg_val){
        return transport_.write_reg(regs_.flag_reg, reg_addr, reg_val);
    }


    IResult<> read_reg(const uint8_t reg_addr, uint16_t & reg_val){
        return transport_.read_reg(regs_.flag_reg, reg_addr, reg_val);
    }

    template<typename T>
    IResult<> read_reg(T & reg){
        return read_reg(uint8_t(T::REG_ADDR), reg.as_bits_mut());
    }




    IResult<> write_fifo(std::span<const uint8_t> bytes);
    IResult<> read_fifo(std::span<uint8_t> bytes);

    IResult<> update_fifo_status();
};


struct LT8920_SpiTransport final : public LT8920_TransportIntf{
private:
    hal::SpiDrv spi_drv_;
public:
    explicit LT8920_SpiTransport(const hal::SpiDrv & spi_drv) : 
        spi_drv_(spi_drv) {;}
    explicit LT8920_SpiTransport(hal::SpiDrv && spi_drv) : 
        spi_drv_(std::move(spi_drv)) {;}
    explicit LT8920_SpiTransport(Some<hal::Spi *> spi, const hal::SpiSlaveRank rank) : 
        spi_drv_(hal::SpiDrv(spi, rank)) {;}

    IResult<> write_reg(FlagReg & flag_reg, const uint8_t reg_addr, const uint16_t reg_val);

    IResult<> read_reg(FlagReg & flag_reg, const uint8_t reg_addr, uint16_t & reg_val);


    IResult<> write_fifo(std::span<const uint8_t> bytes){
        if(const auto res = spi_drv_.write_single<uint8_t>(uint8_t(50), CONT); 
            res.is_err()) return Err(res.unwrap_err());
        if(const auto res = spi_drv_.write_burst<uint8_t>(bytes);
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }

    IResult<> read_fifo(std::span<uint8_t> bytes){
        if(const auto res = spi_drv_.write_single<uint8_t>(uint8_t(50 | 0x80), CONT); 
            res.is_err()) return Err(res.unwrap_err());
        if(const auto res = spi_drv_.read_burst<uint8_t>(bytes);
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }

    IResult<> update_fifo_status(FlagReg & flag_reg){
        (void)flag_reg;
        // return spi_drv_->transceive_single(flag_reg.as_bits_mut(), flag_reg.reg_addr);
        TODO();
    }

private:
    [[maybe_unused]] static void delayT3(){clock::delay(1us);}

    [[maybe_unused]] static void delayT5(){clock::delay(1us);}


};

struct LT8920_I2cTransport final : public LT8920_TransportIntf{
private:
    hal::I2cDrv i2c_drv_;
public:
    explicit LT8920_I2cTransport(const hal::I2cDrv & i2c_drv) : 
        i2c_drv_(i2c_drv) {;}
    explicit LT8920_I2cTransport(hal::I2cDrv && i2c_drv) : 
        i2c_drv_(std::move(i2c_drv)) {;}


    IResult<> write_reg(FlagReg & flag_reg, const uint8_t reg_addr, const uint16_t reg_val){
        (void)flag_reg;
        if(const auto res = i2c_drv_.write_reg(uint8_t(reg_addr), reg_val, std::endian::big);
            res.is_err()) return Err(res.unwrap_err());

        return Ok();
    }


    IResult<> read_reg(FlagReg & flag_reg, const uint8_t reg_addr, uint16_t & reg_val){
        (void)flag_reg;
        if(const auto res = i2c_drv_.read_reg(uint8_t(reg_addr), reg_val, std::endian::big);
            res.is_err()) return Err(res.unwrap_err());
        return Ok();

    }

    IResult<> write_fifo(std::span<const uint8_t> bytes){
        if(const auto res = i2c_drv_.write_burst(uint8_t(50) , bytes);
            res.is_err()) return Err(res.unwrap_err());
        return Ok(); 
    }

    IResult<> read_fifo(std::span<uint8_t> bytes){
        if(const auto res = i2c_drv_.read_burst(uint8_t(50), bytes);
            res.is_err()) return Err(res.unwrap_err());
        return Ok();

    }

    IResult<> update_fifo_status(FlagReg & flag_reg){
        if(const auto res = read_reg(flag_reg, flag_reg.REG_ADDR, flag_reg.as_bits_mut());
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }
};

}