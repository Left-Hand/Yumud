/********************************** (C) COPYRIGHT *******************************
* File Name          : zf_usb_cdc.c
* Author             : WCH
* Version            : V1.0.0
* Date               : 2021/06/06
* Description        : This file provides all the USBOTG firmware functions.
*******************************************************************************/
#include "stdio.h"

#ifdef CH32V30X
#include "../sys/core/platform.h"
#include "ch32v30x_usbotg_device.h"


/* Global define */
/* OTH */
#define pMySetupReqPakHD        ((PUSB_SETUP_REQ)EP0_DatabufHD)
#define RepDescSize             62
#define DevEP0SIZE              8
#define PID_OUT                 0
#define PID_SOF                 1
#define PID_IN                  2
#define PID_SETUP               3

typedef struct{
    UINT8 dataRat[4];
    UINT8 stopBit;
    UINT8 parityType;
    UINT8 dataBit;
}LINECODINGST;

/******************************************************************************/
/* ȫ�ֱ��� */
/* Endpoint Buffer */
__attribute__ ((aligned(4))) UINT8 EP0_DatabufHD[8]; //ep0(64)
__attribute__ ((aligned(4))) UINT8 EP1_DatabufHD[64+64];  //ep1_out(64)+ep1_in(64)
__attribute__ ((aligned(4))) UINT8 EP2_DatabufHD[64+64];  //ep2_out(64)+ep2_in(64)


PUINT8  pEP0_RAM_Addr;                       //ep0(64)
PUINT8  pEP1_RAM_Addr;                       //ep1_out(64)+ep1_in(64)
PUINT8  pEP2_RAM_Addr;                       //ep2_out(64)+ep2_in(64)


const UINT8 *pDescr;
volatile UINT8  USBHD_Dev_SetupReqCode = 0xFF;                                  /* USB2.0�����豸Setup�������� */
volatile UINT16 USBHD_Dev_SetupReqLen = 0x00;                                   /* USB2.0�����豸Setup������ */
volatile UINT8  USBHD_Dev_SetupReqValueH = 0x00;                                /* USB2.0�����豸Setup��Value���ֽ� */
volatile UINT8  USBHD_Dev_Config = 0x00;                                        /* USB2.0�����豸����ֵ */
volatile UINT8  USBHD_Dev_Address = 0x00;                                       /* USB2.0�����豸��ֵַ */
volatile UINT8  USBHD_Dev_SleepStatus = 0x00;                                   /* USB2.0�����豸˯��״̬ */
volatile UINT8  USBHD_Dev_EnumStatus = 0x00;                                    /* USB2.0�����豸ö��״̬ */
volatile UINT8  USBHD_Dev_Endp0_Tog = 0x01;                                     /* USB2.0�����豸�˵�0ͬ����־ */
volatile UINT8  USBHD_Dev_Speed = 0x01;                                         /* USB2.0�����豸�ٶ� */

volatile UINT16 USBHD_Endp1_Up_Flag = 0x00;                                     /* USB2.0�����豸�˵�1�����ϴ�״̬: 0:����; 1:�����ϴ�; */
volatile UINT8  USBHD_Endp1_Down_Flag = 0x00;                                   /* USB2.0�����豸�˵�1�´��ɹ���־ */
volatile UINT8  USBHD_Endp1_Down_Len = 0x00;                                    /* USB2.0�����豸�˵�1�´����� */
volatile BOOL   USBHD_Endp1_T_Tog = 0;                                          /* USB2.0�����豸�˵�1����togλ��ת */
volatile BOOL   USBHD_Endp1_R_Tog = 0;

volatile UINT16 USBHD_Endp2_Up_Flag = 0x00;                                     /* USB2.0�����豸�˵�2�����ϴ�״̬: 0:����; 1:�����ϴ�; */
volatile UINT16 USBHD_Endp2_Up_LoadPtr = 0x00;                                  /* USB2.0�����豸�˵�2�����ϴ�װ��ƫ�� */
volatile UINT8  USBHD_Endp2_Down_Flag = 0x00;                                   /* USB2.0�����豸�˵�2�´��ɹ���־ */

volatile UINT32V Endp2_send_seq=0x00;
volatile UINT8   DevConfig;
volatile UINT8   SetupReqCode;
volatile UINT16  SetupReqLen;


/******************************************************************************/
/* Device Descriptor */
//�豸������������һ��ȫ���豸
const UINT8  MyDevDescrHD[] =
{
    0x12,           //�豸���������ȣ�18�ֽ�
    0x01,           //���������ͣ�0x01Ϊ�豸������
    0x00,0x02,      //���豸��ʹ��USB�汾Э�飬��Ϊ��С�˽ṹ�����Ե��ֽ���ǰ����USB1.1�汾Ϊ0x10,0x01��USB2.0Ϊ0x00,0x02
    0x02,           //����룬CDC��Ϊ0x02��CDC��������豸��������ָ���豸���ͣ���0x02���������������ü������������ӿڣ����ᱻϵͳ����Ϊһ��USB�����豸���Ӷ������豸����������
    //��ָ�����豸����Ϊͨ���豸����������ʹ�õ�Э�鶼����ָ��Ϊ0.
    0x00,           //������룬�������bDeviceClassΪ0ʱ��������������bDeviceSubClassҲ����Ϊ0��
    0x00,           //�豸��ʹ�õ�Э�飬Э�������USBЭ��涨�������ֶ�Ϊ0ʱ����ʾ�豸��ʹ�����������Э�顣

    DevEP0SIZE,     //�˵�0��������������ȡֵ8��16��32��64���˴�Ϊ64�ֽ�
    0x86,0x1a,      //����ID
    0x22,0x57,      //��Ʒ�豸ID
    0x00,0x01,      //�豸�汾��
    0x01,           //�������̵��ַ�������ֵ������ֵΪ0ʱ����ʾû�г����ַ���
    0x02,           //������Ʒ���ַ�������ֵ������ֵΪ0ʱ����ʾû�в�Ʒ�ַ���
    0x03,           //�����豸�����к��ַ�������ֵ������ֵΪ0ʱ����ʾû�����к��ַ���
    0x01,           //���ܵ���������ͨ��Ϊ1
};


