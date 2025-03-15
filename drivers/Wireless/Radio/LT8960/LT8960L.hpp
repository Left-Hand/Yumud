//这个驱动还在推进状态

#pragma once

#include "drivers/device_defs.h"
#include "sys/utils/Result.hpp"


#define DEF_R16(T, name)\
static_assert(sizeof(T) == 2 and std::has_unique_object_representations_v<T>, "x must be 16bit register");\
T name = {};\



template<typename T>
class PtrLikeRef{
public:
    PtrLikeRef(T * pobj):obj_(*pobj) {}
    PtrLikeRef(std::nullptr_t) = delete;
    PtrLikeRef(PtrLikeRef &&) = default;
    T * operator ->() {return &obj_;}
    T * operator &() {return &obj_;}
    T & operator *() {return obj_;}
private:
    T & obj_;
};

template<typename T>
class PtrLikeVase{

public:
    PtrLikeVase(const T & obj):obj_(obj) {}
    PtrLikeVase(T && obj):obj_(std::move(obj)) {}
    PtrLikeVase(PtrLikeRef<T> ref):obj_(*ref) {}

    PtrLikeVase(const PtrLikeVase &) = delete;
    PtrLikeVase(PtrLikeVase &&) = default;
    T * operator ->() {return &obj_;}
private:
    T obj_;
};

namespace ymd::drivers{

class LT8960L{
public:
    static constexpr uint8_t default_i2c_addr = 0x1A;
    using Error = BusError;
    
    enum class PacketType:uint8_t{
        NRZLaw = 0, 
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

    class Channel{
    public:
        Channel (const uint8_t ch):
            ch_(ch){;}

        auto as_code() const {
            return ch_;
        }
    private:    
        const uint8_t ch_;
    };
protected:
    using RegAddress = uint8_t;


    struct R16_RfSynthLock:public Reg16<>{
        static constexpr RegAddress address = 0x03;
        uint16_t i2c_soft_rstn:1;//软件复位标志
        uint16_t __resv1__ :8;
        uint16_t fifo_flag_txrx:1;//fifo状态指示
        uint16_t __resv2__ :2;
        uint16_t synthLocked:1;//RF 频率综合器锁定标志位
        uint16_t pkt_flag_txrx:1;//包状态指示
        uint16_t __resv3__ :2;
    };DEF_R16(R16_RfSynthLock, rf_synthlock_reg)

    struct R16_RfConfig:public Reg16<>{
        static constexpr RegAddress address = 7;
        
        uint16_t rfChannelNo :7;//设定 RF 频道，空中频率为：f=2402+ RF_PLL_CH_NO
        uint16_t rxEn:1;//使芯片进入 TX状态，1 有效
        uint16_t txEn:1;//使芯片进入 RX 状态，1 有效
        uint16_t __resv__ :7;
    };DEF_R16(R16_RfConfig, rf_config_reg)
    
    struct R16_PaConfig:public Reg16<>{
        static constexpr RegAddress address = 9;

        uint16_t __resv1__ :7;
        uint16_t paGain:4;//PA增益控制
        uint16_t __resv2__ :1;
        uint16_t paCurrent:4;//PA电流控制
    };DEF_R16(R16_PaConfig, pa_config_reg)
    
    struct R16_FuncConf:public Reg16<>{
        static constexpr RegAddress address = 15;

        uint16_t sda_pullup_sel:1;
        uint16_t cw_mode:1;
        uint16_t rx_bpf1_gn:5;
        uint16_t __resv__:3;
        uint16_t mixer_lp:1;
        uint16_t ble_mode_set:1;
        uint16_t miss_byte:3;
        uint16_t ble_mode:1;
    };DEF_R16(R16_FuncConf, func_conf_reg)
    
    struct R16_FreqOffs:public Reg16<>{
        static constexpr RegAddress address = 0x1c;

        uint16_t offs:14;
        uint16_t __resv__:2;
    };DEF_R16(R16_FreqOffs, freq_offs_reg)


    struct R16_Config1:public Reg16<>{
        static constexpr RegAddress address = 0x20;

