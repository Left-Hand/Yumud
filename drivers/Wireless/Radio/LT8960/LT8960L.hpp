//这个驱动还在推进状态

#pragma once

#include "drivers/device_defs.h"
#include "sys/utils/Result.hpp"


template<typename T>
class PtrlikeRef{
public:
    PtrlikeRef(T * pobj):obj_(*pobj) {}
    PtrlikeRef(std::nullptr_t) = delete;
    PtrlikeRef(PtrlikeRef &&) = default;
    T * operator ->() {return &obj_;}
    T * operator &() {return &obj_;}
    T & operator *() {return obj_;}
private:
    T & obj_;
};

template<typename T>
class PtrlikeVase{

public:
    PtrlikeVase(const T & obj):obj_(obj) {}
    PtrlikeVase(T && obj):obj_(std::move(obj)) {}
    PtrlikeVase(PtrlikeRef<T> ref):obj_(*ref) {}

    PtrlikeVase(const PtrlikeVase &) = delete;
    PtrlikeVase(PtrlikeVase &&) = default;
    T * operator ->() {return &obj_;}
private:
    T obj_;
};

namespace ymd::drivers{

struct _LT8960L_Regs{
    using RegAddress = uint8_t;

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

    struct R16_ChipId:public Reg16<>{
        static constexpr RegAddress address = 0x00;
        static constexpr uint16_t key = 0x6fe0;
        uint16_t chip_id;
    }DEF_R16(chip_id_reg)
    
    struct R16_RfSynthLock:public Reg16<>{
        static constexpr RegAddress address = 0x03;
        uint16_t i2c_soft_rstn:1;//软件复位标志
        uint16_t __resv1__ :8;
        uint16_t fifo_flag_txrx:1;//fifo状态指示
        uint16_t __resv2__ :2;
        uint16_t synth_locked:1;//RF 频率综合器锁定标志位
        uint16_t pkt_flag_txrx:1;//包状态指示
        uint16_t __resv3__ :2;
    }DEF_R16(rf_synthlock_reg)

    struct R16_RfConfig:public Reg16<>{
        static constexpr RegAddress address = 7;
        
        uint16_t rf_channel_no :7;//设定 RF 频道，空中频率为：f=2402+ RF_PLL_CH_NO
        uint16_t rx_en:1;//使芯片进入 TX状态，1 有效
        uint16_t tx_en:1;//使芯片进入 RX 状态，1 有效
        uint16_t __resv__ :7;
    }DEF_R16(rf_config_reg)
    
    struct R16_PaConfig:public Reg16<>{
        static constexpr RegAddress address = 9;

        uint16_t __resv1__ :7;
        uint16_t pa_gain:4;//PA增益控制
        uint16_t __resv2__ :1;
        uint16_t pa_current:4;//PA电流控制
    }DEF_R16(pa_config_reg)
    
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
    }DEF_R16(func_conf_reg)
    
    struct R16_FreqOffs:public Reg16<>{
        static constexpr RegAddress address = 0x1c;

        uint16_t offs:14;
        uint16_t __resv__:2;
    }DEF_R16(freq_offs_reg)


    struct R16_Config1:public Reg16<>{
        static constexpr RegAddress address = 0x20;

        uint16_t __resv1__ :1;
        BrclkSel brclkSel:3;//时钟选择
        uint16_t __resv2__ :2;
        PacketType packet_type:2;//包类型
        uint16_t trailer_len:3;//尾缀码长度
        SyncWordBits syncword_len :2;//同步字比特数
        uint16_t preamble_len :3;//数据载荷长度
    }DEF_R16(config1_reg)


    struct R16_Config2:public Reg16<>{
        static constexpr RegAddress address = 0x23;

        uint16_t scramable_data :7;//Scramble data 的种子，收发两边必须一致
        uint16_t misoTri :1; //当 SPI_SS=1 时，MISO 保持三态/低阻
        uint16_t retrans_times:4;//在 auto-ack 功能开启时，最多的重发次数。设为 3 时，为重发 2 次
        uint16_t brclk_on_sleep:1;//在 sleep mode 开启晶体振荡器耗电但能快速启动
        uint16_t __resv__ :1;
        uint16_t sleep_mode:1;//进入 sleep mode，晶体关闭，保持 LDO 工作（寄存器值将保留）当 SPI_SS 为低时，芯片将重新工作
        uint16_t power_down:1;//先关闭晶体振荡器，再关闭 LDO。（寄存器值将丢失）
    }DEF_R16(config2_reg)

