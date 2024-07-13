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

class DshotChannel:public ServoChannel{
protected:
    std::array<uint16_t, 40> buf;
public:

};
void dshot_tb(OutputStream & logger, TimerOC & oc1, TimerOC & oc2){
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
        delay(150);
        transfer(m_crc(300));
        delay(100);
        transfer(m_crc(1500));
        logger.println(millis());
    }
}


void dshot_main(){

    auto & logger = uart2;
    logger.init(576000, CommMethod::Blocking);
    logger.setRadix(10);
    logger.setEps(4);

    AdvancedTimer & timer = timer8;
    timer.init(234, 1);
    auto & oc = timer.oc(1);
    auto & oc2 = timer.oc(2);

    dshot_tb(logger,oc, oc2);
}