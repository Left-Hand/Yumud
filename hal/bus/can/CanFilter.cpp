#include "CanFilter.hpp"
#include "sys/core/platform.h"

using namespace yumud;

[[maybe_unused]]static void CANFilterConfig_List_Extend(uint8_t FGrop,uint32_t Ext_Id1, uint32_t Ext_Id2){
    CAN_FilterInitTypeDef	CAN_FilterInitStructure;

    CAN_FilterInitStructure.CAN_FilterNumber = FGrop;				 //���ù�������0����ΧΪ0~13
    CAN_FilterInitStructure.CAN_FilterMode=CAN_FilterMode_IdList;	 //���ù�������0Ϊ��ʶ���б�ģʽ
    CAN_FilterInitStructure.CAN_FilterScale=CAN_FilterScale_32bit;  //���ù�������0λ��Ϊ32λ

    //���ñ�ʶ���Ĵ���
    CAN_FilterInitStructure.CAN_FilterIdHigh=((Ext_Id1<<3)>>16)&0xffff ;		   //���ñ�ʶ���Ĵ������ֽ�
    CAN_FilterInitStructure.CAN_FilterIdLow=((Ext_Id1<<3)&0xffff)|CAN_Id_Extended;//���ñ�ʶ���Ĵ������ֽ�,CAN_FilterIdLow��IDλ�����������ã��ڴ�ģʽ�²�����Ч��

    //�������μĴ��������ﵱ��ʶ���Ĵ�����
    CAN_FilterInitStructure.CAN_FilterMaskIdHigh=((Ext_Id2<<3)>>16)&0xffff;			//�������μĴ������ֽ�
    CAN_FilterInitStructure.CAN_FilterMaskIdLow=((Ext_Id2<<3)&0xffff)|CAN_Id_Extended; //�������μĴ������ֽ�

    CAN_FilterInitStructure.CAN_FilterFIFOAssignment=CAN_FIFO0; //�˹����������������FIFO0
    CAN_FilterInitStructure.CAN_FilterActivation=ENABLE;		 //����˹�������
    CAN_FilterInit(&CAN_FilterInitStructure);					 //���ù�����
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
    CAN_FilterInitSturcture.CAN_FilterIdLow = 0;
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

// CanFilter::CanFilter(const std::initializer_list<uint16_t> & list){
//     switch(list.size()){
//         case 4:

//             break;
//         default:
//             break;
//     }
// }

void CanFilter::init(){
    init(*this);
}

void CanFilter::init(const CanFilter & filter){
    CAN_Init_Filter(0,0,0, 0xf);
}