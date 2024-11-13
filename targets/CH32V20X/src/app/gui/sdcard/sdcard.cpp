#include "sdcard.hpp"

using namespace yumud;
using namespace yumud::drivers;

void SDcard::init(){
//    unsigned char retry,temp;   
//    unsigned char i;   
//    unsigned char CMD[] = {0x40,0x00,0x00,0x00,0x00,0x95};   
//    SD_Port_Init(); //初始化驱动端口   
      
//    Init_Flag=1; //将初始化标志置1   
  
//    for (i=0;i<0x0f;i++)    
//    {   
//       Write_Byte_SD(0xff); //发送至少74个时钟信号   
//    }   
    
//    //向SD卡发送CMD0   
//    retry=0;   
//    do  
//    { //为了能够成功写入CMD0,在这里写200次   
//      temp=Write_Command_SD(CMD);   
//      retry++;   
//      if(retry==200)    
//      { //超过200次   
//        return(INIT_CMD0_ERROR);//CMD0 Error!   
//      }   
//    }    
//    while(temp!=1);  //回应01h，停止写入   
      
//    //发送CMD1到SD卡   
//    CMD[0] = 0x41; //CMD1   
//    CMD[5] = 0xFF;   
//    retry=0;   
//    do  
//    { //为了能成功写入CMD1,写100次   
//      temp=Write_Command_SD(CMD);   
//      retry++;   
//      if(retry==100)    
//      { //超过100次   
//        return(INIT_CMD1_ERROR);//CMD1 Error!   
//      }   
//    }    
//    while(temp!=0);//回应00h停止写入   
      
//    Init_Flag=0; //初始化完毕，初始化标志清零   
      
//    SPI_CS=1;  //片选无效   
//    return(0); //初始化成功   

}

// unsigned char SD_Read_Sector(unsigned long sector,unsigned char *buffer)   
// {     
//    unsigned char retry;   
//    //命令16   
//    unsigned char CMD[] = {0x51,0x00,0x00,0x00,0x00,0xFF};    
//    unsigned char temp;   
      
//    //地址变换   由逻辑块地址转为字节地址   
//    sector = sector << 9; //sector = sector * 512   
  
//    CMD[1] = ((sector & 0xFF000000) >>24 );   
//    CMD[2] = ((sector & 0x00FF0000) >>16 );   
//    CMD[3] = ((sector & 0x0000FF00) >>8 );   
  
//    //将命令16写入SD卡   
//    retry=0;   
//    do  
//    {  //为了保证写入命令  一共写100次   
//       temp=Write_Command_MMC(CMD);   
//       retry++;   
//       if(retry==100)    
//       {   
//         return(READ_BLOCK_ERROR); //block write Error!   
//       }   
//    }   
//    while(temp!=0);    
         
//    //Read Start Byte form MMC/SD-Card (FEh/Start Byte)   
//    //Now data is ready,you can read it out.   
//    while (Read_Byte_MMC() != 0xfe);   
//    readPos=0;   
//   SD_get_data(512,buffer) ;  //512字节被读出到buffer中   
//  return 0;   
// }   
// 其中SD_get_data函数如下：   
// //----------------------------------------------------------------------------   
//     获取数据到buffer中   
// //----------------------------------------------------------------------------   
// void SD_get_data(unsigned int Bytes,unsigned char *buffer)    
// {   
//    unsigned int j;   
//    for (j=0;j<Bytes;j++)   
//       *buffer++ = Read_Byte_SD();   
// }   
  

//   //--------------------------------------------------------------------------------------------   
//     写512个字节到SD卡的某一个扇区中去   返回0说明写入成功   
// //--------------------------------------------------------------------------------------------   
// unsigned char SD_write_sector(unsigned long addr,unsigned char *Buffer)   
// {     
//    unsigned char tmp,retry;   
//    unsigned int i;   
//    //命令24   
//    unsigned char CMD[] = {0x58,0x00,0x00,0x00,0x00,0xFF};    
//    addr = addr << 9; //addr = addr * 512   
    
//    CMD[1] = ((addr & 0xFF000000) >>24 );   
//    CMD[2] = ((addr & 0x00FF0000) >>16 );   
//    CMD[3] = ((addr & 0x0000FF00) >>8 );   
  
//    //写命令24到SD卡中去   
//    retry=0;   
//    do  
//    {  //为了可靠写入，写100次   
//       tmp=Write_Command_SD(CMD);   
//       retry++;   
//       if(retry==100)    
//       {    
//         return(tmp); //send commamd Error!   
//       }   
//    }   
//    while(tmp!=0);    
      
  
//    //在写之前先产生100个时钟信号   
//    for (i=0;i<100;i++)   
//    {   
//       Read_Byte_SD();   
//    }   
    
//    //写入开始字节   
//    Write_Byte_MMC(0xFE);    
    
//    //现在可以写入512个字节   
//    for (i=0;i<512;i++)   
//    {   
//       Write_Byte_MMC(*Buffer++);    
//    }   
  
//    //CRC-Byte    
//    Write_Byte_MMC(0xFF); //Dummy CRC   
//    Write_Byte_MMC(0xFF); //CRC Code   
      
       
//    tmp=Read_Byte_MMC();   // read response   
//    if((tmp & 0x1F)!=0x05) // 写入的512个字节是未被接受   
//    {   
//      SPI_CS=1;   
//      return(WRITE_BLOCK_ERROR); //Error!   
//    }   
//    //等到SD卡不忙为止   
// //因为数据被接受后，SD卡在向储存阵列中编程数据   
//    while (Read_Byte_MMC()!=0xff){};   
    
//    //禁止SD卡   
//    SPI_CS=1;   
//    return(0);//写入成功   
// }   