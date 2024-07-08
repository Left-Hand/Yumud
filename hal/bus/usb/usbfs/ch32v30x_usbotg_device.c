/********************************** (C) COPYRIGHT *******************************
* File Name          : zf_usb_cdc.c
* Author             : WCH
* Version            : V1.0.0
* Date               : 2021/06/06
* Description        : This file provides all the USBOTG firmware functions.
*******************************************************************************/
#include "stdio.h"

#include "ch32v30x_rcc.h"
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
/* 全局变量 */
/* Endpoint Buffer */
__attribute__ ((aligned(4))) UINT8 EP0_DatabufHD[8]; //ep0(64)
__attribute__ ((aligned(4))) UINT8 EP1_DatabufHD[64+64];  //ep1_out(64)+ep1_in(64)
__attribute__ ((aligned(4))) UINT8 EP2_DatabufHD[64+64];  //ep2_out(64)+ep2_in(64)


PUINT8  pEP0_RAM_Addr;                       //ep0(64)
PUINT8  pEP1_RAM_Addr;                       //ep1_out(64)+ep1_in(64)
PUINT8  pEP2_RAM_Addr;                       //ep2_out(64)+ep2_in(64)


const UINT8 *pDescr;
volatile UINT8  USBHD_Dev_SetupReqCode = 0xFF;                                  /* USB2.0高速设备Setup包命令码 */
volatile UINT16 USBHD_Dev_SetupReqLen = 0x00;                                   /* USB2.0高速设备Setup包长度 */
volatile UINT8  USBHD_Dev_SetupReqValueH = 0x00;                                /* USB2.0高速设备Setup包Value高字节 */
volatile UINT8  USBHD_Dev_Config = 0x00;                                        /* USB2.0高速设备配置值 */
volatile UINT8  USBHD_Dev_Address = 0x00;                                       /* USB2.0高速设备地址值 */
volatile UINT8  USBHD_Dev_SleepStatus = 0x00;                                   /* USB2.0高速设备睡眠状态 */
volatile UINT8  USBHD_Dev_EnumStatus = 0x00;                                    /* USB2.0高速设备枚举状态 */
volatile UINT8  USBHD_Dev_Endp0_Tog = 0x01;                                     /* USB2.0高速设备端点0同步标志 */
volatile UINT8  USBHD_Dev_Speed = 0x01;                                         /* USB2.0高速设备速度 */

volatile UINT16 USBHD_Endp1_Up_Flag = 0x00;                                     /* USB2.0高速设备端点1数据上传状态: 0:空闲; 1:正在上传; */
volatile UINT8  USBHD_Endp1_Down_Flag = 0x00;                                   /* USB2.0高速设备端点1下传成功标志 */
volatile UINT8  USBHD_Endp1_Down_Len = 0x00;                                    /* USB2.0高速设备端点1下传长度 */
volatile BOOL   USBHD_Endp1_T_Tog = 0;                                          /* USB2.0高速设备端点1发送tog位翻转 */
volatile BOOL   USBHD_Endp1_R_Tog = 0;

volatile UINT16 USBHD_Endp2_Up_Flag = 0x00;                                     /* USB2.0高速设备端点2数据上传状态: 0:空闲; 1:正在上传; */
volatile UINT16 USBHD_Endp2_Up_LoadPtr = 0x00;                                  /* USB2.0高速设备端点2数据上传装载偏移 */
volatile UINT8  USBHD_Endp2_Down_Flag = 0x00;                                   /* USB2.0高速设备端点2下传成功标志 */

volatile UINT32V Endp2_send_seq=0x00;
volatile UINT8   DevConfig;
volatile UINT8   SetupReqCode;
volatile UINT16  SetupReqLen;


