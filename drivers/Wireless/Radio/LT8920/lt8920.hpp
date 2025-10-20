#pragma once

#include "core/io/regs.hpp"
#include "core/utils/Result.hpp"
#include "core/utils/errno.hpp"


#include "hal/bus/i2c/i2cdrv.hpp"
#include "hal/bus/spi/spidrv.hpp"


namespace ymd::drivers{

struct LT8920_Prelude{
    enum class Error_Kind:uint8_t{
        WrongChipId,
        NoAvailablePhy
    };

    DEF_ERROR_SUMWITH_HALERROR(Error, Error_Kind)
    DEF_FRIEND_DERIVE_DEBUG(Error_Kind)

    template<typename T = void>
    using IResult = Result<T, Error>;

    enum class PacketType:uint8_t{
        NRZLaw = 0,Manchester,Line8_10,Interleave
    };

    enum class TrailerBits:uint8_t{
        _4 = 0,_6,_8,_10,_12,_14,_16,_18
    };

    enum class SyncWordBits:uint8_t{
        _16, _32, _48, _64
    };

    enum class PreambleBits:uint8_t{
        _1 = 0, _2, _3, _4, _5, _6, _7, _8
    };

    enum class BrclkSel:uint8_t{
        Low = 0,Div1, Div2, Div4, Div8,
        Mhz1, Mhz12
    };

    enum class DataRate:uint8_t{
        Mbps1 = 0x01, Kbps250 = 0x04, Kbps125 = 0x08, Kbps62_5 = 0x10
    };

    enum class State:uint8_t{
        OFF = 0,
        IDLE,
        SLEEP,
        VCO_WAIT,
        VCO_SEL,
        TX_PKT,
        TX_WAIT_ACK,
        RX_PKT,
        RX_WAIT_ACK
    };

    enum class Role:uint8_t{
        IDLE,
        BROADCASTER,
        LISTENER
    };
};


struct LT8920_Regs:public LT8920_Prelude{
    #include "lt8920_regs.hpp"

    // uint16_t __resv1__[2];
    // REG3 RO
    R16_RfSynthLock rf_synth_lock_reg = {};
    // uint16_t __resv2__[2];
    // REG6 RO
    R16_RawRssi raw_rssi_reg = {};
    // REG7 
    R16_RfConfig rf_config_reg = {};
    // uint16_t __resv3__;
    // REG9
    R16_PaConfig pa_config_reg = {};

    // REG10
    R16_OscEnable osc_enable_reg = {};

    // REG11
    R16_RssiPdn rssi_pdn_reg = {};
    // uint16_t __resv4__[11];
    // REG23
    R16_DeviceID device_id_reg = {};
    // uint16_t __resv5__[5];
    // REG29 RO

    R16_AutoCali auto_cali_reg = {};
    // uint16_t __resv6__[2];
    // REG32 RO
    R16_Config1 config1_reg = {};
    // REG33 RO
    R16_Delay1 delay1_reg = {};
    // REG34
    R16_Delay2 delay2_reg = {};
    // REG35
    R16_Config2 config2_reg = {};

    R16_SyncWord sync_word_regs[4];

    // REG40
    R16_Threshold threshold_reg = {};

    // REG41
    R16_Config3 config3_reg = {};

    // REG42
    R16_RxConfig rx_config_reg = {};

    // REG43
    R16_RssiConfig rssi_config_reg = {};

    // REG44
    R16_DataRate data_rate_reg = {};
    // uint16_t __resv7__[3];
    // REG48 RO
    R16_Flag flag_reg = {};

    // uint16_t __resv8__[3];
    // REG52
    R16_FifoPtr fifo_ptr_reg = {};
};


class LT8920 final: public LT8920_Regs{
public:
    LT8920(const hal::SpiDrv & spi_drv) : 
        spi_drv_(spi_drv) {;}
    LT8920(hal::SpiDrv && spi_drv) : 
        spi_drv_(std::move(spi_drv)) {;}
    LT8920(Some<hal::Spi *> spi, const hal::SpiSlaveRank index) : 
        spi_drv_(hal::SpiDrv(spi, index)) {;}

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
    [[nodiscard]] IResult<> set_rf_freq_m_hz(const uint freq);
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
        return Ok(State::IDLE == state);}
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

    State state = State::OFF;
    Role role = Role::IDLE;

    bool first_as_len_en = true;
    bool auto_ack_en = true;

    __no_inline void delayT3();
    __no_inline void delayT5();



    template<typename T>
    [[nodiscard]] IResult<> write_reg(const RegCopy<T> & reg){
        if(const auto res = write_reg(T::ADDRESS, reg.as_val());
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
            if(const auto res = i2c_drv_->write_reg(uint8_t(address), reg, MSB);
                res.is_err()) return Err(res.unwrap_err());
        }
        return Ok();
    }

    template<typename T>
    [[nodiscard]] IResult<> read_reg(T & reg){
        return read_reg(uint8_t(T::ADDRESS), reg.as_ref());
    }


    [[nodiscard]] IResult<> read_reg(const uint8_t address, uint16_t & data){
        if(spi_drv_){
            if(const auto res = spi_drv_->transceive_single(
                reinterpret_cast<uint8_t &>(flag_reg.as_bytes()[0]), 
                uint8_t(address | 0x80), CONT); 
            res.is_err()) return Err(res.unwrap_err());
            if(const auto res = spi_drv_->read_single<uint16_t>(data);
                res.is_err()) return Err(res.unwrap_err());
        }else if(i2c_drv_){
            if(const auto res = i2c_drv_->read_reg(uint8_t(address), data, MSB);
                res.is_err()) return Err(res.unwrap_err());
        }
        return Ok();

    }


    [[nodiscard]] IResult<> write_fifo(std::span<const uint8_t> pbuf);
    [[nodiscard]] IResult<> read_fifo(std::span<uint8_t> pbuf);

    [[nodiscard]] IResult<> update_fifo_status();
};

}