    struct R16_SyncWord0:public Reg16<>{
        static constexpr RegAddress address = 0x24;
        uint8_t word[2];
    }DEF_R16(sync_word0_reg)

    struct R16_SyncWord1:public Reg16<>{
        static constexpr RegAddress address = 0x27;
        uint8_t word[2];
    }DEF_R16(sync_word1_reg)

    struct R16_Threshold:public Reg16<>{
        static constexpr RegAddress address = 0x28;
        uint16_t syncword_threshold:6;//认为 SYNCWORD 为正确的阈值 07 表示可以错 6bits，01 表示 0bit 可以错 0bits
        uint16_t fifo_full_threshold:5;//认为 FIFO 为满的阈值
        uint16_t fifo_empty_threshold:5;//认为 FIFO 为空的阈值
    }DEF_R16(threshold_reg)
    
    struct R16_Config3:public Reg16<>{
        static constexpr RegAddress address = 0x29;
        uint16_t crc_inital_data:8;//CRC 计算初始值。
        uint16_t __resv1__ :2;
        uint16_t pkt_fifo_polarity:1; //PKT flag, FIFO flag 低有效.
        uint16_t autoack_en:1;//当接收到数据，自动回 ACK 或者 NACK
        uint16_t fw_term_tx :1;//1: 当 FIFO 的读指针和写指针相等时，LT8960L 将关闭发射。
        uint16_t pack_length_en:1;//1: 第一字节表示 payload 的长度 如要写 8 个 byte 有效字节，那第一个字节应写 8，总长 9
        uint16_t __resv2__ :1;
        uint16_t crc_on:1;//开启 CRC
    }DEF_R16(config3_reg)

    struct R16_RxConfig:public Reg16<>{
        static constexpr RegAddress address = 0x2A;
        uint16_t auot_rx_ack_time:8;//等待 RX_ACK 的时间，1 表示 1uS
        uint16_t wakeup_tim :2;
        uint16_t scan_rssi_ch_no:6;//RSSI 扫描的信道数量，RSSI 值将保留到 FIFO 中
    }DEF_R16(rx_config_reg)

    struct R16_DataRate:public Reg16<>{
        static constexpr RegAddress address = 0x2C;
        uint16_t __resv__ :8;
        uint16_t data_rate:8;//透传速率
    }DEF_R16(data_rate_reg)

    struct R16_ModemOption:public Reg16<>{
        static constexpr RegAddress address = 0x2D;
        uint16_t option;//透传速率
    }DEF_R16(modem_option_reg)

    struct R16_ChiIndex:public Reg16<>{
        static constexpr RegAddress address = 0x2E;
        uint16_t __resv__ :8;
        uint16_t chi_index:6;//透传速率
        uint16_t __resv2__ :2;
    }DEF_R16(chi_index_reg)

    struct R16_Flag:public Reg16<>{
        static constexpr RegAddress address = 0x30;
        uint16_t __resv__ :5;
        uint16_t fifo_flag:1;
        uint16_t pkt_flag:1;
        uint16_t syncword_received:1;//表示收到 syncword，只在接收时有效。跳出接收状态时，为 0
        uint16_t framer_flag:6;//Framer 状态
        uint16_t __resv2__:1;//FEC23 错误标志位
        uint16_t crcerr_flag:1;//CRC 错误标志位
    }DEF_R16(flag_reg)

    struct R16_Fifo:public Reg16<>{
        static constexpr RegAddress address = 0x32;
        uint16_t data;
    }DEF_R16(fifo_reg)

    struct R16_FifoPtr:public Reg16<>{
        static constexpr RegAddress address = 52;
        //FIFO 读指针
        //当使用 auto-ack 功能时，此位可以做为标志位。
        //当 PKT 拉高后，读此寄存器，如果为 0，即收到 ack。
        //如果不为 0，而是发射总 byte 数+1，即没收到 ack。
        uint16_t fifo_read_ptr:6;
        uint16_t __resv1__:1;

        uint16_t clear_read_ptr:1;//清空 RX FIFO 指针为 0..但不清空 RX FIFO 中的数据
        uint16_t fifo_write_ptr:6;//FIFO 写指针
        uint16_t __resv2__:1;
        uint16_t clear_write_ptr:1;//清空 TX FIFO 指针为 0，但不清空 TX FIFO 中的数据
    }DEF_R16(fifo_ptr_reg)

    struct R16_I2cOper:public Reg16<>{
        static constexpr RegAddress address = 0x38;
        uint16_t wakeup_i2c:1;
        uint16_t soft_rst:1;
        uint16_t __resv__:14;
    }DEF_R16(i2c_oper_reg)
};