/******************************************************************************/
/* Device Descriptor */
//设备描述符，这是一个全速设备
const UINT8  MyDevDescrHD[] =
{
    0x12,           //设备描述符长度，18字节
    0x01,           //描述符类型，0x01为设备描述符
    0x00,0x02,      //本设备所使用USB版本协议，因为是小端结构，所以低字节在前，即USB1.1版本为0x10,0x01，USB2.0为0x00,0x02
    0x02,           //类代码，CDC类为0x02，CDC类必须在设备描述符中指定设备类型，即0x02，否则，由于在配置集合中有两个接口，而会被系统误认为一个USB复合设备，从而导致设备工作不正常
    //当指定了设备类型为通信设备类后，子类和所使用的协议都必须指定为0.
    0x00,           //子类代码，当类代码bDeviceClass为0时，下面的子类代码bDeviceSubClass也必须为0。
    0x00,           //设备所使用的协议，协议代码由USB协会规定。当该字段为0时，表示设备不使用类所定义的协议。

    DevEP0SIZE,     //端点0的最大包长，可以取值8、16、32、64，此处为64字节
    0x86,0x1a,      //厂商ID
    0x22,0x57,      //产品设备ID
    0x00,0x01,      //设备版本号
    0x01,           //描述厂商的字符串索引值。当该值为0时，表示没有厂商字符串
    0x02,           //描述产品的字符串索引值。当该值为0时，表示没有产品字符串
    0x03,           //描述设备的序列号字符串索引值。当该值为0时，表示没有序列号字符串
    0x01,           //可能的配置数，通常为1
};


/* Configration Descriptor */
//配置描述符
const UINT8  MyCfgDescrHD[] =
{
    //配置描述符（两个接口）
    0x09,          //配置描述符长度，标准USB配置描述符长度为9字节
    0x02,          //描述符类型，配置描述符为0x02
    0x43,0x00,     //配置描述符集合总长度，67字节
    0x02,          //该配置所支持的接口数，2个接口
    0x01,          //表示该配置的值
    0x00,          //描述该配置的字符串的索引值，0x00表示没有字符串
    0xa0,          //描述设备的一些属性，如供电方式和唤醒等，0xA0表示设备总线供电且支持远程唤醒
    0x32,          //设备需要从总线获取的最大电流量，0x32表示最大电流100ma

    //以下为接口0（CDC接口）描述符，接口描述符不能单独返回，必须附着在配置描述符后一并返回
    0x09,          //接口描述符长度，标准的USB接口描述符长度为9字节
    0x04,          //描述符类型，接口描述符为0x04
    0x00,          //该接口的编号，从0开始，此处为0x00
    0x00,          //该接口的备用编号,通常设置为0
    0x01,          //该接口所使用的端点数，0x01表示使用1个端点。如果该字段为0，则表示没有非0端点，只使用默认的控制端点。CDC接口只使用一个中断输入端点
    0x02,          //该接口所使用的类，0x02为CDC类
    0x02,          //该接口所使用的子类，要实现USB转串口，就必须使用Abstract Control Model（抽象控制模型）子类，它的编号为0x02
    0x01,          //该接口所使用的协议，使用Common AT Commands（通用AT命令）协议
    0x00,          //该接口的字符串的索引值，0x00表示没有字符串


    //以下为类特殊接口描述符
    //功能描述符，主要用来描述接口的功能，功能描述符放在CDC接口（主接口）之后
    //Header Functional Descriptor
    0x05,          //该功能描述符的长度，5个字节
    0x24,          //该描述符的类型，固定位0x24（CS_INTERFACE的编码）
    0x00,          //该描述符子类型
    0x10,0x01,     //USB通信设备协议的版本号。此处为USB1.1

    //管理描述符(没有数据类接口)
    //Call Management Functional Descriptor
    0x05,          //该功能描述符的长度，5个字节
    0x24,          //该描述符的类型，固定位0x24（CS_INTERFACE的编码）
    0x01,          //该描述符子类型
    0x00,          //描述设备的能力，只有最低两位D0和D1有意义，其余位为保留值0。D0为0，表示设备自己不处理调用管理，为1则表示自己处理。
    0x00,          //表示选择用来做调用管理的数据类接口编号，由于不使用数据类接口做调用管理，因而该字段设置为0

    //Abstract Control Management Functional Descriptor  抽象控制管理功能描述符
    0x04,          //该功能描述符的长度，4个字节
    0x24,          //该描述符的类型，固定位0x24（CS_INTERFACE的编码）
    0x02,          //该描述符子类型
    0x02,          //描述设备的能力，其中D7-4位为保留位，设置为0，支持Set_Line_Coding、Set_Control_Line_State、Get_Line_Coding请求和Serial_State通知
                   //D0表示是否支持以下请求：Set_Comm_Feature、Clear_Comm_Feature、Get_Comm_Feature,为1表示支持；
                   //D1位表示是否支持Set_Line_Coding、Set_Control_Line_State、Get_Line_Coding请求和Serial_State通知，为1表示支持
                   //D2为表示是否支持Send_Break，为1表示支持
                   //D3表示是否支持Network_Connection通知，为1表示支持

    //Union Functional Descriptor，至少5字节，他描述一组接口之间的关系可以被当作为一个功能单元来看待，这些接口一个作为主接口，其他作为从接口
    0x05,          //该功能描述符的长度，5个字节
    0x24,          //该描述符的类型，固定位0x24（CS_INTERFACE的编码）
    0x06,          //该描述符子类型
    0x00,          //第四字节为主接口编号，此处为编号为0的CDC接口
    0x01,          //第五字节为第一从接口编号，此处为编号1的数据类接口，本例程只有一个从接口

    //接口0（CDC接口）的端点描述符 端点1 仅输入
    0x07,          //端点描述符长度，7字节
    0x05,          //描述符类型，端点描述符为0x05
    0x81,          //该端点的地址，0x81表示端点1作为输入
    0x03,          //该端点的属性。最低两位D1-0表示该端点的传输类型，0为控制传输，1为等时传输，2为批量传输，3为中断传输
    0x40,0x00,     //该端点支持的最大包长度，64字节
    0xFF,          //端点的查询时间

    //以下为接口1（数据接口）描述符 端点2
    //CDC类接口（接口0）是负责管理整个设备的，而真正的串口数据传输是在数据类接口进行的。这里只使用一个数据类接口，编号为1
    0x09,          //接口描述符长度，9字节
    0x04,          //描述符类型，接口描述符为0x04
    0x01,          //该接口的编号，从0开始，此处为0x01
    0x00,          //该接口的备用编号
    0x02,          //该接口所使用的端点数，该接口要使用一对批量传输端点，因而端点数量为2
    0x0a,          //该接口所使用的类，0x0a为CDC数据类
    0x00,          //该接口所使用的子类
    0x00,          //该接口所使用的协议
    0x00,          //该接口的字符串的索引值，0x00表示没有字符串

    //接口1（数据类接口）的端点描述符 端点2
    0x07,          //端点描述符长度，7字节
    0x05,          //描述符类型，端点描述符为0x05
    0x02,          //该端点的地址，0x02表示端点2作为输出
    0x02,          //该端点的属性。最低两位D1-0表示该端点的传输类型，0为控制传输，1为等时传输，2为批量传输，3为中断传输
    0x40,0x00,     //该端点支持的最大包长度，64字节
    0x00,          //端点的查询时间，这里对批量端点无效

    0x07,          //端点描述符长度，7字节
    0x05,          //描述符类型，端点描述符为0x05
    0x82,          //该端点的地址，0x82表示端点2作为输入
    0x02,          //该端点的属性。最低两位D1-0表示该端点的传输类型，0为控制传输，1为等时传输，2为批量传输，3为中断传输
    0x40,0x00,     //该端点支持的最大包长度，64字节
    0x00,          //端点的查询时间，这里对批量端点无效             //端点描述符

};

