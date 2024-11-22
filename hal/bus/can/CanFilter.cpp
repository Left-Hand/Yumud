#include "CanFilter.hpp"
#include "sys/core/platform.h"

using namespace ymd;



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

[[maybe_unused]]static void CAN_Init_Filter(uint16_t id1, uint16_t mask1, uint16_t id2, uint16_t mask2){
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

    // CAN_FilterInit(&CAN_FilterInitSturcture);

    CAN_FilterInitSturcture.CAN_FilterNumber = 0;
    CAN_FilterInitSturcture.CAN_FilterFIFOAssignment = CAN_Filter_FIFO1;
    CAN_FilterInit(&CAN_FilterInitSturcture);
}

[[maybe_unused]]static void CANFilterConfig_Scale32_IdList(void)  
{  
//   CAN_FilterInitTypeDef  sFilterConfig;  
//   uint32_t StdId =0x321;                //这里写入两个CAN ID，一个位标准CAN ID  
//   uint32_t ExtId =0x1800f001;           //一个位扩展CAN ID  
    
//   sFilterConfig.CAN_FilterNumber = 0;               //使用过滤器0  
//   sFilterConfig.CAN_FilterMode = CAN_FILTERMODE_IDLIST;     //设为列表模式  
//   sFilterConfig.CAN_FilterScale = CAN_FILTERSCALE_32BIT;    //配置为32位宽  
//   sFilterConfig.CAN_FilterHigh = StdId<<5;          //基本ID放入到STID中  
//   sFilterConfig.FilterIdLow = 0|CAN_ID_STD;         //设置IDE位为0  
//   sFilterConfig.FilterMaskIdHigh = ((ExtId<<3)>>16)&0xffff;  
//   sFilterConfig.FilterMaskIdLow = (ExtId<<3)&0xffff|CAN_ID_EXT;   //设置IDE位为1  
//   sFilterConfig.FilterFIFOAssignment = 0;           //接收到的报文放入到FIFO0中  
//   sFilterConfig.FilterActivation = ENABLE;  
//   sFilterConfig.BankNumber = 14;  
    
//   if(HAL_CAN_ConfigFilter(&hcan1, &sFilterConfig) != HAL_OK)  
//   {  
//     Error_Handler();  
//   }  
}  

[[maybe_unused]]static void CANFilterConfig_Scale16_IdList(void)  
{  
//   CAN_FilterInitTypeDef  sFilterConfig;  
//   uint32_t StdId1 =0x123;                       //这里采用4个标准CAN ID作为例子  
//   uint32_t StdId2 =0x124;  
//   uint32_t StdId3 =0x125;  
//   uint32_t StdId4 =0x126;  
    
//   sFilterConfig.CAN_FilterNumber = 1;               //使用过滤器1  
//   sFilterConfig.CAN_FilterMode = CAN_FILTERMODE_IDLIST;     //设为列表模式  
//   sFilterConfig.CAN_FilterScale = CAN_FILTERSCALE_16BIT;    //位宽设置为16位  
//   sFilterConfig.CAN_FilterHigh = StdId1<<5;  //4个标准CAN ID分别放入到4个存储中  
//   sFilterConfig.FilterIdLow = StdId2<<5;  
//   sFilterConfig.FilterMaskIdHigh = StdId3<<5;  
//   sFilterConfig.FilterMaskIdLow = StdId4<<5;  
//   sFilterConfig.FilterFIFOAssignment = 0;           //接收到的报文放入到FIFO0中  
//   sFilterConfig.FilterActivation = ENABLE;  
//   sFilterConfig.BankNumber = 14;  
    
//   if(HAL_CAN_ConfigFilter(&hcan1, &sFilterConfig) != HAL_OK)  
//   {  
//     Error_Handler();  
//   }  
}  