/* Configration Descriptor */
//����������
const UINT8  MyCfgDescrHD[] =
{
    //�����������������ӿڣ�
    0x09,          //�������������ȣ���׼USB��������������Ϊ9�ֽ�
    0x02,          //���������ͣ�����������Ϊ0x02
    0x43,0x00,     //���������������ܳ��ȣ�67�ֽ�
    0x02,          //��������֧�ֵĽӿ�����2���ӿ�
    0x01,          //��ʾ�����õ�ֵ
    0x00,          //���������õ��ַ���������ֵ��0x00��ʾû���ַ���
    0xa0,          //�����豸��һЩ���ԣ��繩�緽ʽ�ͻ��ѵȣ�0xA0��ʾ�豸���߹�����֧��Զ�̻���
    0x32,          //�豸��Ҫ�����߻�ȡ������������0x32��ʾ������100ma

    //����Ϊ�ӿ�0��CDC�ӿڣ����������ӿ����������ܵ������أ����븽����������������һ������
    0x09,          //�ӿ����������ȣ���׼��USB�ӿ�����������Ϊ9�ֽ�
    0x04,          //���������ͣ��ӿ�������Ϊ0x04
    0x00,          //�ýӿڵı�ţ���0��ʼ���˴�Ϊ0x00
    0x00,          //�ýӿڵı��ñ��,ͨ������Ϊ0
    0x01,          //�ýӿ���ʹ�õĶ˵�����0x01��ʾʹ��1���˵㡣������ֶ�Ϊ0�����ʾû�з�0�˵㣬ֻʹ��Ĭ�ϵĿ��ƶ˵㡣CDC�ӿ�ֻʹ��һ���ж�����˵�
    0x02,          //�ýӿ���ʹ�õ��࣬0x02ΪCDC��
    0x02,          //�ýӿ���ʹ�õ����࣬Ҫʵ��USBת���ڣ��ͱ���ʹ��Abstract Control Model���������ģ�ͣ����࣬���ı��Ϊ0x02
    0x01,          //�ýӿ���ʹ�õ�Э�飬ʹ��Common AT Commands��ͨ��AT���Э��
    0x00,          //�ýӿڵ��ַ���������ֵ��0x00��ʾû���ַ���


    //����Ϊ������ӿ�������
    //��������������Ҫ���������ӿڵĹ��ܣ���������������CDC�ӿڣ����ӿڣ�֮��
    //Header Functional Descriptor
    0x05,          //�ù����������ĳ��ȣ�5���ֽ�
    0x24,          //�������������ͣ��̶�λ0x24��CS_INTERFACE�ı��룩
    0x00,          //��������������
    0x10,0x01,     //USBͨ���豸Э��İ汾�š��˴�ΪUSB1.1

    //����������(û��������ӿ�)
    //Call Management Functional Descriptor
    0x05,          //�ù����������ĳ��ȣ�5���ֽ�
    0x24,          //�������������ͣ��̶�λ0x24��CS_INTERFACE�ı��룩
    0x01,          //��������������
    0x00,          //�����豸��������ֻ�������λD0��D1�����壬����λΪ����ֵ0��D0Ϊ0����ʾ�豸�Լ����������ù�����Ϊ1���ʾ�Լ�������
    0x00,          //��ʾѡ�����������ù�����������ӿڱ�ţ����ڲ�ʹ��������ӿ������ù�����������ֶ�����Ϊ0

    //Abstract Control Management Functional Descriptor  ������ƹ�������������
    0x04,          //�ù����������ĳ��ȣ�4���ֽ�
    0x24,          //�������������ͣ��̶�λ0x24��CS_INTERFACE�ı��룩
    0x02,          //��������������
    0x02,          //�����豸������������D7-4λΪ����λ������Ϊ0��֧��Set_Line_Coding��Set_Control_Line_State��Get_Line_Coding�����Serial_State֪ͨ
                   //D0��ʾ�Ƿ�֧����������Set_Comm_Feature��Clear_Comm_Feature��Get_Comm_Feature,Ϊ1��ʾ֧�֣�
                   //D1λ��ʾ�Ƿ�֧��Set_Line_Coding��Set_Control_Line_State��Get_Line_Coding�����Serial_State֪ͨ��Ϊ1��ʾ֧��
                   //D2Ϊ��ʾ�Ƿ�֧��Send_Break��Ϊ1��ʾ֧��
                   //D3��ʾ�Ƿ�֧��Network_Connection֪ͨ��Ϊ1��ʾ֧��

    //Union Functional Descriptor������5�ֽڣ�������һ��ӿ�֮��Ĺ�ϵ���Ա�����Ϊһ�����ܵ�Ԫ����������Щ�ӿ�һ����Ϊ���ӿڣ�������Ϊ�ӽӿ�
    0x05,          //�ù����������ĳ��ȣ�5���ֽ�
    0x24,          //�������������ͣ��̶�λ0x24��CS_INTERFACE�ı��룩
    0x06,          //��������������
    0x00,          //�����ֽ�Ϊ���ӿڱ�ţ��˴�Ϊ���Ϊ0��CDC�ӿ�
    0x01,          //�����ֽ�Ϊ��һ�ӽӿڱ�ţ��˴�Ϊ���1��������ӿڣ�������ֻ��һ���ӽӿ�

    //�ӿ�0��CDC�ӿڣ��Ķ˵������� �˵�1 ������
    0x07,          //�˵����������ȣ�7�ֽ�
    0x05,          //���������ͣ��˵�������Ϊ0x05
    0x81,          //�ö˵�ĵ�ַ��0x81��ʾ�˵�1��Ϊ����
    0x03,          //�ö˵�����ԡ������λD1-0��ʾ�ö˵�Ĵ������ͣ�0Ϊ���ƴ��䣬1Ϊ��ʱ���䣬2Ϊ�������䣬3Ϊ�жϴ���
    0x40,0x00,     //�ö˵�֧�ֵ��������ȣ�64�ֽ�
    0xFF,          //�˵�Ĳ�ѯʱ��

    //����Ϊ�ӿ�1�����ݽӿڣ������� �˵�2
    //CDC��ӿڣ��ӿ�0���Ǹ�����������豸�ģ��������Ĵ������ݴ�������������ӿڽ��еġ�����ֻʹ��һ��������ӿڣ����Ϊ1
    0x09,          //�ӿ����������ȣ�9�ֽ�
    0x04,          //���������ͣ��ӿ�������Ϊ0x04
    0x01,          //�ýӿڵı�ţ���0��ʼ���˴�Ϊ0x01
    0x00,          //�ýӿڵı��ñ��
    0x02,          //�ýӿ���ʹ�õĶ˵������ýӿ�Ҫʹ��һ����������˵㣬����˵�����Ϊ2
    0x0a,          //�ýӿ���ʹ�õ��࣬0x0aΪCDC������
    0x00,          //�ýӿ���ʹ�õ�����
    0x00,          //�ýӿ���ʹ�õ�Э��
    0x00,          //�ýӿڵ��ַ���������ֵ��0x00��ʾû���ַ���

    //�ӿ�1��������ӿڣ��Ķ˵������� �˵�2
    0x07,          //�˵����������ȣ�7�ֽ�
    0x05,          //���������ͣ��˵�������Ϊ0x05
    0x02,          //�ö˵�ĵ�ַ��0x02��ʾ�˵�2��Ϊ���
    0x02,          //�ö˵�����ԡ������λD1-0��ʾ�ö˵�Ĵ������ͣ�0Ϊ���ƴ��䣬1Ϊ��ʱ���䣬2Ϊ�������䣬3Ϊ�жϴ���
    0x40,0x00,     //�ö˵�֧�ֵ��������ȣ�64�ֽ�
    0x00,          //�˵�Ĳ�ѯʱ�䣬����������˵���Ч

    0x07,          //�˵����������ȣ�7�ֽ�
    0x05,          //���������ͣ��˵�������Ϊ0x05
    0x82,          //�ö˵�ĵ�ַ��0x82��ʾ�˵�2��Ϊ����
    0x02,          //�ö˵�����ԡ������λD1-0��ʾ�ö˵�Ĵ������ͣ�0Ϊ���ƴ��䣬1Ϊ��ʱ���䣬2Ϊ�������䣬3Ϊ�жϴ���
    0x40,0x00,     //�ö˵�֧�ֵ��������ȣ�64�ֽ�
    0x00,          //�˵�Ĳ�ѯʱ�䣬����������˵���Ч             //�˵�������

};

