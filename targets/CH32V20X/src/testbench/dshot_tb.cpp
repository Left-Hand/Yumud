#include "tb.h"
#include "../drivers/Modem/dshot/dshot.hpp"

static constexpr size_t n = 40;
static std::array<uint16_t, 40> data;


[[maybe_unused]] static uint16_t m_crc(uint16_t data_in){
	uint16_t speed_data;
	speed_data = data_in << 5;
	data_in = data_in << 1;
	data_in = (data_in ^ (data_in >> 4) ^ (data_in >> 8)) & 0x0f;
	
	return speed_data | data_in;
}
	

[[maybe_unused]] static void transfer(uint16_t data_in){
	uint8_t i;
	for(i=0;i<16;i++)
	{
		if(data_in & 0x8000)data[i] = 171;
		else data[i] = 81;
		data_in = data_in << 1;
	}
}

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


    auto entry = millis();
    while(millis() - entry < 3000){
        // ch1.enable();
        // ch2.enable();
        ch1 = 0;
        ch2 = 0;

        delay(20);
    }

    delay(200);
    // delay(3000);
    // ch1.init();
    // ch2.init();

    // oc1.init();
    // oc2.init();
    // constexpr real_t base = 0.12;
    // constexpr real_t full = 0.95;
    // constexpr real_t delta = full-  base;
    // ch1 = base;
    // ch2 = base;

    // auto t0 = t;
    // real_t temp = base;
    // while(true){
        // ch2 = 0.2;
        // delay(15);

        // temp = std::max(temp, base + (0.5 + 0.5 * sin((t - t0)/2)) * delta);
        // ch1 = temp;
        // ch2 = temp;
        // delay(20);

        // delay(10);
        // ch1 = 0.6;
        // ch2 = 0.6;
        // oc2 = 0.4;
        // logger.println(temp);
        // delay(20);
    // }
}

void dshot_main(){

    auto & logger = DEBUGGER;
    logger.init(DEBUG_UART_BAUD, CommMethod::Blocking);
    logger.setRadix(10);
    logger.setEps(4);
    AdvancedTimer & timer = timer8;

    // timer.enableArrSync();
    // timer.enableCvrSync();


    timer.init(234, 1);
    auto & oc = timer.oc(1);
    auto & oc2 = timer.oc(2);

    dshot_tb_new(logger,oc, oc2);
    // dshot_tb_old(logger,oc, oc2);

    // while(true);
}