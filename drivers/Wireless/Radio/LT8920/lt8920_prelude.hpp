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


    using RegAddr = uint8_t;
    struct R16_RfSynthLock:public Reg16<>{
        static constexpr RegAddr ADDRESS = 3;
        uint16_t __resv1__ :12;
        uint16_t synth_locked:1;//RF 频率综合器锁定标志位
        uint16_t __resv2__ :3;
    };

    struct R16_RawRssi:public Reg16<>{
        static constexpr RegAddr ADDRESS = 6;
        uint16_t __resv__ :10;
        uint16_t raw_rssi:6;//RSSI 原始数据
    };

    struct R16_RfConfig:public Reg16<>{
        static constexpr RegAddr ADDRESS = 7;
        uint16_t rf_channel_no :7;//设定 RF 频道，空中频率为：f=2402+ RF_PLL_CH_NO
        uint16_t rx_en:1;//使芯片进入 TX 状态，1 有效
        uint16_t tx_en:1;//使芯片进入 RX 状态，1 有效
        uint16_t __resv__ :7;
    };

    struct R16_PaConfig:public Reg16<>{
        static constexpr RegAddr ADDRESS =9;
        uint16_t __resv1__ :7;
        uint16_t pa_gain:4;//PA增益控制
        uint16_t __resv2__ :1;
        uint16_t pa_current:4;//PA电流控制
    };

    struct R16_OscEnable:public Reg16<>{
        static constexpr RegAddr ADDRESS =10;
        uint16_t osc_en :1;//开启晶体振荡器
        uint16_t __resv__ :15;
    };

    struct R16_RssiPdn:public Reg16<>{
        static constexpr RegAddr ADDRESS =11;
        uint16_t __resv1__ :8;
        uint16_t rssi_pdn:1;//关闭 RSSI
        uint16_t __resv2__ :7;
    };

    struct R16_AutoCali:public Reg16<>{
        static constexpr RegAddr ADDRESS = 23;
        uint16_t __resv__ :2;
        uint16_t auto_cali:1;//在 TX/RX 开启前重新校准 VCO
        uint16_t __resv1__ :13;
    };

    struct R16_DeviceID:public Reg16<>{
        static constexpr RegAddr ADDRESS = 29;
        uint16_t digi_version:3;//数字版本号
        uint16_t __resv1__ :1;
        uint16_t rf_version:4;//RF 版本号
        uint16_t __resv2__ :8;
    };

    struct R16_Config1:public Reg16<>{
        static constexpr RegAddr ADDRESS = 32;
        uint16_t __resv1__ :1;
        uint16_t brclk_sel:3;//时钟选择
        uint16_t __resv2__ :2;
        uint16_t packet_type:2;//包类型
        uint16_t trailer_len:3;//尾缀码长度
        uint16_t sync_word_len :2;//同步字比特数
        uint16_t preamble_len :3;//数据载荷长度
    };

    struct R16_Delay1:public Reg16<>{
        static constexpr RegAddr ADDRESS = 33;
        uint16_t tx_pa_on_delays:6;//在 VCO_ON 以后，等待内部 PA开启的时间，单位为 1uS
        uint16_t tx_pa_off_delays:2;//PA 关闭的等待时间，单位是 1uS，基数是 4uS，00 表示 4uS
        uint16_t vco_on_delays:8;//在每次进入 RX或者 TX 后，等待内部 VCO 稳定的时间，单位为 1uS。
    };

    struct R16_Delay2:public Reg16<>{
        static constexpr RegAddr ADDRESS = 34;
        uint16_t tx_sw_on_delays:6;//VCO_ON 后，等待 RF switch 开启的时间，单位 1uS
        uint16_t __resv__ :2;
        uint16_t rx_cw_delays:7;//在发射数据前，传输 CW 调制信号的时间
        uint16_t bpkt_direct:1;//在 direct mode 中，它控制 TX 的 PA 和 RX 的宽带/窄带模式
    };


    struct R16_Config2:public Reg16<>{
        static constexpr RegAddr ADDRESS = 35;
        uint16_t scramable_data :7;//Scramble data 的种子，收发两边必须一致
        uint16_t miso_tri :1; //当 SPI_SS=1 时，MISO 保持三态/低阻
        uint16_t retrans_times:4;//在 auto-ack 功能开启时，最多的重发次数。设为 3 时，为重发 2 次
        uint16_t brclk_on_sleep:1;//在 sleep mode 开启晶体振荡器耗电但能快速启动
        uint16_t __resv__ :1;
        uint16_t sleep_mode:1;//进入 sleep mode，晶体关闭，保持 LDO 工作（寄存器值将保留）当 SPI_SS 为低时，芯片将重新工作
        uint16_t power_down:1;//先关闭晶体振荡器，再关闭 LDO。（寄存器值将丢失）
    };

    struct R16_SyncWord:public Reg16<>{
        static constexpr RegAddr head_address = 36;
        uint16_t data;
    };

    struct R16_Threshold:public Reg16<>{
        static constexpr RegAddr ADDRESS = 40;
        uint16_t errbits:6;//认为 SYNCWORD 为正确的阈值 07 表示可以错 6bits，01 表示 0bit 可以错 0bits
        uint16_t fifo_full_threshold:4;//认为 FIFO 为满的阈值
        uint16_t fifo_empty_threshold:4;//认为 FIFO 为空的阈值
        uint16_t __resv__:2;
    };

    struct R16_Config3:public Reg16<>{
        static constexpr RegAddr ADDRESS = 41;
        uint16_t crc_inital_data:8;//CRC 计算初始值。
        uint16_t __resv1__ :2;
        uint16_t pkt_fifo_polarity:1; //PKT flag, FIFO flag 低有效.
        uint16_t auto_ack:1;//1：接收到数据，自动回 ACK 或者 NACK
                           //0：接收数据后，不回 ACK，直接进 IDLE
        uint16_t fw_term_tx :1;//1: 当 FIFO 的读指针和写指针相等时，LT8920 将关闭发射。
        uint16_t pack_length_en:1;//1: 第一字节表示 payload 的长度 如要写 8 个 byte 有效字节，那第一个字节应写 8，总长 9
        uint16_t __resv2__ :1;
        uint16_t crc_en:1;//开启 CRC
    };

    struct R16_RxConfig:public Reg16<>{
        static constexpr RegAddr ADDRESS = 42;
        uint16_t rx_ack_time:8;//等待 RX_ACK 的时间，1 表示 1uS
        uint16_t __resv__ :2;
        uint16_t scan_rssi_ch_no:6;//RSSI 扫描的信道数量，RSSI 值将保留到 FIFO 中
    };

    struct R16_RssiConfig:public Reg16<>{
        static constexpr RegAddr ADDRESS = 43;
        uint16_t wait_rssi_scan_time :8;//设置在扫描不同信道 RSSI 时，VCO&SYN 稳定时间
        uint16_t rssi_start_ch_offset:7;//通常 RSSI 从 2402MHz 开始扫描（0 信道）。这里可以开始的信道数 如设为 10，将从 2412MHz 开始扫描
        uint16_t scan_rssi_en :1;//开始扫描 RSSI
    };

    struct R16_DataRate:public Reg16<>{
        static constexpr RegAddr ADDRESS = 44;
        uint16_t __resv__ :8;
        uint16_t dataRate:8;//透传速率
    };

    struct R16_Flag:public Reg16<>{
        static constexpr RegAddr ADDRESS = 48;
        uint16_t __resv__ :5;
        uint16_t fifo_flag:1;
        uint16_t pkt_flag:1;
        uint16_t syncword_received:1;//表示收到 syncword，只在接收时有效。跳出接收状态时，为 0
        uint16_t framer_flag:6;//Framer 状态
        uint16_t fec23_error_flag:1;//FEC23 错误标志位
        uint16_t crc_error_flag:1;//CRC 错误标志位
    };


    struct R16_FifoPtr:public Reg16<>{
        static constexpr RegAddr ADDRESS = 52;
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
    };

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



}