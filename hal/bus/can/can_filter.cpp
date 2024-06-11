#include "can_filter.hpp"

void CANFilterConfig_List_Extend(uint8_t FGrop,uint32_t Ext_Id1, uint32_t Ext_Id2){
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