        uint16_t __resv1__ :1;
        BrclkSel brclkSel:3;//时钟选择
        uint16_t __resv2__ :2;
        PacketType packetType:2;//包类型
        uint16_t trailerLen:3;//尾缀码长度
        SyncWordBits syncWordLen :2;//同步字比特数
        uint16_t preambleLen :3;//数据载荷长度
    };DEF_R16(R16_Config1, config1_reg)


    struct R16_Config2:public Reg16<>{
        static constexpr RegAddress address = 0x23;

        uint16_t scramableData :7;//Scramble data 的种子，收发两边必须一致
        uint16_t misoTri :1; //当 SPI_SS=1 时，MISO 保持三态/低阻
        uint16_t retransTimes:4;//在 auto-ack 功能开启时，最多的重发次数。设为 3 时，为重发 2 次
        uint16_t brclkOnSleep:1;//在 sleep mode 开启晶体振荡器耗电但能快速启动
        uint16_t __resv__ :1;
        uint16_t sleepMode:1;//进入 sleep mode，晶体关闭，保持 LDO 工作（寄存器值将保留）当 SPI_SS 为低时，芯片将重新工作
        uint16_t powerDown:1;//先关闭晶体振荡器，再关闭 LDO。（寄存器值将丢失）
    };DEF_R16(R16_Config2, config2_reg)

    struct R16_SyncWord0:public Reg16<>{
        static constexpr RegAddress address = 0x24;
        uint8_t word[2];
    };DEF_R16(R16_SyncWord0, sync_word0_reg);

    struct R16_SyncWord1:public Reg16<>{
        static constexpr RegAddress address = 0x27;
        uint8_t word[2];
    };DEF_R16(R16_SyncWord1, sync_word1_reg);

    struct R16_Threshold:public Reg16<>{
        static constexpr RegAddress address = 0x28;
        uint16_t syncWordThreshold:6;//认为 SYNCWORD 为正确的阈值 07 表示可以错 6bits，01 表示 0bit 可以错 0bits
        uint16_t fifoFullThreshold:5;//认为 FIFO 为满的阈值
        uint16_t fifoEmptyThreshold:5;//认为 FIFO 为空的阈值
    };DEF_R16(R16_Threshold, threshold_reg)
    
    struct R16_Config3:public Reg16<>{
        static constexpr RegAddress address = 0x29;
        uint16_t crcInitalData:8;//CRC 计算初始值。
        uint16_t __resv1__ :2;
        uint16_t pktFifoPolarity:1; //PKT flag, FIFO flag 低有效.
        uint16_t autoAck:1;//当接收到数据，自动回 ACK 或者 NACK
        uint16_t fwTermTx :1;//1: 当 FIFO 的读指针和写指针相等时，LT8960L 将关闭发射。
        uint16_t packLengthEN:1;//1: 第一字节表示 payload 的长度 如要写 8 个 byte 有效字节，那第一个字节应写 8，总长 9
        uint16_t __resv2__ :1;
        uint16_t crcOn:1;//开启 CRC
    };DEF_R16(R16_Config3, config3_reg)

    struct R16_RxConfig:public Reg16<>{
        static constexpr RegAddress address = 0x2A;
        uint16_t auot_rx_ack_time:8;//等待 RX_ACK 的时间，1 表示 1uS
        uint16_t wakeup_tim :2;
        uint16_t scanRssiChNo:6;//RSSI 扫描的信道数量，RSSI 值将保留到 FIFO 中
    };DEF_R16(R16_RxConfig, rx_config_reg)

    struct R16_DataRate:public Reg16<>{
        static constexpr RegAddress address = 0x2C;
        uint16_t __resv__ :8;
        uint16_t dataRate:8;//透传速率
    };DEF_R16(R16_DataRate, data_rate_reg)

    struct R16_ModemOption:public Reg16<>{
        static constexpr RegAddress address = 0x2D;
        uint16_t __resv__ :8;
        uint16_t dataRate:8;//透传速率
    };DEF_R16(R16_ModemOption, modem_option_reg)

    struct R16_ChiIndex:public Reg16<>{
        static constexpr RegAddress address = 0x2E;
        uint16_t __resv__ :8;
        uint16_t chi_index:6;//透传速率
        uint16_t __resv2__ :2;
    };DEF_R16(R16_ChiIndex, chi_index_reg)

