#include "fdc2x1x.hpp"

using namespace ymd::drivers;


#ifdef FDC2X1X_DEBUG
#define FDC2X1X_DEBUG(...) DEBUG_LOG(__VA_ARGS__)
#else
#define FDC2X1X_DEBUG(...)
#endif



void FDC2X1X::init(){
    write_reg(0x08,0x8329);   //(CHx_RCOUNT*16)/55M ==9.76ms，,每10ms左右可以读一次值
	write_reg(0x09,0x8329);
    write_reg(0x0A,0x8329);	
	write_reg(0x0B,0x8329);

	write_reg(0x10,0x000A);  //设置4个通道最小稳定时间
	write_reg(0x11,0x000A);
	write_reg(0x12,0x000A);
	write_reg(0x13,0x000A);
	
    write_reg(0x14,0x1001); //时钟除以1，设置传感器频率在0.01M到8.5M之间
	write_reg(0x15,0x1001);
	write_reg(0x16,0x1001);
	write_reg(0x17,0x1001);
	
	write_reg(0x19,0x0000); //不设置中断标志位
	write_reg(0x1B,0xC20D);//使能0,1,2,3通道，且带宽设置为10M

	write_reg(0x1E,0x8000); //设置4个通道的驱动电流
    write_reg(0x1F,0x8000);
	write_reg(0x20,0x8000);
	write_reg(0x21,0x8000);


	write_reg(0x1A,0x1401);	//使能FDC2214,且取内部时钟为参考时钟
}