/* USB���������� */
const UINT8  MyReportDescHD[ ] =
{0x14,0x03,0x32,0x00,0x30,0x00,0x31,0x00,0x37,0x00,0x2D,0x00,0x32,0x00,0x2D,0x00,0x32,0x00,0x35,0x00};

/* Language Descriptor */
const UINT8  MyLangDescrHD[] =
{
    0x04, 0x03, 0x09, 0x04
};

/* Manufactor Descriptor */
const UINT8  MyManuInfoHD[] =
{
    0x0E, 0x03, 'w', 0, 'c', 0, 'h', 0, '.', 0, 'c', 0, 'n', 0
};

/* Product Information */
const UINT8  MyProdInfoHD[] =
{
    0x0C, 0x03, 'C', 0, 'H', 0, '3', 0, '0', 0, '7', 0
};

/* USB���к��ַ��������� */
const UINT8  MySerNumInfoHD[ ] =
{
    /* 0123456789 */
    22,03,48,0,49,0,50,0,51,0,52,0,53,0,54,0,55,0,56,0,57,0
};

/* USB�豸�޶������� */
const UINT8 MyUSBQUADescHD[ ] =
{
    0x0A, 0x06, 0x00, 0x02, 0xFF, 0x00, 0xFF, 0x40, 0x01, 0x00,
};

/* USBȫ��ģʽ,�����ٶ����������� */
UINT8 TAB_USB_FS_OSC_DESC[ sizeof( MyCfgDescrHD ) ] =
{
    0x09, 0x07,                                                                 /* ��������ͨ�������� */
};


void OTG_FS_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));