[[maybe_unused]]static void CANFilterConfig_Scale32_IdMask_StandardIdOnly(void)  
{  
//   CAN_FilterInitTypeDef  sFilterConfig;  
//   uint16_t StdIdArray[10] ={0x7e0,0x7e1,0x7e2,0x7e3,0x7e4,  
//                                 0x7e5,0x7e6,0x7e7,0x7e8,0x7e9}; //定义一组标准CAN ID  
//   uint16_t      mask,num,tmp,i;  
    
//   sFilterConfig.CAN_FilterNumber = 2;               //使用过滤器2  
//   sFilterConfig.CAN_FilterMode = CAN_FILTERMODE_IDMASK;     //配置为掩码模式  
//   sFilterConfig.CAN_FilterScale = CAN_FILTERSCALE_32BIT;    //设置为32位宽  
//   sFilterConfig.CAN_FilterHigh =(StdIdArray[0]<<5);     //验证码可以设置为StdIdArray[]数组中任意一个，这里使用StdIdArray[0]作为验证码  
//   sFilterConfig.FilterIdLow =0;  
    
//   mask =0x7ff;                      //下面开始计算屏蔽码  
//   num =sizeof(StdIdArray)/sizeof(StdIdArray[0]);  
//   for(i =0; i<num; i++)      //屏蔽码位StdIdArray[]数组中所有成员的同或结果  
//   {  
//     tmp =StdIdArray[i] ^ (~StdIdArray[0]);  //所有数组成员与第0个成员进行同或操作  
//     mask &=tmp;  
//   }  
//   sFilterConfig.FilterMaskIdHigh =(mask<<5);  
//   sFilterConfig.FilterMaskIdLow =0|0x02;        //只接收数据帧  
    
//   sFilterConfig.FilterFIFOAssignment = 0;       //设置通过的数据帧进入到FIFO0中  
//   sFilterConfig.FilterActivation = ENABLE;  
//   sFilterConfig.BankNumber = 14;  
    
//   if(HAL_CAN_ConfigFilter(&hcan1, &sFilterConfig) != HAL_OK)  
//   {  
//     Error_Handler();  
//   }  
}  

[[maybe_unused]]static void CANFilterConfig_Scale32_IdMask_ExtendIdOnly(void)  
{  
//   CAN_FilterInitTypeDef  sFilterConfig;  
//   //定义一组扩展CAN ID用来测试  
// uint32_t ExtIdArray[10] ={0x1839f101,0x1835f102,0x1835f113,0x1835f124,0x1835f105,  
//                             0x1835f106,0x1835f107,0x1835f108,0x1835f109,0x1835f10A};  
//   uint32_t      mask,num,tmp,i;  
    
//   sFilterConfig.CAN_FilterNumber = 3;                   //使用过滤器3  
//   sFilterConfig.CAN_FilterMode = CAN_FILTERMODE_IDMASK;         //配置为掩码模式  
//   sFilterConfig.CAN_FilterScale = CAN_FILTERSCALE_32BIT;        //设为32位宽  
//   sFilterConfig.CAN_FilterHigh =((ExtIdArray[0]<<3) >>16) &0xffff;//数组任意一个成员都可以作为验证码  
//   sFilterConfig.FilterIdLow =((ExtIdArray[0]<<3)&0xffff) | CAN_ID_EXT;  
    
//   mask =0x1fffffff;  
//   num =sizeof(ExtIdArray)/sizeof(ExtIdArray[0]);  
//   for(i =0; i<num; i++)              //屏蔽码位数组各成员相互同或的结果  
//   {  
//     tmp =ExtIdArray[i] ^ (~ExtIdArray[0]);  //都与第一个数据成员进行同或操作  
//     mask &=tmp;  
//   }  
//   mask <<=3;                                  //对齐寄存器  
//   sFilterConfig.FilterMaskIdHigh = (mask>>16)&0xffff;  
//   sFilterConfig.FilterMaskIdLow = (mask&0xffff)|0x02;       //只接收数据帧  
//   sFilterConfig.FilterFIFOAssignment = 0;  
//   sFilterConfig.FilterActivation = ENABLE;  
//   sFilterConfig.BankNumber = 14;  
    
//   if(HAL_CAN_ConfigFilter(&hcan1, &sFilterConfig) != HAL_OK)  
//   {  
//     Error_Handler();  
//   }  
}  


