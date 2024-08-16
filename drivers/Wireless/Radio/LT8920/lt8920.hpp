#ifndef __LT8920_HPP__

#define __LT8920_HPP__

#include "../../hal/bus/spi/spidrv.hpp"
#include "../../hal/bus/i2c/i2cdrv.hpp"
#include <optional>

#ifdef LT8920_DEBUG
#define LT8920_DEBUG(...) DEBUG_LOG(__VA_ARGS__)
#else
#define LT8920_DEBUG(...)
#endif

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstrict-aliasing"

#define REG16(x) (*reinterpret_cast<uint16_t *>(&x))
#define REG8(x) (*reinterpret_cast<uint8_t *>(&x))


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
        Mbps1 = 0, Kbps250, Kbps125, Kbps62_5
    };

protected:
    std::optional<SpiDrv> spi_drv;
    std::optional<I2cDrv> i2c_drv;
    GpioConcept * packet_status_gpio = nullptr;
    GpioConcept * fifo_status_gpio = nullptr;

    using RegAddress = uint8_t;
    struct RfSynthLockReg{
        static constexpr RegAddress address = 3;
        uint16_t __resv1__ :12;
        bool synthLocked:1;//RF 频率综合器锁定标志位
        uint16_t __resv2__ :3;
    };

    struct RawRssiReg{
        static constexpr RegAddress address = 6;
        uint16_t __resv__ :10;
        uint16_t rawRssi:6;//RSSI 原始数据
    };

    struct RfConfigReg{
        static constexpr RegAddress address = 7;
        uint16_t rfChannelNo :6;//设定 RF 频道，空中频率为：f=2402+ RF_PLL_CH_NO
        bool rxEn:1;//使芯片进入 TX状态，1 有效
        bool txEn:1;//使芯片进入 RX 状态，1 有效
        uint16_t __resv__ :8;
    };

    struct PaConfigReg{
        static constexpr RegAddress address =9;
        uint16_t __resv1__ :7;
        uint16_t paGain:4;//PA增益控制
        uint16_t __resv2__ :1;
        uint16_t paCurrent:4;//PA电流控制
    };

    struct OscEnableReg{
        static constexpr RegAddress address =10;
        uint16_t oscEn :1;//开启晶体振荡器
        uint16_t __resv__ :15;
    };

    struct RssiPdnReg{
        static constexpr RegAddress address =11;
        uint16_t __resv1__ :8;
        uint16_t rssiPdn:1;//关闭 RSSI
        uint16_t __resv2__ :7;
    };

    struct AutoCaliReg{
        static constexpr RegAddress address = 23;
        uint16_t __resv__ :2;
        uint16_t autoCali:1;//在 TX/RX 开启前重新校准 VCO
        uint16_t __resv1__ :14;
    };

    struct DeviceIDReg{
        static constexpr RegAddress address = 29;
        uint16_t digiVersion:3;//数字版本号
        uint16_t __resv1__ :1;
        uint16_t rfVersion:4;//RF 版本号
        uint16_t __resv2__ :8;
    };

    struct Config1Reg{
        static constexpr RegAddress address = 32;
        uint16_t __resv1__ :1;
        BrclkSel brclkSel:3;//时钟选择
        uint16_t __resv2__ :2;
        PacketType packetType:2;//包类型
        uint16_t trailerLen:3;//尾缀码长度
        SyncWordBits syncWordLen :2;//同步字比特数
        uint16_t preambleLen :3;//数据载荷长度
    };

    struct Delay1Reg{
        static constexpr RegAddress address = 33;
        uint16_t txPaOnDelays:6;//在 VCO_ON 以后，等待内部 PA开启的时间，单位为 1uS
        uint16_t txPaOffDelays:2;//PA 关闭的等待时间，单位是 1uS，基数是 4uS，00 表示 4uS
        uint16_t vcoOnDelays:8;//在每次进入 RX或者 TX 后，等待内部 VCO 稳定的时间，单位为 1uS。
    };

    struct Delay2Reg{
        static constexpr RegAddress address = 34;
        uint16_t txSwOnDelays:6;//VCO_ON 后，等待 RF switch 开启的时间，单位 1uS
        uint16_t __resv__ :2;
        uint16_t rxCwDelays:7;//在发射数据前，传输 CW 调制信号的时间
        uint16_t bpktDirect:1;//在 direct mode 中，它控制 TX 的 PA 和 RX 的宽带/窄带模式
    };


    struct Config2Reg{
        static constexpr RegAddress address = 35;
        uint16_t scramableData :7;//Scramble data 的种子，收发两边必须一致
        bool misoTri :1; //当 SPI_SS=1 时，MISO 保持三态/低阻
        uint16_t retransTimes:4;//在 auto-ack 功能开启时，最多的重发次数。设为 3 时，为重发 2 次
        bool brclkOnSleep:1;//在 sleep mode 开启晶体振荡器耗电但能快速启动
        bool __resv__ :1;
        bool sleepMode:1;//进入 sleep mode，晶体关闭，保持 LDO 工作（寄存器值将保留）当 SPI_SS 为低时，芯片将重新工作
        bool powerDown:1;//先关闭晶体振荡器，再关闭 LDO。（寄存器值将丢失）
    };

    struct SyncWordReg{
        static constexpr RegAddress head_address = 36;
        uint16_t data;
    };

    struct ThresholdReg{
        static constexpr RegAddress address = 40;
        uint16_t syncWordThreshold:6;//认为 SYNCWORD 为正确的阈值 07 表示可以错 6bits，01 表示 0bit 可以错 0bits
        uint16_t fifoFullThreshold:4;//认为 FIFO 为满的阈值
        uint16_t fifoEmptyThreshold:4;//认为 FIFO 为空的阈值
    };

    struct Config3Reg{
        static constexpr RegAddress address = 41;
        uint16_t crcInitalData:8;//CRC 计算初始值。
        uint16_t __resv1__ :2;
        bool pktFifoPolarity:1; //PKT flag, FIFO flag 低有效.
        bool autoAck:1;//当接收到数据，自动回 ACK 或者 NACK
        bool fwTermTx :1;//1: 当 FIFO 的读指针和写指针相等时，LT8920 将关闭发射。
        bool packLengthEN:1;//1: 第一字节表示 payload 的长度 如要写 8 个 byte 有效字节，那第一个字节应写 8，总长 9
        bool __resv2__ :1;
        bool crcOn:1;//开启 CRC
    };

    struct RxConfigReg{
        static constexpr RegAddress address = 42;
        uint16_t rxAckTime:8;//等待 RX_ACK 的时间，1 表示 1uS
        uint16_t __resv__ :2;
        uint16_t scanRssiChNo:6;//RSSI 扫描的信道数量，RSSI 值将保留到 FIFO 中
    };

    struct RssiConfigReg{
        static constexpr RegAddress address = 43;
        uint16_t waitRssiScanTime :8;//设置在扫描不同信道 RSSI 时，VCO&SYN 稳定时间
        uint16_t rssiStartChOffset:7;//通常 RSSI 从 2402MHz 开始扫描（0 信道）。这里可以开始的信道数 如设为 10，将从 2412MHz 开始扫描
        bool scanRssiEn :1;//开始扫描 RSSI
    };

    struct DataRateReg{
        static constexpr RegAddress address = 44;
        uint16_t __resv__ :8;
        DataRate dataRate:8;//透传速率
    };

    struct FlagReg{
        static constexpr RegAddress address = 48;
        uint16_t __resv__ :5;
        bool fifoFlag:1;
        bool pktFlag:1;
        bool syncWordRecved:1;//表示收到 syncword，只在接收时有效。跳出接收状态时，为 0
        uint16_t framerFlag:6;//Framer 状态
        bool fec23ErrorFlag:1;//FEC23 错误标志位
        bool crcErrorFlag:1;//CRC 错误标志位
    };

    struct FifoReg{
        static constexpr RegAddress address = 50;
        uint16_t data:16;
    };

    struct FifoPtrReg{
        static constexpr RegAddress address = 52;
        //FIFO 读指针
        //当使用 auto-ack 功能时，此位可以做为标志位。
        //当 PKT 拉高后，读此寄存器，如果为 0，即收到 ack。
        //如果不为 0，而是发射总 byte 数+1，即没收到 ack。
        uint16_t fifoReadPtr:6;

        uint16_t __resv1__:1;

        bool clearReadPtr:1;//清空 RX FIFO 指针为 0..但不清空 RX FIFO 中的数据
        uint16_t fifoWritePtr:6;//FIFO 写指针
        uint16_t __resv2__:1;
        uint16_t clearWritePtr:1;//清空 TX FIFO 指针为 0，但不清空 TX FIFO 中的数据
    };

    struct{

        //REG3 RO
        RfSynthLockReg rfSynthLockReg;

        //REG6 RO
        RawRssiReg rawRssiReg;

        //REG7 
        RfConfigReg rfConfigReg;

        //REG9
        PaConfigReg paConfigReg;

        //REG10
        OscEnableReg oscEnableReg;

        //REG11
        RssiPdnReg rssiPdnReg;

        //REG23
        AutoCaliReg autoCaliReg;

        //REG29 RO
        DeviceIDReg deviceIDReg;

        //REG32 RO
        Config1Reg config1Reg;

        //REG33 RO
        Delay1Reg delay1Reg;

        //REG34
        Delay2Reg delay2Reg;

        //REG35
        Config2Reg config2Reg;
        union{
            struct{
                //REG36
                uint16_t syncWord0Reg;

                //REG37
                uint16_t syncWord1Reg;

                //REG38
                uint16_t syncWord2Reg;

                //REG39
                uint16_t syncWord3Reg;
            };
            uint16_t syncWordRegs[4]; 
        };


        //REG40
        ThresholdReg thresholdReg;

        //REG41
        Config3Reg config3Reg;

        //REG42
        RxConfigReg rxConfigReg;

        //REG43
        RssiConfigReg rssiConfigReg;

        //REG44
        DataRateReg dataRateReg;

        //REG48 RO
        FlagReg flagReg;

        //REG50
        uint16_t fifoReg;

        //REG51
        FifoPtrReg fifoPtrReg;
    };

    void delayT3(){
        delayMicroseconds(1);
    }

    void delayT5(){
        delayMicroseconds(1);
    }

    void writeReg(const RegAddress address, const uint16_t reg){
        if(spi_drv){
            spi_drv->write((uint8_t)((uint8_t)address & 0x80), false);
            delayT3();

            uint8_t * reg_ptr = (uint8_t *)&reg;
            spi_drv->write(reg_ptr[1], false);
            delayT5();
            spi_drv->write(reg_ptr[0]);
        }else if(i2c_drv){
            i2c_drv->writeReg((uint8_t)address, reg);
        }
        LT8920_DEBUG("write",*(uint16_t *)&reg, "at", (uint8_t)address);
    }

    void readReg(const RegAddress address, uint16_t & reg){
        if(spi_drv){
            uint8_t temp = 0;
            spi_drv->transfer(temp, (uint8_t)((uint8_t)address & 0x80), false);

            uint16_t * rawFlagReg = reinterpret_cast<uint16_t *>(&flagReg);

            *rawFlagReg &= 0x00ff;
            *rawFlagReg |= temp << 8;

            delayT3();

            uint8_t buf[2];
            spi_drv->read(buf, 2);

            uint8_t * reg_ptr = (uint8_t *)&reg;
            reg_ptr[0] = buf[0];
            reg_ptr[1] = buf[1];
        }else if(i2c_drv){
            i2c_drv->readReg((uint8_t)address, reg);
        }
        LT8920_DEBUG("read",*(uint16_t *)&reg, "at", (uint8_t)address);
    }

    void writeByte(const RegAddress address, const uint8_t data){
        if(spi_drv){
            spi_drv->write((uint8_t)((uint8_t)address & 0x80), false);
            delayT3();

            spi_drv->write(data);
        }else if(i2c_drv){
            i2c_drv->writeReg((uint8_t)address, data);
        }
    }

    void readByte(const RegAddress address, uint8_t & data){
        if(spi_drv){
            spi_drv->write((uint8_t)((uint8_t)address & 0x80), false);
            delayT3();
            spi_drv->read(data);
        }else if(i2c_drv){
            i2c_drv->readReg((uint8_t)address, data);
        }
    }
