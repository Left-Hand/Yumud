    #include <cstdint>
    
    using RegAddress = uint8_t;
    struct RfSynthLockReg{
        static constexpr RegAddress address = 3;
        uint16_t __resv1__ :12;
        uint16_t synthLocked:1;//RF 频率综合器锁定标志位
        uint16_t __resv2__ :3;
    };

    struct RawRssiReg{
        static constexpr RegAddress address = 6;
        uint16_t __resv__ :10;
        uint16_t rawRssi:6;//RSSI 原始数据
    };

    struct RfConfigReg{
        static constexpr RegAddress address = 7;
        uint16_t rfChannelNo :7;//设定 RF 频道，空中频率为：f=2402+ RF_PLL_CH_NO
        uint16_t rxEn:1;//使芯片进入 TX 状态，1 有效
        uint16_t txEn:1;//使芯片进入 RX 状态，1 有效
        uint16_t __resv__ :7;
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
        uint16_t __resv1__ :13;
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
        uint16_t brclkSel:3;//时钟选择
        uint16_t __resv2__ :2;
        uint16_t packetType:2;//包类型
        uint16_t trailerLen:3;//尾缀码长度
        uint16_t syncWordLen :2;//同步字比特数
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
        uint16_t misoTri :1; //当 SPI_SS=1 时，MISO 保持三态/低阻
        uint16_t retransTimes:4;//在 auto-ack 功能开启时，最多的重发次数。设为 3 时，为重发 2 次
        uint16_t brclkOnSleep:1;//在 sleep mode 开启晶体振荡器耗电但能快速启动
        uint16_t __resv__ :1;
        uint16_t sleepMode:1;//进入 sleep mode，晶体关闭，保持 LDO 工作（寄存器值将保留）当 SPI_SS 为低时，芯片将重新工作
        uint16_t powerDown:1;//先关闭晶体振荡器，再关闭 LDO。（寄存器值将丢失）
    };

    struct SyncWordReg{
        static constexpr RegAddress head_address = 36;
        uint16_t data;
    };

    struct ThresholdReg{
        static constexpr RegAddress address = 40;
        uint16_t errbits:6;//认为 SYNCWORD 为正确的阈值 07 表示可以错 6bits，01 表示 0bit 可以错 0bits
        uint16_t fifoFullThreshold:4;//认为 FIFO 为满的阈值
        uint16_t fifoEmptyThreshold:4;//认为 FIFO 为空的阈值
        uint16_t __resv__:2;
    };

    struct Config3Reg{
        static constexpr RegAddress address = 41;
        uint16_t crcInitalData:8;//CRC 计算初始值。
        uint16_t __resv1__ :2;
        uint16_t pktFifoPolarity:1; //PKT flag, FIFO flag 低有效.
        uint16_t autoAck:1;//1：接收到数据，自动回 ACK 或者 NACK
                           //0：接收数据后，不回 ACK，直接进 IDLE
        uint16_t fwTermTx :1;//1: 当 FIFO 的读指针和写指针相等时，LT8920 将关闭发射。
        uint16_t packLengthEN:1;//1: 第一字节表示 payload 的长度 如要写 8 个 byte 有效字节，那第一个字节应写 8，总长 9
        uint16_t __resv2__ :1;
        uint16_t crcEn:1;//开启 CRC
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
        uint16_t scanRssiEn :1;//开始扫描 RSSI
    };

    struct DataRateReg{
        static constexpr RegAddress address = 44;
        uint16_t __resv__ :8;
        uint16_t dataRate:8;//透传速率
    };

    struct FlagReg{
        static constexpr RegAddress address = 48;
        uint16_t __resv__ :5;
        uint16_t fifoFlag:1;
        uint16_t pktFlag:1;
        uint16_t syncWordRecved:1;//表示收到 syncword，只在接收时有效。跳出接收状态时，为 0
        uint16_t framerFlag:6;//Framer 状态
        uint16_t fec23ErrorFlag:1;//FEC23 错误标志位
        uint16_t crcErrorFlag:1;//CRC 错误标志位
    };


    struct FifoPtrReg{
        static constexpr RegAddress address = 52;
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
    };