/*******************************************************************************
* Function Name  : USBOTG_FS_DeviceInit
* Description    : Initializes USB device.
* Input          : None
* Return         : None
*******************************************************************************/
void USBDeviceInit( void )
{
    USBOTG_FS->BASE_CTRL = 0x00;

    USBOTG_FS->UEP4_1_MOD = USBHD_UEP4_RX_EN|USBHD_UEP4_TX_EN|USBHD_UEP1_RX_EN|USBHD_UEP1_TX_EN;
    USBOTG_FS->UEP2_3_MOD = USBHD_UEP2_RX_EN|USBHD_UEP2_TX_EN|USBHD_UEP3_RX_EN|USBHD_UEP3_TX_EN;
    USBOTG_FS->UEP5_6_MOD = USBHD_UEP5_RX_EN|USBHD_UEP5_TX_EN|USBHD_UEP6_RX_EN|USBHD_UEP6_TX_EN;
    USBOTG_FS->UEP7_MOD   = USBHD_UEP7_RX_EN|USBHD_UEP7_TX_EN;

    USBOTG_FS->UEP0_DMA = (UINT32)pEP0_RAM_Addr;
    USBOTG_FS->UEP1_DMA = (UINT32)pEP1_RAM_Addr;
    USBOTG_FS->UEP2_DMA = (UINT32)pEP2_RAM_Addr;


    USBOTG_FS->UEP0_RX_CTRL = USBHD_UEP_R_RES_ACK;
    USBOTG_FS->UEP1_RX_CTRL = USBHD_UEP_R_RES_ACK;
    USBOTG_FS->UEP2_RX_CTRL = USBHD_UEP_R_RES_ACK;

    USBOTG_FS->UEP1_TX_LEN = 8;
    USBOTG_FS->UEP2_TX_LEN = 8;


    USBOTG_FS->UEP0_TX_CTRL = USBHD_UEP_T_RES_NAK;
    USBOTG_FS->UEP1_TX_CTRL = USBHD_UEP_T_RES_NAK;
    USBOTG_FS->UEP2_TX_CTRL = USBHD_UEP_T_RES_NAK;


    USBOTG_FS->INT_FG   = 0xFF;
    USBOTG_FS->INT_EN   = USBHD_UIE_SUSPEND | USBHD_UIE_BUS_RST | USBHD_UIE_TRANSFER;
    USBOTG_FS->DEV_ADDR = 0x00;

    USBOTG_FS->BASE_CTRL = USBHD_UC_DEV_PU_EN | USBHD_UC_INT_BUSY | USBHD_UC_DMA_EN;
    USBOTG_FS->UDEV_CTRL = USBHD_UD_PD_DIS|USBHD_UD_PORT_EN;
}
/*******************************************************************************
* Function Name  : USBOTG_RCC_Init
* Description    : USBOTG RCC init
* Input          : None
* Return         : None
*******************************************************************************/
void USBOTG_RCC_Init( void )
{
    RCC_USBCLK48MConfig( RCC_USBCLK48MCLKSource_USBPHY );
    RCC_USBHSPLLCLKConfig( RCC_HSBHSPLLCLKSource_HSE );
    RCC_USBHSConfig( RCC_USBPLL_Div2 );
    RCC_USBHSPLLCKREFCLKConfig( RCC_USBHSPLLCKREFCLK_4M );
    RCC_USBHSPHYPLLALIVEcmd( ENABLE );
    RCC_AHBPeriphClockCmd( RCC_AHBPeriph_USBHS, ENABLE );
    RCC_AHBPeriphClockCmd( RCC_AHBPeriph_OTG_FS, ENABLE );



}


/**********************************************************/
UINT8   Ready = 0;
UINT8   UsbConfig;
//UINT8   SetupReqCode;
//UINT16  SetupReqLen;

//CDC����
UINT8 LineCoding[7]={0x00,0xC2,0x01,0x00,0x00,0x00,0x08};   //��ʼ��������Ϊ57600��1ֹͣλ����У�飬8����λ��

#define  SET_LINE_CODING                0x20            //Configures DTE rate, stop-bits, parity, and number-of-character
#define  GET_LINE_CODING                0x21            //This request allows the host to find out the currently configured line coding.
#define  SET_CONTROL_LINE_STATE         0x22            //This request generates RS-232/V.24 style control signals.
#define  UART_REV_LEN                   0x40            //���ڽ��ջ�������С

UINT8 Receive_Uart_Buf[UART_REV_LEN];   //���ڽ��ջ�����
volatile UINT8 Uart_Input_Point = 0;    //ѭ��������д��ָ�룬���߸�λ��Ҫ��ʼ��Ϊ0
volatile UINT8 Uart_Output_Point = 0;   //ѭ��������ȡ��ָ�룬���߸�λ��Ҫ��ʼ��Ϊ0
volatile UINT8 UartByteCount = 0;       //��ǰ������ʣ���ȡ�ֽ���
volatile UINT8 USBByteCount = 0;        //����USB�˵���յ�������
volatile UINT8 USBBufOutPoint = 0;      //ȡ����ָ��
volatile UINT8 UpPoint2_Busy  = 0;      //�ϴ��˵��Ƿ�æ��־

const UINT8 *pDescr;


/*******************************************************************************
* Function Name  : DevEP1_IN_Deal
* Description    : Device endpoint1 IN.
* Input          : l: IN length(<64B)
* Return         : None
*******************************************************************************/
void DevEP1_IN_Deal( UINT8 l )
{
    USBOTG_FS->UEP1_TX_LEN = l;
    USBOTG_FS->UEP1_TX_CTRL = (USBOTG_FS->UEP1_TX_CTRL & ~USBHD_UEP_T_RES_MASK)| USBHD_UEP_T_RES_ACK;
}

/*******************************************************************************
* Function Name  : DevEP2_IN_Deal
* Description    : Device endpoint2 IN.
* Input          : l: IN length(<64B)
* Return         : None
*******************************************************************************/
void DevEP2_IN_Deal( UINT8 l )
{
    USBOTG_FS->UEP2_TX_LEN = l;
    USBOTG_FS->UEP2_TX_CTRL = (USBOTG_FS->UEP2_TX_CTRL & ~USBHD_UEP_T_RES_MASK)| USBHD_UEP_T_RES_ACK;
}

/*******************************************************************************
* Function Name  : DevEP1_OUT_Deal
* Description    : Deal device Endpoint 1 OUT.
* Input          : l: Data length.
* Return         : None
*******************************************************************************/
void DevEP1_OUT_Deal( UINT8 l )
{
    UINT8 i;

    for(i=0; i<l; i++)
    {
        pEP1_IN_DataBuf[i] = ~pEP1_OUT_DataBuf[i];
    }

    DevEP1_IN_Deal( l );
}