class LT8960L{
public:
    static constexpr uint8_t default_i2c_addr = 0x1A;

    enum class Error:uint8_t{
        TransmitTimeout,
        PacketOverlength,
        ChipIdMismatch,
        Unspecified = 0xff
    };
    

    class Channel{
    public:
        Channel (const uint8_t ch):
            ch_(ch){;}

        auto into_code() const {
            return ch_;
        }
    private:    
        const uint8_t ch_;
    };

    class LT8960L_Phy:public hal::ProtocolBusDrv<hal::I2c> {
    protected:
        uint8_t index_r_ = 0;
    public:
        LT8960L_Phy(hal::I2c & i2c, const uint8_t index, const uint8_t index_r):
            hal::ProtocolBusDrv<hal::I2c>(i2c, index), index_r_(index_r){;};
    
        [[nodiscard]] 
        Result<void, Error> write_reg(
            uint8_t address, 
            uint16_t data
        );
    
        [[nodiscard]] 
        Result<void, Error> read_reg(
            uint8_t address, 
            uint16_t & data
        );

        [[nodiscard]] 
        Result<size_t, Error> read_burst(uint8_t address, std::span<std::byte> pbuf);

        [[nodiscard]] 
        Result<size_t, Error> write_burst(uint8_t address, std::span<const std::byte> pbuf);
    
        [[nodiscard]]
        BusError verify();
    
        [[nodiscard]]
        BusError release();
    };
    
protected:

    using Regs = _LT8960L_Regs;

    using RegAddress = Regs::RegAddress;
    using PacketType = Regs::PacketType;
    using Power = Regs::Power;
    using TrailerBits = Regs::TrailerBits;

    using  SyncWordBits = Regs::SyncWordBits;
    using  PreambleBits = Regs::PreambleBits;
    using  BrclkSel = Regs::BrclkSel;
    using  DataRate = Regs::DataRate;
    using  Mode = Regs::Mode;
    
    Regs regs_ = {};

    // struct DevDriver{
    // public:
    //     DevDriver(const hal::I2cDrv & i2c_drv):i2c_drv_(i2c_drv){}
    //     DevDriver(hal::I2cDrv && i2c_drv):i2c_drv_(std::move(i2c_drv)){}

    //     [[nodiscard]] Result<size_t, Error> write_burst(const RegAddress address, std::span<const std::byte> buf);

    //     [[nodiscard]] Result<size_t, Error> read_burst(const RegAddress address, std::span<std::byte> buf);

    //     [[nodiscard]] Result<void, Error> write_reg(const RegAddress address, const uint16_t reg);

    //     [[nodiscard]] Result<void, Error> read_reg(const RegAddress address, uint16_t & reg);

    //     [[nodiscard]] Result<void, Error> verify();
    // private:
    //     hal::I2cDrv i2c_drv_;
    // };

    

    LT8960L_Phy dev_drv_;

    hal::GpioIntf * p_packet_status_gpio = nullptr;
    hal::GpioIntf * p_fifo_status_gpio = nullptr;

    bool use_hw_pkt_ = false;//使能通过监听引脚判断数据是否发送完成

    DataRate datarate_;


    __no_inline void delay_t3();
    __no_inline void delay_t5();


    [[nodiscard]] __fast_inline
    Result<void, Error> write_reg(const RegAddress address, const uint16_t reg){
        return dev_drv_.write_reg(address, reg);
    }


    [[nodiscard]] __fast_inline
    Result<void, Error> read_reg(const RegAddress address, uint16_t & reg){
        return dev_drv_.read_reg(address, reg);
    }


    template<typename ... Ts>
    [[nodiscard]] __fast_inline
    Result<void, Error> write_regs(Ts const & ... reg) {
        return (dev_drv_.write_reg(reg.address, reg.as_val()) | ...);
    }

    template<typename ... Ts>
    [[nodiscard]] __fast_inline
    Result<void, Error> read_regs(Ts & ... reg) {
        return (dev_drv_.read_reg(reg.address, reg.as_ref()) | ...);
    }

    template<typename T>
    [[nodiscard]] __fast_inline
    Result<void, Error> read_reg(T & reg){
        return dev_drv_.read_reg(reg.address, reg);
    }


    [[nodiscard]] __fast_inline
    Result<size_t, Error> write_fifo(std::span<const std::byte> buf){
        return dev_drv_.write_burst(Regs::R16_Fifo::address, buf);
    }

    [[nodiscard]] Result<void, Error> fill_fifo(const uint16_t data, const size_t len);


