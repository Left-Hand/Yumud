#include "ad9910.hpp"
#include "core/clock/clock.hpp"
#include "primitive/analog_channel.hpp"

#include "hal/bus/spi/spidrv.hpp"
#include "hal/gpio/gpio_port.hpp"


using namespace ymd::drivers;
using namespace ymd;



static constexpr uint8_t cfr3[4]={0x05,0x0F,0x41,0x32};       									//cfr3控制字  40M输入  25倍频  VC0=101   ICP=001;
  

#define AD9910_PWR 		hal::PB<5>() 
#define DRHOLD 			hal::PB<7>()   
#define DROVER 			hal::PB<8>()   
#define UP_DAT 			hal::PB<9>()    

#define MAS_REST 		hal::PA<2>()   
#define DRCTL  			hal::PA<4>()   
#define OSK 			hal::PA<5>()  
     
#define PROFILE0 		hal::PA<6>()  
#define PROFILE1 		hal::PB<12>() 
#define PROFILE2 		hal::PA<7>()  

#define DLY __nopn(10);
#define UDT()\
UP_DAT=1;\
DLY;\
UP_DAT=0;\

static constexpr real_t magic_1 = real_t(4.294967296);//将输入频率因子分为四个字节  4.294967296=(2^32)/1000000000 （1G 是内部时钟速度）
static constexpr real_t magic_2 = real_t(25.20615385);	   //将输入幅度因子分为两个字节  25.20615385=(2^14)/650

void AD9910::init(void)
{	
	AD9910_PWR.clr();//软件拉低
	
	PROFILE2.clr();
    PROFILE1.clr();
    PROFILE0.clr();
	DRCTL.clr();
    DRHOLD.clr();
	MAS_REST.set(); 
    clock::delay(5ms);
	MAS_REST.clr(); 

    write_burst(0x00, std::span(cfr1));
    write_burst(0x01, std::span(cfr2));
    write_burst(0x02, std::span(cfr3));

	clock::delay(1ms);
}      



hal::HalResult AD9910::write_burst(const uint8_t addr, const std::span<const uint8_t> pbuf){
    __nopn(4);

    UNWRAP_OR_RETURN(spi_drv.write_single<uint8_t>(addr));
    UNWRAP_OR_RETURN(spi_drv.write_burst<uint8_t>(pbuf));

    __nopn(4);

	return hal::HalResult::Ok();
}

hal::HalResult AD9910::write_data(const uint8_t txdat){
    return spi_drv.write_single<uint8_t>(txdat);
}  

    
void AD9910::write_profile(const Profile & profile){

    write_burst(0x0e, profile.as_bytes());

	clock::delay(1ms);
}         


void AD9910::freq_convert(uint32_t Freq)
{
	uint32_t Temp;
	if(Freq > 400000000)
		Freq = 400000000;
	Temp=(uint32_t)(Freq*magic_1); 

	Profile profile;
	
	profile[7]=(uint8_t)Temp;
	profile[6]=(uint8_t)(Temp>>8);
	profile[5]=(uint8_t)(Temp>>16);
	profile[4]=(uint8_t)(Temp>>24);
	write_profile(profile);
}

void AD9910::set_amplitude(uint32_t Amp)
{
	uint32_t Temp;
	Temp = (uint32_t)(Amp*magic_2);
	if(Temp > 0x3fff)
		Temp = 0x3fff;
	Temp &= 0x3fff;

	Profile profile;

	profile[1]=(uint8_t)Temp;
	profile[0]=(uint8_t)(Temp>>8);

	write_profile(profile);
}

void AD9910::write_drg(const DrgParamenter & drgparameter){
	const auto bytes = drgparameter.as_bytes();
    write_burst(0x0b, bytes.subspan(0, 8));
    write_burst(0x0c, bytes.subspan(8, 4));
    write_burst(0x0d, bytes.subspan(16, 4));
	
	clock::delay(1ms);
}         


void AD9910::freq_sweep(uint32_t SweepMinFre, uint32_t SweepMaxFre, uint32_t SweepStepFre, uint32_t SweepTime){


	const uint32_t Temp1 = (uint32_t)(SweepMinFre*magic_1);
	SweepMaxFre = MIN(SweepMaxFre, 400000000);

	const uint32_t  Temp2 = (uint32_t)(SweepMaxFre*magic_1);

	const uint32_t ITemp3 = (uint32_t)(SweepStepFre*magic_1);
	const uint32_t DTemp3 = ITemp3;
	const uint32_t ITemp4 = MIN((uint32_t)SweepTime >> 2, 0xffff); //1GHz/4, 单位：ns
	const uint32_t DTemp4 = ITemp4;
	
	const DrgParamenter drgparameter = [&]() -> DrgParamenter{
		DrgParamenter _drgparameter;
		//扫频上下限
		_drgparameter[7]=(uint8_t)Temp1;
		_drgparameter[6]=(uint8_t)(Temp1>>8);
		_drgparameter[5]=(uint8_t)(Temp1>>16);
		_drgparameter[4]=(uint8_t)(Temp1>>24);
		_drgparameter[3]=(uint8_t)Temp2;
		_drgparameter[2]=(uint8_t)(Temp2>>8);
		_drgparameter[1]=(uint8_t)(Temp2>>16);
		_drgparameter[0]=(uint8_t)(Temp2>>24);

		//频率步进（单位：Hz）
		_drgparameter[15]=(uint8_t)ITemp3;
		_drgparameter[14]=(uint8_t)(ITemp3>>8);
		_drgparameter[13]=(uint8_t)(ITemp3>>16);
		_drgparameter[12]=(uint8_t)(ITemp3>>24);
		_drgparameter[11]=(uint8_t)DTemp3;
		_drgparameter[10]=(uint8_t)(DTemp3>>8);
		_drgparameter[9]=(uint8_t)(DTemp3>>16);
		_drgparameter[8]=(uint8_t)(DTemp3>>24);

		//步进时间间隔（单位：us）
		_drgparameter[19]=(uint8_t)ITemp4;
		_drgparameter[18]=(uint8_t)(ITemp4>>8);
		_drgparameter[17]=(uint8_t)DTemp4;
		_drgparameter[16]=(uint8_t)(DTemp4>>8);

		return _drgparameter;
	}();

	//发送DRG参数
	write_drg(drgparameter);
}

void AD9910::write_ramprofile(void)
{
    write_burst(0x0e, std::span(ramprofile0));
	clock::delay(1ms);
}         

void AD9910::send_sample(const std::span<const uint8_t> pbuf){
    write_burst(0x16, pbuf);
	clock::delay(1ms);
}         

// void AD9910::Square_wave(uint32_t Sample_interval)//方波
// {
// 	uint32_t Temp;
// 	Temp = Sample_interval/4; //1GHz/4, 采样间隔范围：4*(1~65536)ns
// 	if(Temp > 0xffff)
// 		Temp = 0xffff;
// 	ramprofile0[7] = 0x24;
// 	ramprofile0[6] = 0x00;
// 	ramprofile0[5] = 0x00;
// 	ramprofile0[4] = 0xc0;
// 	ramprofile0[3] = 0x0f;
// 	ramprofile0[2] = (uint8_t)Temp;
// 	ramprofile0[1] = (uint8_t)(Temp>>8);
// 	ramprofile0[0] = 0x00;
// 	writeRamprofile();
// 	sendSample();
//  }