/*******************************************************************************
* Function Name  : DevEP2_OUT_Deal
* Description    : Deal device Endpoint 2 OUT.
* Input          : l: Data length.
* Return         : None
*******************************************************************************/
void DevEP2_OUT_Deal( UINT8 l )
{
    UINT8 i;

    for(i=0; i<l; i++)
    {
        pEP2_IN_DataBuf[i] = ~pEP2_OUT_DataBuf[i];
    }

    DevEP2_IN_Deal( l );
}

/*******************************************************************************
* Function Name  : OTG_FS_IRQHandler
* Description    : OTG_FS_IRQHandler OTG�豸�жϴ�������
* Input          : None
* Return         : None
*******************************************************************************/
void OTG_FS_IRQHandler( void )
{
    UINT8  len, chtype;
    UINT8  intflag, errflag = 0;

    intflag = USBOTG_FS->INT_FG;

    if( intflag & USBHD_UIF_TRANSFER )
    {
        switch ( USBOTG_FS->INT_ST & USBHD_UIS_TOKEN_MASK )
        {

            /* SETUP������ */
            case USBHD_UIS_TOKEN_SETUP:
                /* ��ӡ��ǰUsbsetup����  */
//                printf( "Setup Req :\n" );
//                printf( "%02X ", pSetupReqPakHD->bRequestType );
//                printf( "%02X ", pSetupReqPakHD->bRequest );
//                printf( "%04X ", pSetupReqPakHD->wValue );
//                printf( "%04X ", pSetupReqPakHD->wIndex );
//                printf( "%04X ", pSetupReqPakHD->wLength );
//                printf( "\n" );

                USBOTG_FS->UEP0_TX_CTRL = USBHD_UEP_T_TOG|USBHD_UEP_T_RES_NAK;
                USBOTG_FS->UEP0_RX_CTRL = USBHD_UEP_R_TOG|USBHD_UEP_R_RES_ACK;
                SetupReqLen  = pSetupReqPakHD->wLength;
                SetupReqCode = pSetupReqPakHD->bRequest;
                chtype = pSetupReqPakHD->bRequestType;
                len = 0;
                errflag = 0;
                /* �жϵ�ǰ�Ǳ�׼�������������� */
                if ( ( pSetupReqPakHD->bRequestType & USB_REQ_TYP_MASK ) != USB_REQ_TYP_STANDARD )  //�Ǳ�׼����
                {
                    //CDC������
                    if(SetupReqCode & 0x20)
                    {
                        switch( SetupReqCode )
                        {
                            case GET_LINE_CODING:   //0x21  currently configured
                                pDescr = LineCoding;
                                len = sizeof(LineCoding);
                                len = SetupReqLen >= DEFAULT_ENDP0_SIZE ? DEFAULT_ENDP0_SIZE : SetupReqLen;  // ���δ��䳤��
                                memcpy(pEP0_DataBuf,pDescr,len);
                                SetupReqLen -= len;
                                pDescr += len;
                            break;

                            case SET_CONTROL_LINE_STATE:  //0x22  generates RS-232/V.24 style control signals
                            break;

                            case SET_LINE_CODING:      //0x20  Configure
                            break;

                            default:
                                errflag = 0xFF;
                        }
                    }


//                    /* ��������,��������,��������� */
//                    if( pSetupReqPakHD->bRequestType & 0x40 )
//                    {
//                        /* �������� */
//                        switch( pSetupReqPakHD->bRequest )
//                        {
//                            default:
//                                errflag = 0xFF;/* ����ʧ�� */
//                                break;
//                        }
//                    }
//                    }

                    /* �ж��Ƿ������������ */
                    if( errflag != 0xFF )
                    {
                        if( SetupReqLen > len )
                        {
                            SetupReqLen = len;
                        }
                        len = ( USBHD_Dev_SetupReqLen >= DevEP0SIZE ) ? DevEP0SIZE : USBHD_Dev_SetupReqLen;
                        memcpy( EP0_DatabufHD, pDescr, len );
                        pDescr += len;
                    }
                }
                else
                {
                    /* ������׼USB����� */
                    switch( SetupReqCode )
                    {
                        case USB_GET_DESCRIPTOR:
                        {
                            switch( ((pSetupReqPakHD->wValue)>>8) )
                            {
                                case USB_DESCR_TYP_DEVICE:
                                    /* ��ȡ�豸������ */
                                    pDescr = MyDevDescrHD;
                                    len = MyDevDescrHD[0];
                                    break;

                                case USB_DESCR_TYP_CONFIG:
                                    /* ��ȡ���������� */
                                    pDescr = MyCfgDescrHD;
                                    len = MyCfgDescrHD[2];
                                    break;

                                case USB_DESCR_TYP_STRING:
                                    /* ��ȡ�ַ��������� */
                                    switch( (pSetupReqPakHD->wValue)&0xff )
                                    {
                                        case 0:
                                            /* �����ַ��������� */
                                        pDescr = MyLangDescrHD;
                                        len = MyLangDescrHD[0];
                                            break;

                                        case 1:
                                            /* USB�����ַ��������� */
                                            pDescr = MyManuInfoHD;
                                            len = MyManuInfoHD[0];
                                            break;

                                        case 2:
                                            /* USB��Ʒ�ַ��������� */
                                            pDescr = MyProdInfoHD;
                                            len = MyProdInfoHD[0];
                                            break;

                                        case 3:
                                            /* USB���к��ַ��������� */
                                            pDescr = MySerNumInfoHD;
                                            len = sizeof( MySerNumInfoHD );
                                            break;

                                        default:
                                            errflag = 0xFF;
                                            break;
                                    }
                                    break;

                                case USB_DESCR_TYP_REPORT:
                                    /* USB�豸���������� */
                                    pDescr = MyReportDescHD;
                                    len = sizeof( MyReportDescHD );
                                    break;

                                case USB_DESCR_TYP_QUALIF:
                                    /* �豸�޶������� */
                                    pDescr = ( PUINT8 )&MyUSBQUADescHD[ 0 ];
                                    len = sizeof( MyUSBQUADescHD );
                                    break;

                                case USB_DESCR_TYP_SPEED:
                                    /* �����ٶ����������� */
                                    /* �����ٶ����������� */
                                    if( USBHD_Dev_Speed == 0x00 )
                                    {
                                      /* ȫ��ģʽ */
                                      memcpy( &TAB_USB_FS_OSC_DESC[ 2 ], &MyCfgDescrHD[ 2 ], sizeof( MyCfgDescrHD ) - 2 );
                                      pDescr = ( PUINT8 )&TAB_USB_FS_OSC_DESC[ 0 ];
                                      len = sizeof( TAB_USB_FS_OSC_DESC );
                                    }
                                    else
                                    {
                                      errflag = 0xFF;
                                    }
                                    break;

                                case USB_DESCR_TYP_BOS:
                                    /* BOS������ */
                                    /* USB2.0�豸��֧��BOS������ */
                                    errflag = 0xFF;
                                    break;

                                default :
                                    errflag = 0xff;
                                    break;

                            }

                            if( SetupReqLen>len )   SetupReqLen = len;
                            len = (SetupReqLen >= DevEP0SIZE) ? DevEP0SIZE : SetupReqLen;
                            memcpy( pEP0_DataBuf, pDescr, len );
                            pDescr += len;
                        }
                            break;

                        case USB_SET_ADDRESS:
                            /* ���õ�ַ */
                            SetupReqLen = (pSetupReqPakHD->wValue)&0xff;
                            break;

                        case USB_GET_CONFIGURATION:
                            /* ��ȡ����ֵ */
                            pEP0_DataBuf[0] = DevConfig;
                            if ( SetupReqLen > 1 ) SetupReqLen = 1;
                            break;

                        case USB_SET_CONFIGURATION:
                            /* ��������ֵ */
                            DevConfig = (pSetupReqPakHD->wValue)&0xff;
                            break;

                        case USB_CLEAR_FEATURE:
                            /* ������� */
                            if ( ( pSetupReqPakHD->bRequestType & USB_REQ_RECIP_MASK ) == USB_REQ_RECIP_ENDP )
                            {
                                /* ����˵� */
                                switch( (pSetupReqPakHD->wIndex)&0xff )
                                {
                                case 0x82:
                                    USBOTG_FS->UEP2_TX_CTRL = (USBOTG_FS->UEP2_TX_CTRL & ~( USBHD_UEP_T_TOG|USBHD_UEP_T_RES_MASK )) | USBHD_UEP_T_RES_NAK;
                                    break;

                                case 0x02:
                                    USBOTG_FS->UEP2_RX_CTRL = (USBOTG_FS->UEP2_RX_CTRL & ~( USBHD_UEP_R_TOG|USBHD_UEP_R_RES_MASK )) | USBHD_UEP_R_RES_ACK;
                                    break;

                                case 0x81:
                                    USBOTG_FS->UEP1_TX_CTRL = (USBOTG_FS->UEP1_TX_CTRL & ~( USBHD_UEP_T_TOG|USBHD_UEP_T_RES_MASK )) | USBHD_UEP_T_RES_NAK;
                                    break;

                                case 0x01:
                                    USBOTG_FS->UEP1_RX_CTRL = (USBOTG_FS->UEP1_RX_CTRL & ~( USBHD_UEP_R_TOG|USBHD_UEP_R_RES_MASK )) | USBHD_UEP_R_RES_ACK;
                                    break;

                                default:
                                    errflag = 0xFF;
                                    break;

                                }
                            }
                            else    errflag = 0xFF;
                            break;

                        case USB_SET_FEATURE:
                            /* �������� */
                            if( ( pMySetupReqPakHD->bRequestType & 0x1F ) == 0x00 )
                            {
                                /* �����豸 */
                                if( pMySetupReqPakHD->wValue == 0x01 )
                                {
                                    if( MyCfgDescrHD[ 7 ] & 0x20 )
                                    {
                                        /* ���û���ʹ�ܱ�־ */
                                        USBHD_Dev_SleepStatus = 0x01;
                                    }
                                    else
                                    {
                                        errflag = 0xFF;
                                    }
                                }
                                else
                                {
                                    errflag = 0xFF;
                                }
                            }
                            else if( ( pMySetupReqPakHD->bRequestType & 0x1F ) == 0x02 )
                            {
                                /* ���ö˵� */
                                if( pMySetupReqPakHD->wValue == 0x00 )
                                {
                                    /* ����ָ���˵�STALL */
                                    switch( ( pMySetupReqPakHD->wIndex ) & 0xff )
                                    {
                                        case 0x82:
                                            /* ���ö˵�2 IN STALL */
                                            USBOTG_FS->UEP2_TX_CTRL = ( USBOTG_FS->UEP2_TX_CTRL &= ~USBHD_UEP_T_RES_MASK ) | USBHD_UEP_T_RES_STALL;
                                            //USBHS->UEP2_CTRL  = ( USBHS->UEP2_CTRL & ~USBHS_EP_T_RES_MASK ) | USBHS_EP_T_RES_STALL;
                                            break;

                                        case 0x02:
                                            /* ���ö˵�2 OUT Stall */
                                            USBOTG_FS->UEP2_RX_CTRL = ( USBOTG_FS->UEP2_RX_CTRL &= ~USBHD_UEP_R_RES_MASK ) | USBHD_UEP_R_RES_STALL;
                                            //USBHS->UEP2_CTRL  = ( USBHS->UEP2_CTRL & ~USBHS_EP_R_RES_MASK ) | USBHS_EP_R_RES_STALL;
                                            break;

                                        case 0x81:
                                            /* ���ö˵�1 IN STALL */
                                            USBOTG_FS->UEP1_TX_CTRL = ( USBOTG_FS->UEP1_TX_CTRL &= ~USBHD_UEP_T_RES_MASK ) | USBHD_UEP_T_RES_STALL;
                                            //USBHS->UEP1_CTRL  = ( USBHS->UEP1_CTRL & ~USBHS_EP_T_RES_MASK ) | USBHS_EP_T_RES_STALL;
                                            break;

                                        case 0x01:
                                            /* ���ö˵�1 OUT STALL */
                                            USBOTG_FS->UEP1_RX_CTRL = ( USBOTG_FS->UEP1_RX_CTRL &= ~USBHD_UEP_R_RES_MASK ) | USBHD_UEP_R_RES_STALL;
                                            //USBHS->UEP1_CTRL  = ( USBHS->UEP1_CTRL & ~USBHS_EP_R_RES_MASK ) | USBHS_EP_R_RES_STALL;
                                            break;

                                        default:
                                            errflag = 0xFF;
                                            break;
                                    }
                                }
                                else
                                {
                                    errflag = 0xFF;
                                }
                            }
                            else
                            {
                                errflag = 0xFF;
                            }
                            break;

                        case USB_GET_INTERFACE:
                            /* ��ȡ�ӿ� */
                            pEP0_DataBuf[0] = 0x00;
                            if ( SetupReqLen > 1 ) SetupReqLen = 1;
                            break;

                        case USB_SET_INTERFACE:
                            /* ���ýӿ� */
                            EP0_DatabufHD[ 0 ] = 0x00;
                            if( USBHD_Dev_SetupReqLen > 1 )
                            {
                                USBHD_Dev_SetupReqLen = 1;
                            }
                            break;

                        case USB_GET_STATUS:
                            /* ���ݵ�ǰ�˵�ʵ��״̬����Ӧ�� */
                            EP0_DatabufHD[ 0 ] = 0x00;
                            EP0_DatabufHD[ 1 ] = 0x00;
                            if( pMySetupReqPakHD->wIndex == 0x81 )
                            {
                                if( ( USBOTG_FS->UEP1_TX_CTRL & USBHD_UEP_T_RES_MASK ) == USBHD_UEP_T_RES_STALL )
                                {
                                    EP0_DatabufHD[ 0 ] = 0x01;
                                }
                            }
                            else if( pMySetupReqPakHD->wIndex == 0x01 )
                            {
                                if( ( USBOTG_FS->UEP1_RX_CTRL & USBHD_UEP_R_RES_MASK ) == USBHD_UEP_R_RES_STALL )
                                {
                                    EP0_DatabufHD[ 0 ] = 0x01;
                                }
                            }
                            else if( pMySetupReqPakHD->wIndex == 0x82 )
                            {
                                if( ( USBOTG_FS->UEP2_TX_CTRL & USBHD_UEP_T_RES_MASK ) == USBHD_UEP_T_RES_STALL )
                                {
                                    EP0_DatabufHD[ 0 ] = 0x01;
                                }
                            }
                            else if( pMySetupReqPakHD->wIndex == 0x02 )
                            {
                                if( ( USBOTG_FS->UEP2_RX_CTRL & USBHD_UEP_R_RES_MASK ) == USBHD_UEP_R_RES_STALL )
                                {
                                    EP0_DatabufHD[ 0 ] = 0x01;
                                }
                            }
                            if( USBHD_Dev_SetupReqLen > 2 )
                            {
                                USBHD_Dev_SetupReqLen = 2;
                            }
                            break;

                        default:
                            errflag = 0xFF;
                            break;
                    }
                }
                if( errflag == 0xff)
                {
//                    printf("uep0 stall\n");
                    USBOTG_FS->UEP0_TX_CTRL = USBHD_UEP_T_TOG|USBHD_UEP_T_RES_STALL;
                    USBOTG_FS->UEP0_RX_CTRL = USBHD_UEP_R_TOG|USBHD_UEP_R_RES_STALL;
                }
                else
                {
                    if( chtype & 0x80 )
                    {
                        len = (SetupReqLen>DevEP0SIZE) ? DevEP0SIZE : SetupReqLen;
                        SetupReqLen -= len;
                    }
                    else  len = 0;

                    USBOTG_FS->UEP0_TX_LEN  = len;
                    USBOTG_FS->UEP0_TX_CTRL = USBHD_UEP_T_TOG|USBHD_UEP_T_RES_ACK;
                    USBOTG_FS->UEP0_RX_CTRL = USBHD_UEP_R_TOG|USBHD_UEP_R_RES_ACK;
                }
                break;

            case USBHD_UIS_TOKEN_IN:
                switch ( USBOTG_FS->INT_ST & ( USBHD_UIS_TOKEN_MASK | USBHD_UIS_ENDP_MASK ) )
                {
                    case USBHD_UIS_TOKEN_IN:
                        switch( SetupReqCode )
                        {
                            case USB_GET_DESCRIPTOR:
                                    len = SetupReqLen >= DevEP0SIZE ? DevEP0SIZE : SetupReqLen;
                                    memcpy( pEP0_DataBuf, pDescr, len );
                                    SetupReqLen -= len;
                                    pDescr += len;
                                    USBOTG_FS->UEP0_TX_LEN   = len;
                                    USBOTG_FS->UEP0_TX_CTRL ^= USBHD_UEP_T_TOG;
                                    break;

                            case USB_SET_ADDRESS:
                                    USBOTG_FS->DEV_ADDR = (USBOTG_FS->DEV_ADDR&USBHD_UDA_GP_BIT) | SetupReqLen;
                                    USBOTG_FS->UEP0_TX_CTRL = USBHD_UEP_T_RES_NAK;
                                    USBOTG_FS->UEP0_RX_CTRL = USBHD_UEP_R_RES_ACK;
                                    break;

                            default:
                                    USBOTG_FS->UEP0_TX_LEN = 0;
                                    USBOTG_FS->UEP0_TX_CTRL = USBHD_UEP_T_RES_NAK;
                                    USBOTG_FS->UEP0_RX_CTRL = USBHD_UEP_R_RES_ACK;
                                    break;

                        }
                        break;

                case USBHD_UIS_TOKEN_IN | 1:
                    USBOTG_FS->UEP1_TX_CTRL  = (USBHD_UEP1_TX_CTRL & ~USBHD_UEP_T_RES_MASK) | USBHD_UEP_T_RES_ACK;
                    USBOTG_FS->UEP1_TX_CTRL ^= USBHD_UEP_T_TOG;
                    break;

                case USBHD_UIS_TOKEN_IN | 2:
                    USBOTG_FS->UEP2_TX_CTRL ^= USBHD_UEP_T_TOG;
                    USBOTG_FS->UEP2_TX_CTRL = (USBOTG_FS->UEP2_TX_CTRL & ~USBHD_UEP_T_RES_MASK) | USBHD_UEP_T_RES_NAK;
                    break;


                default :
                    break;

                }
                break;

            case USBHD_UIS_TOKEN_OUT:

                switch ( USBOTG_FS->INT_ST & ( USBHD_UIS_TOKEN_MASK | USBHD_UIS_ENDP_MASK ) )
                {
                    case USBHD_UIS_TOKEN_OUT:
                        switch( SetupReqCode )
                        {
                            case 0x20:
                                if((((LINECODINGST *)pEP0_DataBuf)->dataRat[0] || ((LINECODINGST *)pEP0_DataBuf)->dataRat[1] ||
                                  ((LINECODINGST *)pEP0_DataBuf)->dataRat[2] || ((LINECODINGST *)pEP0_DataBuf)->dataRat[3]))
                                {

                                    // printf( "\nBaud Rate = %d",
                                    //         (((LINECODINGST *)pEP0_DataBuf)->dataRat[3] <<24) |
                                    //         (((LINECODINGST *)pEP0_DataBuf)->dataRat[2] <<16) |
                                    //         (((LINECODINGST *)pEP0_DataBuf)->dataRat[1] <<8) |
                                    //         (((LINECODINGST *)pEP0_DataBuf)->dataRat[0]));
                                }
                                else
                                {
                                    // printf( "\ndataBit = %d",  ((LINECODINGST *)pEP0_DataBuf)->dataBit);
                                    // printf( "\nstopBit = %d",  ((LINECODINGST *)pEP0_DataBuf)->stopBit);
                                    // printf( "\nparityType = %d",  ((LINECODINGST *)pEP0_DataBuf)->parityType);

                                }
                               break;
                  }


                    len = USBOTG_FS->RX_LEN;
                    break;

                    case USBHD_UIS_TOKEN_OUT | 1:
                        if ( USBOTG_FS->INT_ST & USBHD_UIS_TOG_OK )
                        {
                            USBOTG_FS->UEP1_RX_CTRL ^= USBHD_UEP_R_TOG;
                            len = USBOTG_FS->RX_LEN;
//                            printf( "point 1 len %d\n", len );
                            DevEP1_OUT_Deal( len );
                        }
                        break;

                    case USBHD_UIS_TOKEN_OUT | 2:
                        if ( USBOTG_FS->INT_ST & USBHD_UIS_TOG_OK )
                        {
                            USBOTG_FS->UEP2_RX_CTRL ^= USBHD_UEP_R_TOG;
                            len = USBOTG_FS->RX_LEN;
//                            printf( "point 2 len %d\n", len );
                            DevEP2_OUT_Deal( len );
                        }
                        break;
                }

                break;

            case USBHD_UIS_TOKEN_SOF:

                break;

            default :
                break;

        }

        USBOTG_FS->INT_FG = USBHD_UIF_TRANSFER;
    }
    else if( intflag & USBHD_UIF_BUS_RST )
    {
        USBOTG_FS->DEV_ADDR = 0;

        USBOTG_FS->UEP0_RX_CTRL = USBHD_UEP_R_RES_ACK;
        USBOTG_FS->UEP1_RX_CTRL = USBHD_UEP_R_RES_ACK;
        USBOTG_FS->UEP2_RX_CTRL = USBHD_UEP_R_RES_ACK;

        USBOTG_FS->UEP0_TX_CTRL = USBHD_UEP_T_RES_NAK;
        USBOTG_FS->UEP1_TX_CTRL = USBHD_UEP_T_RES_NAK;
        USBOTG_FS->UEP2_TX_CTRL = USBHD_UEP_T_RES_NAK;

        USBOTG_FS->INT_FG |= USBHD_UIF_BUS_RST;
    }
    else if( intflag & USBHD_UIF_SUSPEND )
    {
        if ( USBOTG_FS->MIS_ST & USBHD_UMS_SUSPEND ) {;}
        else{;}
        USBOTG_FS->INT_FG = USBHD_UIF_SUSPEND;
    }
    else
    {
        USBOTG_FS->INT_FG = intflag;
    }
}

#endif