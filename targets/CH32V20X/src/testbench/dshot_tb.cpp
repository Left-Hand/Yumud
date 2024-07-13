#include "tb.h"

static constexpr size_t n = 40;
static std::array<uint16_t, 40> data;


[[maybe_unused]] static uint16_t m_crc(uint16_t data_in){
	uint16_t speed_data;
	speed_data = data_in << 5;
	data_in = data_in << 1;
	data_in = (data_in ^ (data_in >> 4) ^ (data_in >> 8)) & 0x0f;
	
	return speed_data | data_in;
}
	

[[maybe_unused]]static void transfer(uint16_t data_in){
	uint8_t i;
	for(i=0;i<16;i++)
	{
		if(data_in & 0x8000)data[i] = 171;
		else data[i] = 81;
		data_in = data_in << 1;
	}
}

class ServoChannel{

};

class DShotChannel:public ServoChannel{
public:

    enum class Command : uint8_t {
        MOTOR_STOP       = 0, // 目前未实施
        BEEP1            = 1, // 在下一个命令之前至少等待蜂鸣声长度（260 毫秒）
        BEEP2            = 2, // 在下一个命令之前至少等待蜂鸣声长度（260 毫秒）
        BEEP3            = 3, // 在下一个命令之前至少等待蜂鸣声长度（260 毫秒）
        BEEP4            = 4, // 在下一个命令之前至少等待蜂鸣声长度（260 毫秒）
        BEEP5            = 5, // 在下一个命令之前至少等待蜂鸣声长度（260 毫秒）
        ESC_INFO         = 6, // 在下一个命令之前等待至少 12 毫秒
        SPIN_DIRECTION_1 = 7, // 需要 6 倍
        SPIN_DIRECTION_2 = 8, // 需要 6 倍
        _3D_MODE_OFF      = 9, // 需要 6 倍
        _3D_MODE_ON       = 10, // 需要 6 倍
        SETTINGS_REQUEST = 11, // 目前未实施
        SAVE_SETTINGS    = 12, // 需要 6 倍，等待至少 35 毫秒，然后再下一条命令
        DSHOT_EXTENDED_TELEMETRY_ENABLE = 13, // 需要 6 倍（仅在启用 EDT 的固件上）
        DSHOT_EXTENDED_TELEMETRY_DISABLE = 14, // 需要 6 倍（仅在启用 EDT 的固件上）
        

        SPIN_DIRECTION_NORMAL = 20, // 需要 6 倍
        SPIN_DIRECTION_REVERSED = 21, // 需要 6 倍
        LED0_ON             = 22, // -
        LED1_ON             = 23, // -
        LED2_ON             = 24, // -
        LED3_ON             = 25, // -
        LED0_OFF            = 26, // -
        LED1_OFF            = 27, // -
        LED2_OFF            = 28, // -
        LED3_OFF            = 29, // -

        AUDIO_STREAM_MODE           = 30, // Audio_Stream模式开/关 目前未实施
        MUTE_MODE                   = 31, // 静音模式开/关 目前未实施
        SIGNAL_LINE_TELEMETRY_DISABLE = 32, // 需要 6 倍。禁用命令 42 到 47
        SIGNAL_LINE_TELEMETRY_ENABLE = 33, // 需要 6 倍。启用命令 42 到 47
        SIGNAL_LINE_CONTINUOUS_ERPM_TELEMETRY = 34, // 启用命令 42 到 47，如果 DShot 帧正常，则发送 eRPM
        SIGNAL_LINE_CONTINUOUS_ERPM_PERIOD_TELEMETRY = 35, // 启用命令 42 到 47，如果 DShot 帧正常，则发送 eRPM 周期

        SIGNAL_LINE_TEMPERATURE_TELEMETRY = 42, // 每 LSB 为 1°C
        SIGNAL_LINE_VOLTAGE_TELEMETRY     = 43, // 每个LSB为10mV，最大值为40.95V
        SIGNAL_LINE_CURRENT_TELEMETRY     = 44, // 每个LSB为100mA，最大409.5A
        SIGNAL_LINE_CONSUMPTION_TELEMETRY = 45, // 每个 LSB 10mAh，最大 40.95Ah
        SIGNAL_LINE_ERPM_TELEMETRY        = 46, // 每个 LSB 100erpm，最大 409500erpm
        SIGNAL_LINE_ERPM_PERIOD_TELEMETRY = 47  // 每个 LSB 16μs，最大 65520μs 待定
    };
protected:
    std::array<uint16_t, 40> buf;
    const uint16_t high_cnt;
    const uint16_t low_cnt;
    TimerOC & oc;
    DmaChannel & dma_channel;

    static uint16_t m_crc(uint16_t data_in){
        uint16_t speed_data;
        speed_data = data_in << 5;
        data_in = data_in << 1;
        data_in = (data_in ^ (data_in >> 4) ^ (data_in >> 8)) & 0x0f;
        return speed_data | data_in;
    }
        

    void update(uint16_t data){
        uint16_t data_in = m_crc(data);
        uint8_t i;
        for(i=0;i<16;i++){
            buf[i] = (data_in & 0x8000) ? high_cnt : low_cnt;
            data_in = data_in << 1;
        }
    }

    void trigger(){
        dma_channel.begin((void *)&oc.cvr(), (void *)buf.begin(), buf.size());
    }


public:
    DShotChannel(TimerOC & _oc):
        high_cnt(_oc.arr() * 2 / 3),
        low_cnt(_oc.arr() * 1 / 3),
        oc(_oc),
        dma_channel(_oc.dma())
        {;}

    DShotChannel(DShotChannel & other) = delete;
    DShotChannel(DShotChannel && other) = delete;

    void init(){
        dma_channel.init(DmaChannel::Mode::toPeriph, DmaChannel::Priority::ultra);
        oc.init();
        oc.enableDma();
        buf.fill(0);
    }
    auto & operator = (const real_t duty){
        update(MAX(int(duty * 4096), 48));
        trigger();
        return *this;
    }
};
[[maybe_unused]] static void dshot_tb_old(OutputStream & logger, TimerOC & oc1, TimerOC & oc2){
    auto setup = [](TimerOC & oc, DmaChannel & channel){
        channel.init(DmaChannel::Mode::toPeriphCircular, DmaChannel::Priority::ultra);
        channel.begin((void *)&oc.cvr(), (void *)data.begin(), data.size());
        oc.init();
        oc.enableDma();
    };

    setup(oc1, dma2Ch3);
    setup(oc2, dma2Ch5);

    while(true){
        transfer(m_crc(0));
        delay(15);
        transfer(m_crc(300));
        delay(10);
        transfer(m_crc(1500));
        logger.println(millis());
    }
}

[[maybe_unused]] static void dshot_tb_new(OutputStream & logger, TimerOC & oc1, TimerOC & oc2){
    DShotChannel ch1{oc1};
    DShotChannel ch2{oc2};

    ch1.init();
    ch2.init();

    while(true){
        ch2 = 0.2;
        delay(15);
        ch1 = 0.4;
        delay(10);
        ch1 = 0.6;
        ch2 = 0.6;
        logger.println(millis());
        delay(20);
    }
}

void dshot_main(){

    auto & logger = uart2;
    logger.init(576000, CommMethod::Blocking);
    logger.setRadix(10);
    logger.setEps(4);
    data.fill(128);
    AdvancedTimer & timer = timer8;
    timer.init(234, 1);
    auto & oc = timer.oc(1);
    auto & oc2 = timer.oc(2);

    dshot_tb_new(logger,oc, oc2);
    // dshot_tb_old(logger,oc, oc2);

    while(true);
}