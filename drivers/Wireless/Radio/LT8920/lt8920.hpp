#pragma once

#include "../../hal/bus/spi/spidrv.hpp"
#include "../../hal/bus/i2c/i2cdrv.hpp"
#include <optional>

#define LT8920_DEBUG

#ifdef LT8920_DEBUG
#undef LT8920_DEBUG
#define LT8920_DEBUG(...) DEBUG_PRINTLN(SpecToken::Space, std::hex, ##__VA_ARGS__, "\t|", __PRETTY_FUNCTION__);
#else
#define LT8920_DEBUG(...)
#endif

#pragma pack(push, 1)
#pragma GCC diagnostic ignored "-Wstrict-aliasing"


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

    enum class State{
        OFF,
        IDLE,
        SLEEP,
        VCO_WAIT,
        VCO_SEL,
        TX_PKT,
        TX_WAIT_ACK,
        RX_PKT,
        RX_WAIT_ACK
    } state = State::OFF;

public:
    LT8920(const SpiDrv & _spi_drv) : spi_drv(_spi_drv) {;}
    LT8920(SpiDrv && _spi_drv) : spi_drv(_spi_drv) {;}
    LT8920(Spi & _spi, const uint8_t _index) : spi_drv(SpiDrv(_spi, _index)) {;}

    uint16_t isRfSynthLocked();
    uint8_t getRssi();
    void setRfChannel(const uint8_t ch);
    void setRfFreqMHz(const uint freq);
    void setRadioMode(const uint16_t isRx);
    void setPaCurrent(const uint8_t current);
    void setPaGain(const uint8_t gain);
    void enableRssi(const uint16_t open = true);

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

    void enableTx(bool en);
    void enableRx(bool en);
    void clearFifoWritePtr();
    void clearFifoReadPtr();
    void clearFifoPtr();

    void handleFifoInterrupt();
    void handlePktInterrupt();
    void handleRxTimeoutInterrupt();
 
    bool getFifoStatus();
    bool getPktStatus();

    std::optional<SpiDrv> spi_drv;
    std::optional<I2cDrv> i2c_drv;
    GpioConcept * pkt_status_gpio = nullptr;
    GpioConcept * fifo_status_gpio = nullptr;
    GpioConcept * nrst_gpio = nullptr;

    bool first_as_len_en = true;
    bool auto_ack_en = true;

    #include "lt8920_regs.hpp"

    struct{
        uint16_t __resv1__[2];
        // REG3 RO
        RfSynthLockReg rf_synth_lock_reg;
        uint16_t __resv2__[2];
        // REG6 RO
        RawRssiReg raw_rssi_reg;
        // REG7 
        RfConfigReg rf_config_reg;
        uint16_t __resv3__;
        // REG9
        PaConfigReg pa_config_reg;

        // REG10
        OscEnableReg osc_enable_reg;

        // REG11
        RssiPdnReg rssi_pdn_reg;
        uint16_t __resv4__[11];
        // REG23
        DeviceIDReg device_id_reg;
        uint16_t __resv5__[5];
        // REG29 RO

        AutoCaliReg auto_cali_reg;
        uint16_t __resv6__[2];
        // REG32 RO
        Config1Reg config1_reg;
        // REG33 RO
        Delay1Reg delay1_reg;
        // REG34
        Delay2Reg delay2_reg;
        // REG35
        Config2Reg config2_reg;

        SyncWordReg sync_word_regs[4];

        // REG40
        ThresholdReg threshold_reg;

        // REG41
        Config3Reg config3_reg;

        // REG42
        RxConfigReg rx_config_reg;

        // REG43
        RssiConfigReg rssi_config_reg;

        // REG44
        DataRateReg data_rate_reg;
        uint16_t __resv7__[3];
        // REG48 RO
        FlagReg flag_reg;

        uint16_t __resv8__[3];
        // REG52
        FifoPtrReg fifo_ptr_reg;
    };

    void delayT3(){delayMicroseconds(1);}

    void delayT5(){delayMicroseconds(1);}

    void writeReg(const RegAddress address, const uint16_t reg);
    void readReg(const RegAddress address, uint16_t & reg);
    void writeFifo(const uint8_t * data, const size_t len);
    void readFifo(uint8_t * data, const size_t len);

    void updateFifoStatus();
};

#pragma pack(pop)
#pragma GCC diagnostic pop