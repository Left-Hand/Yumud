#include "ad9854.hpp"
#include "hal/gpio/gpio_intf.hpp"

#include "core/clock/clock.hpp"

using namespace ymd::drivers;
using namespace ymd;

#define AD9854_DEBUG

#ifdef AD9854_DEBUG
#undef AD9854_DEBUG
#define AD9854_DEBUG(...) DEBUG_LOG(__VA_ARGS__)
#else
#define AD9854_DEBUG(...)
#endif



#define PA1				0x00
#define PA2				0x01
#define FTW1			0x02
#define FTW2			0x03
#define DFW				0x04
#define UC				0x05
#define RRC				0x06
#define CTRL			0x07
#define OSKIM			0x08
#define OSKQM			0x09
#define OSKRR			0x0A
#define QDAC			0x0B

#define Freq_mult 1876499.84473770


void AD9854::Init(void){

    ports.IO_RESET.outpp();
    ports.MRESET.outpp();
    ports.UD_CLK.outpp();
    ports.F_B_H.outpp();
    ports.OSK.outpp();

	uint8_t data[4] = {0x00, 0x06, 0x00, 0x60};
	uint8_t freq[6];
	uint8_t shape[2] = {0x0F, 0xFF};
	int i;
	long long ftw;
	ftw = (Freq_mult * 1000.0) + 0.5;
	for(i = 5; i >= 0; i--)	freq[i] = ftw >> ((5 - i) * 8);
	
	
	delay(10);
	ports.UD_CLK = 0;
	ports.MRESET = 1;
	delayMicroseconds(10);
	ports.MRESET = 0;
	
	AD9854::SendData(CTRL, data, 4);
	delay(50);
	ports.UD_CLK = 1;
	ports.UD_CLK = 0;	
	AD9854::SendData(FTW1, freq, 6);
	AD9854::SendData(OSKIM, shape, 2);
	ports.UD_CLK = 1;
	ports.UD_CLK = 0;
}

void AD9854::SendOneByte(uint8_t data){
	spi_drv_.write_single(data);
}

void AD9854::SendData(uint8_t _register, uint8_t* data, uint8_t ByteNum){
	int i;
	ports.IO_RESET = 1;
	delayMicroseconds(1);
	ports.IO_RESET = 0;
	SendOneByte(_register);
	for(i = 0; i < ByteNum; i++)	SendOneByte(data[i]);
}

void AD9854::SetFreq(real_t fre){
	uint8_t freq[6];
	int i;
	int64_t ftw = (Freq_mult * float(fre)) + 0.5;
	for(i = 5; i >= 0; i--)	freq[i] = ftw >> ((5 - i) * 8);
	AD9854::SendData(FTW1, freq, 6);
	ports.UD_CLK = 1;
	ports.UD_CLK = 0;	
}