    struct R16_Flag:public Reg16<>{
        static constexpr RegAddress address = 0x30;
        uint16_t __resv__ :5;
        uint16_t fifoFlag:1;
        uint16_t pktFlag:1;
        uint16_t syncWordRecved:1;//表示收到 syncword，只在接收时有效。跳出接收状态时，为 0
        uint16_t framerFlag:6;//Framer 状态
        uint16_t __resv2__:1;//FEC23 错误标志位
        uint16_t crcErrorFlag:1;//CRC 错误标志位
    };DEF_R16(R16_Flag, flag_reg)

    struct R16_Fifo:public Reg16<>{
        static constexpr RegAddress address = 0x32;
        uint16_t data;
    };DEF_R16(R16_Fifo, fifo_reg)

    struct R16_FifoPtr:public Reg16<>{
        static constexpr RegAddress address = 0x34;
        //FIFO 读指针
        //当使用 auto-ack 功能时，此位可以做为标志位。
        //当 PKT 拉高后，读此寄存器，如果为 0，即收到 ack。
        //如果不为 0，而是发射总 byte 数+1，即没收到 ack。
        uint16_t fifoReadPtr:6;
        uint16_t __resv1__:1;

        uint16_t clearReadPtr:1;//清空 RX FIFO 指针为 0..但不清空 RX FIFO 中的数据
        uint16_t fifoWritePtr:6;//FIFO 写指针
        uint16_t __resv2__:1;
        uint16_t clearWritePtr:1;//清空 TX FIFO 指针为 0，但不清空 TX FIFO 中的数据
    };DEF_R16(R16_FifoPtr, fifo_ptr_reg)


    struct R16_I2cOper:public Reg16<>{
        static constexpr RegAddress address = 0x38;
        uint16_t wakeup_i2c:1;
        uint16_t soft_rst:1;
        uint16_t __resv__:14;
    };DEF_R16(R16_I2cOper, i2c_oper_reg)

    struct DevDriver{
    public:
        DevDriver(const hal::I2cDrv & i2c_drv):i2c_drv_(i2c_drv){}
        DevDriver(hal::I2cDrv && i2c_drv):i2c_drv_(std::move(i2c_drv)){}

        [[nodiscard]] Result<size_t, Error> writeBurst(const RegAddress address, std::span<const std::byte> buf);

        [[nodiscard]] Result<size_t, Error> readBurst(const RegAddress address, std::span<std::byte> buf);

        [[nodiscard]] Result<void, Error> writeReg(const RegAddress address, const uint16_t reg);

        [[nodiscard]] Result<void, Error> readReg(const RegAddress address, uint16_t & reg);

        [[nodiscard]] Result<void, Error> verify();
    private:
        hal::I2cDrv i2c_drv_;
    };

    DevDriver dev_drv_;

    hal::GpioIntf * p_packet_status_gpio = nullptr;
    hal::GpioIntf * p_fifo_status_gpio = nullptr;

    __no_inline void delayT3();
    __no_inline void delayT5();


    [[nodiscard]] __fast_inline
    Result<void, Error> writeReg(const RegAddress address, const uint16_t reg){
        return dev_drv_.writeReg(address, reg);
    }


    [[nodiscard]] __fast_inline
    Result<void, Error> readReg(const RegAddress address, uint16_t & reg){
        return dev_drv_.readReg(address, reg);
    }


    template<typename ... Ts>
    [[nodiscard]] __fast_inline
    Result<void, Error> writeRegs(Ts const & ... reg) {
        return (dev_drv_.writeReg(reg.address, reg.as_val()) | ...);
    }

    template<typename ... Ts>
    [[nodiscard]] __fast_inline
    Result<void, Error> readRegs(Ts & ... reg) {
        return (dev_drv_.readReg(reg.address, reg.as_ref()) | ...);
    }

    template<typename T>
    [[nodiscard]] __fast_inline
    Result<void, Error> readReg(T & reg){
        return dev_drv_.readReg(reg.address, reg);
    }

    [[nodiscard]] __fast_inline
    Result<size_t, Error> writeFifo(std::span<const std::byte> buf){
        return dev_drv_.writeBurst(R16_Fifo::address, buf);
    }

    [[nodiscard]] Result<void, Error> fillFifo(const uint16_t data, const size_t len);