/* USB报告描述符 */
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

/* USB序列号字符串描述符 */
const UINT8  MySerNumInfoHD[ ] =
{
    /* 0123456789 */
    22,03,48,0,49,0,50,0,51,0,52,0,53,0,54,0,55,0,56,0,57,0
};

/* USB设备限定描述符 */
const UINT8 MyUSBQUADescHD[ ] =
{
    0x0A, 0x06, 0x00, 0x02, 0xFF, 0x00, 0xFF, 0x40, 0x01, 0x00,
};

/* USB全速模式,其他速度配置描述符 */
UINT8 TAB_USB_FS_OSC_DESC[ sizeof( MyCfgDescrHD ) ] =
{
    0x09, 0x07,                                                                 /* 其他部分通过程序复制 */
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

//CDC参数
UINT8 LineCoding[7]={0x00,0xC2,0x01,0x00,0x00,0x00,0x08};   //初始化波特率为57600，1停止位，无校验，8数据位。

#define  SET_LINE_CODING                0x20            //Configures DTE rate, stop-bits, parity, and number-of-character
#define  GET_LINE_CODING                0x21            //This request allows the host to find out the currently configured line coding.
#define  SET_CONTROL_LINE_STATE         0x22            //This request generates RS-232/V.24 style control signals.
#define  UART_REV_LEN                   0x40            //串口接收缓冲区大小

UINT8 Receive_Uart_Buf[UART_REV_LEN];   //串口接收缓冲区
volatile UINT8 Uart_Input_Point = 0;    //循环缓冲区写入指针，总线复位需要初始化为0
volatile UINT8 Uart_Output_Point = 0;   //循环缓冲区取出指针，总线复位需要初始化为0
volatile UINT8 UartByteCount = 0;       //当前缓冲区剩余待取字节数
volatile UINT8 USBByteCount = 0;        //代表USB端点接收到的数据
volatile UINT8 USBBufOutPoint = 0;      //取数据指针
volatile UINT8 UpPoint2_Busy  = 0;      //上传端点是否忙标志

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
* Description    : OTG_FS_IRQHandler OTG设备中断处理函数
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

            /* SETUP包处理 */
            case USBHD_UIS_TOKEN_SETUP:
                /* 打印当前Usbsetup命令  */
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
                /* 判断当前是标准请求还是其他请求 */
                if ( ( pSetupReqPakHD->bRequestType & USB_REQ_TYP_MASK ) != USB_REQ_TYP_STANDARD )  //非标准请求
                {
                    //CDC类请求
                    if(SetupReqCode & 0x20)
                    {
                        switch( SetupReqCode )
                        {
                            case GET_LINE_CODING:   //0x21  currently configured
                                pDescr = LineCoding;
                                len = sizeof(LineCoding);
                                len = SetupReqLen >= DEFAULT_ENDP0_SIZE ? DEFAULT_ENDP0_SIZE : SetupReqLen;  // 本次传输长度
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


//                    /* 其它请求,如类请求,产商请求等 */
//                    if( pSetupReqPakHD->bRequestType & 0x40 )
//                    {
//                        /* 厂商请求 */
//                        switch( pSetupReqPakHD->bRequest )
//                        {
//                            default:
//                                errflag = 0xFF;/* 操作失败 */
//                                break;
//                        }
//                    }
//                    }

                    /* 判断是否可以正常处理 */
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
                    /* 处理标准USB请求包 */
                    switch( SetupReqCode )
                    {
                        case USB_GET_DESCRIPTOR:
                        {
                            switch( ((pSetupReqPakHD->wValue)>>8) )
                            {
                                case USB_DESCR_TYP_DEVICE:
                                    /* 获取设备描述符 */
                                    pDescr = MyDevDescrHD;
                                    len = MyDevDescrHD[0];
                                    break;

                                case USB_DESCR_TYP_CONFIG:
                                    /* 获取配置描述符 */
                                    pDescr = MyCfgDescrHD;
                                    len = MyCfgDescrHD[2];
                                    break;

                                case USB_DESCR_TYP_STRING:
                                    /* 获取字符串描述符 */
                                    switch( (pSetupReqPakHD->wValue)&0xff )
                                    {
                                        case 0:
                                            /* 语言字符串描述符 */
                                        pDescr = MyLangDescrHD;
                                        len = MyLangDescrHD[0];
                                            break;

                                        case 1:
                                            /* USB产商字符串描述符 */
                                            pDescr = MyManuInfoHD;
                                            len = MyManuInfoHD[0];
                                            break;

                                        case 2:
                                            /* USB产品字符串描述符 */
                                            pDescr = MyProdInfoHD;
                                            len = MyProdInfoHD[0];
                                            break;

                                        case 3:
                                            /* USB序列号字符串描述符 */
                                            pDescr = MySerNumInfoHD;
                                            len = sizeof( MySerNumInfoHD );
                                            break;

                                        default:
                                            errflag = 0xFF;
                                            break;
                                    }
                                    break;

                                case USB_DESCR_TYP_REPORT:
                                    /* USB设备报告描述符 */
                                    pDescr = MyReportDescHD;
                                    len = sizeof( MyReportDescHD );
                                    break;

                                case USB_DESCR_TYP_QUALIF:
                                    /* 设备限定描述符 */
                                    pDescr = ( PUINT8 )&MyUSBQUADescHD[ 0 ];
                                    len = sizeof( MyUSBQUADescHD );
                                    break;

                                case USB_DESCR_TYP_SPEED:
                                    /* 其他速度配置描述符 */
                                    /* 其他速度配置描述符 */
                                    if( USBHD_Dev_Speed == 0x00 )
                                    {
                                      /* 全速模式 */
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
                                    /* BOS描述符 */
                                    /* USB2.0设备不支持BOS描述符 */
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
                            /* 设置地址 */
                            SetupReqLen = (pSetupReqPakHD->wValue)&0xff;
                            break;

                        case USB_GET_CONFIGURATION:
                            /* 获取配置值 */
                            pEP0_DataBuf[0] = DevConfig;
                            if ( SetupReqLen > 1 ) SetupReqLen = 1;
                            break;

                        case USB_SET_CONFIGURATION:
                            /* 设置配置值 */
                            DevConfig = (pSetupReqPakHD->wValue)&0xff;
                            break;

                        case USB_CLEAR_FEATURE:
                            /* 清除特性 */
                            if ( ( pSetupReqPakHD->bRequestType & USB_REQ_RECIP_MASK ) == USB_REQ_RECIP_ENDP )
                            {
                                /* 清除端点 */
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
                            /* 设置特性 */
                            if( ( pMySetupReqPakHD->bRequestType & 0x1F ) == 0x00 )
                            {
                                /* 设置设备 */
                                if( pMySetupReqPakHD->wValue == 0x01 )
                                {
                                    if( MyCfgDescrHD[ 7 ] & 0x20 )
                                    {
                                        /* 设置唤醒使能标志 */
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
                                /* 设置端点 */
                                if( pMySetupReqPakHD->wValue == 0x00 )
                                {
                                    /* 设置指定端点STALL */
                                    switch( ( pMySetupReqPakHD->wIndex ) & 0xff )
                                    {
                                        case 0x82:
                                            /* 设置端点2 IN STALL */
                                            USBOTG_FS->UEP2_TX_CTRL = ( USBOTG_FS->UEP2_TX_CTRL &= ~USBHD_UEP_T_RES_MASK ) | USBHD_UEP_T_RES_STALL;
                                            //USBHS->UEP2_CTRL  = ( USBHS->UEP2_CTRL & ~USBHS_EP_T_RES_MASK ) | USBHS_EP_T_RES_STALL;
                                            break;

                                        case 0x02:
                                            /* 设置端点2 OUT Stall */
                                            USBOTG_FS->UEP2_RX_CTRL = ( USBOTG_FS->UEP2_RX_CTRL &= ~USBHD_UEP_R_RES_MASK ) | USBHD_UEP_R_RES_STALL;
                                            //USBHS->UEP2_CTRL  = ( USBHS->UEP2_CTRL & ~USBHS_EP_R_RES_MASK ) | USBHS_EP_R_RES_STALL;
                                            break;

                                        case 0x81:
                                            /* 设置端点1 IN STALL */
                                            USBOTG_FS->UEP1_TX_CTRL = ( USBOTG_FS->UEP1_TX_CTRL &= ~USBHD_UEP_T_RES_MASK ) | USBHD_UEP_T_RES_STALL;
                                            //USBHS->UEP1_CTRL  = ( USBHS->UEP1_CTRL & ~USBHS_EP_T_RES_MASK ) | USBHS_EP_T_RES_STALL;
                                            break;

                                        case 0x01:
                                            /* 设置端点1 OUT STALL */
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
                            /* 获取接口 */
                            pEP0_DataBuf[0] = 0x00;
                            if ( SetupReqLen > 1 ) SetupReqLen = 1;
                            break;

                        case USB_SET_INTERFACE:
                            /* 设置接口 */
                            EP0_DatabufHD[ 0 ] = 0x00;
                            if( USBHD_Dev_SetupReqLen > 1 )
                            {
                                USBHD_Dev_SetupReqLen = 1;
                            }
                            break;

                        case USB_GET_STATUS:
                            /* 根据当前端点实际状态进行应答 */
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

                                    printf( "\nBaud Rate = %d",
                                            (((LINECODINGST *)pEP0_DataBuf)->dataRat[3] <<24) |
                                            (((LINECODINGST *)pEP0_DataBuf)->dataRat[2] <<16) |
                                            (((LINECODINGST *)pEP0_DataBuf)->dataRat[1] <<8) |
                                            (((LINECODINGST *)pEP0_DataBuf)->dataRat[0]));
                                }
                                else
                                {
                                    printf( "\ndataBit = %d",  ((LINECODINGST *)pEP0_DataBuf)->dataBit);
                                    printf( "\nstopBit = %d",  ((LINECODINGST *)pEP0_DataBuf)->stopBit);
                                    printf( "\nparityType = %d",  ((LINECODINGST *)pEP0_DataBuf)->parityType);

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
