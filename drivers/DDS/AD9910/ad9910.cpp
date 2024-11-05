#include "ad9910.hpp"


using namespace yumud::drivers;
using namespace yumud;



scexpr uint8_t cfr3[4]={0x05,0x0F,0x41,0x32};       									//cfr3控制字  40M输入  25倍频  VC0=101   ICP=001;
  

#define AD9910_PWR 	portB[5] 
#define DRHOLD 			portB[7]   
#define DROVER 			portB[8]   
#define UP_DAT 			portB[9]    

#define MAS_REST 		portA[2]   
#define DRCTL  			portA[4]   
#define OSK 			portA[5]  
     
#define PROFILE0 		portA[6]  
#define PROFILE1 		portB[12] 
#define PROFILE2 		portA[7]  

#define DLY __nopn(10);
#define UDT()\
UP_DAT=1;\
DLY;\
UP_DAT=0;\



void AD9910::init(void)
{	
	AD9910_PWR = 0;//软件拉低
	
	PROFILE2 = 0;
    PROFILE1 = 0;
    PROFILE0 = 0;
	DRCTL=0;
    DRHOLD=0;
	MAS_REST=1; 
    delay(5);
	MAS_REST=0; 

    writeReg(0x00, cfr1, 4);
    writeReg(0x01, cfr2, 4);
    writeReg(0x02, cfr3, 4);

	delay(1);
}      



void AD9910::writeReg(const uint8_t addr, const uint8_t * data, const size_t len){
    __nopn(4);
    spi_drv.writeSingle(addr);
    spi_drv.writeMulti<uint8_t>(data, len);
    __nopn(4);
}

void AD9910::writeData(const uint8_t txdat){
    spi_drv.writeSingle(txdat);
}  

    
void AD9910::writeProfile(void){

    writeReg(0x0e, profile11, 8);

	delay(1);
}         


void AD9910::freqConvert(uint32_t Freq)
{
	uint32_t Temp;
	if(Freq > 400000000)
		Freq = 400000000;
	Temp=(uint32_t)Freq*4.294967296; //将输入频率因子分为四个字节  4.294967296=(2^32)/1000000000 （1G 是内部时钟速度）
	profile11[7]=(uint8_t)Temp;
	profile11[6]=(uint8_t)(Temp>>8);
	profile11[5]=(uint8_t)(Temp>>16);
	profile11[4]=(uint8_t)(Temp>>24);
	writeProfile();
}

void AD9910::setAmplitude(uint32_t Amp)
{
	uint32_t Temp;
	Temp = (uint32_t)Amp*25.20615385;	   //将输入幅度因子分为两个字节  25.20615385=(2^14)/650
	if(Temp > 0x3fff)
		Temp = 0x3fff;
	Temp &= 0x3fff;
	profile11[1]=(uint8_t)Temp;
	profile11[0]=(uint8_t)(Temp>>8);
	writeProfile();
}

void AD9910::writeDrg(void){
    writeReg(0x0b, drgparameter, 8);
    writeReg(0x0c, drgparameter + 8, 4);
    writeReg(0x0d, drgparameter + 16, 4);
	
	delay(1);
}         


void AD9910::freqSweep(uint32_t SweepMinFre, uint32_t SweepMaxFre, uint32_t SweepStepFre, uint32_t SweepTime){
	uint32_t Temp1, Temp2, ITemp3, DTemp3, ITemp4, DTemp4;
	Temp1 = (uint32_t)SweepMinFre*4.294967296;
	if(SweepMaxFre > 400000000)
		SweepMaxFre = 400000000;
	Temp2 = (uint32_t)SweepMaxFre*4.294967296;
	if(SweepStepFre > 400000000)
		SweepStepFre = 400000000;
	ITemp3 = (uint32_t)SweepStepFre*4.294967296;
	DTemp3 = ITemp3;
	ITemp4 = (uint32_t)SweepTime/4; //1GHz/4, 单位：ns
	if(ITemp4 > 0xffff)
		ITemp4 = 0xffff;
	DTemp4 = ITemp4;
	
	//扫频上下限
	drgparameter[7]=(uint8_t)Temp1;
	drgparameter[6]=(uint8_t)(Temp1>>8);
	drgparameter[5]=(uint8_t)(Temp1>>16);
	drgparameter[4]=(uint8_t)(Temp1>>24);
	drgparameter[3]=(uint8_t)Temp2;
	drgparameter[2]=(uint8_t)(Temp2>>8);
	drgparameter[1]=(uint8_t)(Temp2>>16);
	drgparameter[0]=(uint8_t)(Temp2>>24);
	//频率步进（单位：Hz）
	drgparameter[15]=(uint8_t)ITemp3;
	drgparameter[14]=(uint8_t)(ITemp3>>8);
	drgparameter[13]=(uint8_t)(ITemp3>>16);
	drgparameter[12]=(uint8_t)(ITemp3>>24);
	drgparameter[11]=(uint8_t)DTemp3;
	drgparameter[10]=(uint8_t)(DTemp3>>8);
	drgparameter[9]=(uint8_t)(DTemp3>>16);
	drgparameter[8]=(uint8_t)(DTemp3>>24);
	//步进时间间隔（单位：us）
	drgparameter[19]=(uint8_t)ITemp4;
	drgparameter[18]=(uint8_t)(ITemp4>>8);
	drgparameter[17]=(uint8_t)DTemp4;
	drgparameter[16]=(uint8_t)(DTemp4>>8);
	//发送DRG参数
	writeDrg();
}

void AD9910::writeRamprofile(void)
{
    writeReg(0x0e, ramprofile0, 8);
	delay(1);
}         

void AD9910::sendSample(const uint8_t * data, const size_t len)
{
    writeReg(0x16, data, len);
	delay(1);
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

 #undef DLY