    [[nodiscard]] __fast_inline
    Result<size_t, Error> read_burst(std::span<std::byte> buf){
        return dev_drv_.read_burst(Regs::R16_Fifo::address, buf);
    }

    [[nodiscard]]
    Result<void, Error> set_pa_current(const uint8_t current);

    [[nodiscard]]
    Result<void, Error> set_pa_gain(const uint8_t gain);

    [[nodiscard]] Result<void, Error> enable_analog(bool en = true);
    [[nodiscard]] Result<void, Error> change_carrier(const Channel ch);

    [[nodiscard]] Result<void, Error> set_rf_channel(const Channel ch, const bool tx, const bool rx);
    [[nodiscard]] Result<void, Error> set_rf_channel_and_into_tx(const Channel ch){return set_rf_channel(ch, 1, 0);}
    [[nodiscard]] Result<void, Error> set_rf_channel_and_into_rx(const Channel ch){return set_rf_channel(ch, 0, 1);}
    [[nodiscard]] Result<void, Error> set_rf_channel_no_tx_rx(const Channel ch){return set_rf_channel(ch, 0, 0);}

    [[nodiscard]] Result<bool, Error> is_pkt_ready();

    [[nodiscard]] Result<bool, Error> is_rst_done();

    [[nodiscard]] Result<void, Error> wait_pkt_ready(const uint timeout);

    [[nodiscard]] Result<void, Error> wait_rst_done(const uint timeout);

    [[nodiscard]] Result<void, Error> clear_fifo_write_and_read_ptr();

    [[nodiscard]] Result<void, Error> verify_chipid();
public:

    LT8960L(hal::I2c * bus, const uint8_t i2c_addr = default_i2c_addr):
        dev_drv_(*bus, i2c_addr, i2c_addr | 0x80){;}


    [[nodiscard]] Result<bool, Error> is_rfsynth_locked();

    [[nodiscard]] Result<void, Error> set_rf_freq_mhz(const uint freq);

    [[nodiscard]] Result<void, Error> set_radio_mode(const bool isRx);

    [[nodiscard]] Result<void, Error> set_brclk_sel(const BrclkSel brclkSel);

    [[nodiscard]] Result<void, Error> clear_fifo_write_ptr();

    [[nodiscard]] Result<void, Error> clear_fifo_read_ptr();

    [[nodiscard]] Result<void, Error> set_syncword_bits(const SyncWordBits len);

    [[nodiscard]] Result<void, Error> set_syncword(const uint32_t syncword);

    [[nodiscard]] Result<void, Error> set_retrans_time(const uint8_t times);

    [[nodiscard]] Result<void, Error> enable_autoack(const bool en = true);

    [[nodiscard]] Result<void, Error> init(const Power power, uint32_t syncword);

    [[nodiscard]] Result<void, Error> init_rf();
    
    [[nodiscard]] Result<void, Error> init_ble(const Power power);

    [[nodiscard]] Result<void, Error> into_sleep();

    [[nodiscard]] Result<void, Error> into_wake();


    [[nodiscard]] Result<void, Error> reset();

    [[nodiscard]] Result<void, Error> wakup(){return reset();}

    [[nodiscard]] Result<void, Error> sleep();

    [[nodiscard]] Result<void, Error> verify();

    [[nodiscard]] Result<void, Error> set_tx_power(const Power power);

    [[nodiscard]] Result<size_t, Error> transmit_rf(Channel ch, std::span<const std::byte> buf);

    [[nodiscard]] Result<size_t, Error> receive_rf(std::span<std::byte> buf);

    [[nodiscard]] Result<size_t, Error> transmit_ble(std::span<const std::byte> buf);
    
    [[nodiscard]] Result<size_t, Error> receive_ble(std::span<std::byte> buf);

    [[nodiscard]] Result<void, Error> set_datarate(LT8960L::DataRate rate);

};

}

namespace ymd::custom{
    template<typename T>
    struct result_converter<T, drivers::LT8960L::Error, BusError> {
        static Result<T, drivers::LT8960L::Error> convert(const BusError berr){
            using Error = drivers::LT8960L::Error;
            using BusError = BusError;
            
            if constexpr(std::is_void_v<T>)
                if(berr.ok()) return Ok();
            
            Error err = [](const BusError berr_){
                switch(berr_.type){
                    // case BusError::NO_ACK : return Error::I2C_NOT_ACK;

                    // case BusError::I2C_NOT_READY: return LT8960L::Error::I2C_NOT_READY;
                    default: return Error::Unspecified;
                }
            }(berr);

            return Err(err); 
        }
    };
}