    [[nodiscard]] __fast_inline
    Result<size_t, Error> readBurst(std::span<std::byte> buf){
        return dev_drv_.readBurst(R16_Fifo::address, buf);
    }

    [[nodiscard]]
    Result<void, Error> setPaCurrent(const uint8_t current);

    [[nodiscard]]
    Result<void, Error> setPaGain(const uint8_t gain);

    [[nodiscard]] Result<void, Error> change0x38();
    [[nodiscard]] Result<void, Error> enableAnalog(bool en = true);
    [[nodiscard]] Result<void, Error> changeCarrier(const Channel ch);

    [[nodiscard]] Result<void, Error> setRfChannel(const Channel ch, const bool tx, const bool rx);
    [[nodiscard]] Result<void, Error> setRfChannelAndIntoTx(const Channel ch){return setRfChannel(ch, 1, 0);}
    [[nodiscard]] Result<void, Error> setRfChannelAndIntoRx(const Channel ch){return setRfChannel(ch, 0, 1);}

    
public:


    LT8960L(const hal::I2cDrv & i2c_drv):dev_drv_(i2c_drv){;}
    LT8960L(hal::I2cDrv && i2c_drv):dev_drv_(std::move(i2c_drv)){;}
    LT8960L(hal::I2c * bus, const uint8_t i2c_addr = default_i2c_addr):
        dev_drv_(hal::I2cDrv(*bus, i2c_addr)){;}


    [[nodiscard]] Result<bool, Error> getPktStatus();

    [[nodiscard]] Result<bool, Error> isRfSynthLocked();

    [[nodiscard]] Result<void, Error> setRfFreqMHz(const uint freq);

    [[nodiscard]] Result<void, Error> setRadioMode(const bool isRx);

    [[nodiscard]] Result<void, Error> setBrclkSel(const BrclkSel brclkSel);

    [[nodiscard]] Result<void, Error> clearFifoWritePtr();

    [[nodiscard]] Result<void, Error> clearFifoReadPtr();

    [[nodiscard]] Result<void, Error> setSyncWordBits(const SyncWordBits len);

    [[nodiscard]] Result<void, Error> setSyncWord(const uint32_t syncword);

    [[nodiscard]] Result<void, Error> setRetransTime(const uint8_t times);

    [[nodiscard]] Result<void, Error> enableAutoAck(const bool en = true);

    [[nodiscard]] Result<void, Error> init(const Power power, uint32_t syncword);

    [[nodiscard]] Result<void, Error> initRf();

    [[nodiscard]] Result<void, Error> intoSleep();

    [[nodiscard]] Result<void, Error> intoWake();

    [[nodiscard]] Result<void, Error> initBle(const Power power);

    [[nodiscard]] Result<void, Error> reset();
    [[nodiscard]] Result<void, Error> wakup(){return reset();}

    [[nodiscard]] Result<void, Error> sleep();

    [[nodiscard]] Result<void, Error> verify();

    [[nodiscard]] Result<void, Error> setTxPower(const Power power);

    [[nodiscard]] Result<size_t, Error> transmitRf(std::span<const std::byte> buf);

    [[nodiscard]] Result<size_t, Error> receiveRf(std::span<std::byte> buf);

    [[nodiscard]] Result<size_t, Error> transmitBle(std::span<const std::byte> buf);
    
    [[nodiscard]] Result<size_t, Error> receiveBle(std::span<std::byte> buf);

    [[nodiscard]] Result<void, Error> setDataRate(LT8960L::DataRate rate);

};

}

namespace ymd::custom{
    template<typename T>
    struct result_converter<T, drivers::LT8960L::Error, BusError> {
        static Result<T, drivers::LT8960L::Error> convert(const BusError berr){
            using Error = drivers::LT8960L::Error;
            using BusError = BusError;
            
            if(berr.ok()) return Ok();

            Error err = [](const BusError berr_){
                switch(berr_.type){
                    // case BusError::NO_ACK : return Error::I2C_NOT_ACK;

                    // case BusError::I2C_NOT_READY: return LT8960L::Error::I2C_NOT_READY;
                    default: return Error::UNSPECIFIED;
                }
            }(berr);

            return Err(err); 
        }
    };
}