public:
    LT8920(SpiDrv & _spi_drv) : spi_drv(_spi_drv) {;}

    bool isRfSynthLocked(){
        readReg(rfSynthLockReg.address, REG16(rfSynthLockReg));
        return rfSynthLockReg.synthLocked;
    }

    uint8_t getRssi(){
        readReg(rawRssiReg.address, REG16(rawRssiReg));
        return rawRssiReg.rawRssi;
    }

    void setRfChannel(const uint8_t ch){
        rfConfigReg.rfChannelNo = ch;
        writeReg(rfConfigReg.address, REG16(rfConfigReg));
    }

    void setRfFreqMHz(const uint freq){}

    void setRadioMode(const bool isRx){
        if(isRx){
            rfConfigReg.txEn = false;
            rfConfigReg.rxEn = true;
        }else{
            rfConfigReg.rxEn = false;
            rfConfigReg.txEn = true;
        }
        writeReg(rfConfigReg.address, REG16(rfConfigReg));
    }

    void read(uint8_t *buffer, size_t maxBuffer);

    void setPaCurrent(const uint8_t current){
        paConfigReg.paCurrent = current;
        writeReg(paConfigReg.address,REG16(paConfigReg));
    }

    void setPaGain(const uint8_t gain){
        paConfigReg.paGain = gain;
        writeReg(paConfigReg.address, REG16(paConfigReg));
    }

    void enableRssi(const bool open = true){
        rssiPdnReg.rssiPdn = open;
        writeReg(rssiPdnReg.address, REG16(rssiPdnReg));
    }

    void enableAutoCali(const bool open){
        autoCaliReg.autoCali = open;
        writeReg(autoCaliReg.address, REG16(autoCaliReg));
    }

    uint8_t getDigiVersion(){
        readReg(deviceIDReg.address, REG16(deviceIDReg));
        return deviceIDReg.digiVersion;
    }

    uint8_t getRfVersion(){
        readReg(deviceIDReg.address, REG16(deviceIDReg));
        return deviceIDReg.rfVersion;
    }

    void setBrclkSel(const BrclkSel brclkSel){
        config1Reg.brclkSel = brclkSel;
        writeReg(config1Reg.address, REG16(config1Reg));
    }

    void clearFifoWritePtr(){
        fifoPtrReg.clearWritePtr = 1;
        writeReg(fifoPtrReg.address, REG16(fifoPtrReg));
    }

    void clearFifoReadPtr(){
        fifoPtrReg.clearReadPtr = 1;
        writeReg(fifoPtrReg.address, REG16(fifoPtrReg));
    }

    void setSyncWordBitsgth(const SyncWordBits len){
        config1Reg.syncWordLen= len;
        writeReg(config1Reg.address, REG16(config1Reg));
    }

    void setRetransTime(const uint8_t times){
        config2Reg.retransTimes = times - 1;
        writeReg(config2Reg.address, REG16(config2Reg));
    }

    void enableAutoAck(const bool en = true){
        config3Reg.autoAck = en;
        writeReg(config3Reg.address, REG16(config3Reg));
    }

    void init(){
        delay(5);
        setBrclkSel(BrclkSel::Mhz12);
        delay(5);
        enableRssi();
        enableAutoAck();
        setSyncWordBitsgth(SyncWordBits::_32);
    }
};

#pragma GCC diagnostic pop

#ifdef LT8920_DEBUG
#undef LT8920_DEBUG
#endif
#endif