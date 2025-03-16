#pragma once

#include "drivers/device_defs.h"
#include <optional>


namespace ymd::drivers{

class LT8920{
public:
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

public:
    LT8920(const hal::SpiDrv & spi_drv) : 
        spi_drv_(spi_drv) {;}
    LT8920(hal::SpiDrv && spi_drv) : 
        spi_drv_(std::move(spi_drv)) {;}
    LT8920(hal::Spi & spi, const uint8_t index) : 
        spi_drv_(hal::SpiDrv(spi, index)) {;}

    void bindNrstGpio(hal::GpioIntf & gpio){nrst_gpio = &gpio;}
    void bindPktGpio(hal::GpioIntf & gpio){pkt_status_gpio = &gpio;}
    uint16_t isRfSynthLocked();
    uint8_t getRssi();
    void setRfChannel(const uint8_t ch);
    void setRfFreqMHz(const uint freq);
    void startListen(){setRole(Role::LISTENER);}
    void setPaCurrent(const uint8_t current);
    void setPaGain(const uint8_t gain);
    void enableRssi(const uint16_t open = true);
    void reset();

    void setBrclkSel(const BrclkSel brclkSel);

    void setSyncWordBitsgth(const SyncWordBits len);
    void setRetransTime(const uint8_t times);

    void enableAutoCali(const uint16_t open);
    void enableAutoAck(const bool en = true);
    void enableCrc(const bool en = true);
    void init();
    bool verify();
    void setSyncWord(const uint64_t syncword);
    void setErrBitsTolerance(uint8_t errbits);
    void setDataRate(const DataRate dr);
    void setDataRate(const uint32_t dr);
    bool receivedAck();


    void writeBlock(const uint8_t * data, const uint8_t len);
    void readBlock(uint8_t * data, const uint8_t len);

    void tick();

    bool isIdle(){return State::IDLE == state;}
protected:
    void setRole(const Role _role);
    void clearFifoWritePtr();
    void clearFifoReadPtr();
    void clearFifoPtr();

    void onFifoInterrupt();
    void onPktInterrupt();
    void onRxTimeoutInterrupt();
 
    bool getFifoStatus();
    bool getPktStatus();

    std::optional<hal::SpiDrv> spi_drv_;
    std::optional<hal::I2cDrv> i2c_drv_;
    hal::GpioIntf * pkt_status_gpio = nullptr;
    hal::GpioIntf * fifo_status_gpio = nullptr;
    hal::GpioIntf * nrst_gpio = nullptr;

    State state = State::OFF;
    Role role = Role::IDLE;

    bool first_as_len_en = true;
    bool auto_ack_en = true;

    #include "lt8920_regs.hpp"

    // uint16_t __resv1__[2];
    // REG3 RO
    RfSynthLockReg rf_synth_lock_reg = {};
    // uint16_t __resv2__[2];
    // REG6 RO
    RawRssiReg raw_rssi_reg = {};
    // REG7 
    RfConfigReg rf_config_reg = {};
    // uint16_t __resv3__;
    // REG9
    PaConfigReg pa_config_reg = {};

    // REG10
    OscEnableReg osc_enable_reg = {};

    // REG11
    RssiPdnReg rssi_pdn_reg = {};
    // uint16_t __resv4__[11];
    // REG23
    DeviceIDReg device_id_reg = {};
    // uint16_t __resv5__[5];
    // REG29 RO

    AutoCaliReg auto_cali_reg = {};
    // uint16_t __resv6__[2];
    // REG32 RO
    Config1Reg config1_reg = {};
    // REG33 RO
    Delay1Reg delay1_reg = {};
    // REG34
    Delay2Reg delay2_reg = {};
    // REG35
    Config2Reg config2_reg = {};

    SyncWordReg sync_word_regs[4];

    // REG40
    ThresholdReg threshold_reg = {};

    // REG41
    Config3Reg config3_reg = {};

    // REG42
    RxConfigReg rx_config_reg = {};

    // REG43
    RssiConfigReg rssi_config_reg = {};

    // REG44
    DataRateReg data_rate_reg = {};
    // uint16_t __resv7__[3];
    // REG48 RO
    FlagReg flag_reg = {};

    // uint16_t __resv8__[3];
    // REG52
    FifoPtrReg fifo_ptr_reg = {};

    void delayT3(){delayMicroseconds(1);}

    void delayT5(){delayMicroseconds(1);}

    BusError writeReg(const RegAddress address, const uint16_t reg);
    BusError readReg(const RegAddress address, uint16_t & reg);
    BusError writeFifo(const uint8_t * data, const size_t len);
    BusError readFifo(uint8_t * data, const size_t len);

    BusError updateFifoStatus();
};

}