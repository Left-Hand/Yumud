#include "can_filter.hpp"

[[maybe_unused]]static void CANFilterConfig_List_Extend(uint8_t FGrop,uint32_t Ext_Id1, uint32_t Ext_Id2){
    CAN_FilterInitTypeDef	CAN_FilterInitStructure;

    CAN_FilterInitStructure.CAN_FilterNumber = FGrop;				 //设置过滤器组0，范围为0~13
    CAN_FilterInitStructure.CAN_FilterMode=CAN_FilterMode_IdList;	 //设置过滤器组0为标识符列表模式
    CAN_FilterInitStructure.CAN_FilterScale=CAN_FilterScale_32bit;  //设置过滤器组0位宽为32位

    //设置标识符寄存器
    CAN_FilterInitStructure.CAN_FilterIdHigh=((Ext_Id1<<3)>>16)&0xffff ;		   //设置标识符寄存器高字节
    CAN_FilterInitStructure.CAN_FilterIdLow=((Ext_Id1<<3)&0xffff)|CAN_Id_Extended;//设置标识符寄存器低字节,CAN_FilterIdLow的ID位可以随意设置，在此模式下不会有效。

    //设置屏蔽寄存器，这里当标识符寄存器用
    CAN_FilterInitStructure.CAN_FilterMaskIdHigh=((Ext_Id2<<3)>>16)&0xffff;			//设置屏蔽寄存器高字节
    CAN_FilterInitStructure.CAN_FilterMaskIdLow=((Ext_Id2<<3)&0xffff)|CAN_Id_Extended; //设置屏蔽寄存器低字节

    CAN_FilterInitStructure.CAN_FilterFIFOAssignment=CAN_FIFO0; //此过滤器组关联到接收FIFO0
    CAN_FilterInitStructure.CAN_FilterActivation=ENABLE;		 //激活此过滤器组
    CAN_FilterInit(&CAN_FilterInitStructure);					 //设置过滤器
}

static void CAN_Init_Filter(uint16_t id1, uint16_t mask1, uint16_t id2, uint16_t mask2){
    CAN_FilterInitTypeDef CAN_FilterInitSturcture = {0};

    CAN_FilterInitSturcture.CAN_FilterNumber = 1;

    CAN_FilterInitSturcture.CAN_FilterMode = CAN_FilterMode_IdMask;
    CAN_FilterInitSturcture.CAN_FilterScale = CAN_FilterScale_16bit;

    // CAN_FilterInitSturcture.CAN_FilterIdLow  = id1 << 5;
    // CAN_FilterInitSturcture.CAN_FilterMaskIdLow = mask2 << 5;
    // CAN_FilterInitSturcture.CAN_FilterIdHigh = id2 << 5;
    // CAN_FilterInitSturcture.CAN_FilterMaskIdHigh = mask2 << 5;
    CAN_FilterInitSturcture.CAN_FilterIdLow  = 0;
    CAN_FilterInitSturcture.CAN_FilterMaskIdLow = 0;
    CAN_FilterInitSturcture.CAN_FilterIdHigh = 0;
    CAN_FilterInitSturcture.CAN_FilterMaskIdHigh = 0;

    CAN_FilterInitSturcture.CAN_FilterFIFOAssignment = CAN_Filter_FIFO0;
    CAN_FilterInitSturcture.CAN_FilterActivation = ENABLE;
    CAN_FilterInitSturcture.CAN_FilterScale = CAN_FilterScale_16bit;

    CAN_FilterInit(&CAN_FilterInitSturcture);

    CAN_FilterInitSturcture.CAN_FilterNumber = 0;
    CAN_FilterInitSturcture.CAN_FilterFIFOAssignment = CAN_Filter_FIFO1;
    CAN_FilterInit(&CAN_FilterInitSturcture);
}

void CanFilter::init(){
    init(*this);
}

void CanFilter::init(const CanFilter & filter){
    CAN_Init_Filter(0,0,0, 0xf);
}