// CanFilter::CanFilter(const uint32_t id, const uint32_t mask) : id16{id, CanID16(0)}, mask16{mask, CanID16(0)}{

// }

void CanFilter::set(const std::initializer_list<CanID16> & list){
    switch(list.size()){
        default:
            HALT;
        case 2:
            id16[0] =       *std::next(list.begin(), 0);
            id16[1] =       *std::next(list.begin(), 1);
            mask16[0] =     *std::next(list.begin(), 2);
            mask16[1] =     *std::next(list.begin(), 3);

            break;
    }
    is32 = false;
    islist = true;

    apply();
}



void CanFilter::set(const std::initializer_list<CanID32> & list){
    switch(list.size()){
        default:
            HALT;
        case 2:
            id32 =      *list.begin();
            mask32 =    *std::next(list.begin());
            break;
    }
    is32 = true;
    islist = true;

    apply();
}

// CanFilter CanFilter::conv_copy(const bool rmt) const {
//     auto ret = this->copy();
//     // if(is32){
//     //     ret.id32.rtr = 1;
//     // }else{
//     //     ret.id16[0].rtr = 1;
//     //     ret.id16[1].rtr = 1;
//     // }
//     return ret;
// }

void CanFilter::set(const CanID16 & id, const CanID16 & mask){
    set(id, mask, 0, 0);
}

void CanFilter::set(const CanID16 & id1, const CanID16 & mask1, const CanID16 & id2, const CanID16 & mask2){
    id16[0] = id1;
    id16[1] = id2;
    mask16[0] = mask1;
    mask16[1] = mask2;

    is32 = false;
    islist = false;

    apply();
}

void CanFilter::set(const CanID32 & id, const CanID32 & mask){
    id32 = id;
    mask32 = mask;

    is32 = true;
    islist = false;

    apply();
}


void CanFilter::init()
{
    init(*this);
}

void CanFilter::init(const CanFilter & filter){
    CAN_Init_Filter(0,0,0, 0xf);
}

void CanFilter::apply(){
    CAN_FilterInitTypeDef CAN_FilterInitSturcture = {0};

    CAN_FilterInitSturcture.CAN_FilterNumber = 1;

    CAN_FilterInitSturcture.CAN_FilterMode = islist ? CAN_FilterMode_IdList : CAN_FilterMode_IdMask;
    CAN_FilterInitSturcture.CAN_FilterScale = is32 ? CAN_FilterScale_32bit : CAN_FilterScale_16bit;

    CAN_FilterInitSturcture.CAN_FilterIdLow = id16[0];
    CAN_FilterInitSturcture.CAN_FilterMaskIdLow = mask16[0];
    CAN_FilterInitSturcture.CAN_FilterIdHigh = id16[1];
    CAN_FilterInitSturcture.CAN_FilterMaskIdHigh = mask16[1];

    CAN_FilterInitSturcture.CAN_FilterFIFOAssignment = CAN_Filter_FIFO0;
    CAN_FilterInitSturcture.CAN_FilterActivation = ENABLE;

    CAN_FilterInitSturcture.CAN_FilterNumber = idx;

    CAN_FilterInitSturcture.CAN_FilterFIFOAssignment = CAN_Filter_FIFO1;
    CAN_FilterInit(&CAN_FilterInitSturcture);
    
    CAN_FilterInitSturcture.CAN_FilterFIFOAssignment = CAN_Filter_FIFO0;
    CAN_FilterInit(&CAN_FilterInitSturcture);
}