/*********************************************************************************************************************
* CH32V307VCT6 Opensourec Library ????CH32V307VCT6 ???????????????? SDK ??????????????
* Copyright (c) 2022 SEEKFREE ?????
*
* ???????CH32V307VCT6 ???????????
*
* CH32V307VCT6 ????? ?????????
* ?????????????????????????? GPL??GNU General Public License???? GNU??��????????????????
* ?? GPL ???3?��?? GPL3.0????????????�ʦ�?????��?????��?????/???????
*
* ????????????????????????????????��???????�ʦ�???
* ?????????????????????????????????
* ?????????��? GPL
*
* ?????????????????????????? GPL ?????
* ?????��??????<https://www.gnu.org/licenses/>
*
* ?????????
* ?????????? GPL3.0 ????????��?? ?????????????????��
* ?????????????? libraries/doc ???????? GPL3_permission_statement.txt ?????
* ??????????? libraries ??????? ???????????? LICENSE ???
* ?????��??��??????????? ?????????????????????????????????????????
*
* ???????          zf_driver_usb_cdc
* ???????          ??????????????
* ?��???          ?? libraries/doc ??????? version ??? ?��???
* ????????          MounRiver Studio V1.8.1
* ??????          CH32V307VCT6
* ????????          https://seekfree.taobao.com/
*
* ?????
* ????                                      ????                             ???
* 2022-09-15        ??W            first version
********************************************************************************************************************/

#include "../sys/core/platform.h"
#include "zf_driver_usb_cdc.h"

#include "../sys/kernel/clock.h"

#ifdef CH32V30X
//-------------------------------------------------------------------------------------------------------------------
// ???????     USB_CDC??????? ??
// ???????     *p          ?????????????
// ???????     length      ????????????( length ???��??64)
// ???????     void
// ??????
//-------------------------------------------------------------------------------------------------------------------
void usb_cdc_send_pack(const uint8_t *p, const size_t length)
{
    if(length == 0) return;
    uint32_t i = 0;

    for(i=0; i<length; i++)
    {
       pEP2_IN_DataBuf[i] = p[i];
    }
    DevEP2_IN_Deal(length);

    //??????????
    while(!(USBOTG_FS->UEP2_TX_CTRL&USBHD_UEP_T_RES1));

    // bUEP_T_RES1 & bUEP_T_RES0: handshake response type for USB endpoint X transmittal (IN)
    // 00: DATA0 or DATA1 then expecting ACK (ready)
    // 01: DATA0 or DATA1 then expecting no response, time out from host, for non-zero endpoint isochronous transactions
    // 10: NAK (busy)
    // 11: TALL (error)
}


//-------------------------------------------------------------------------------------------------------------------
// ???????     USB?��????��?????
// ???????     void          ??
// ???????     void
// ??????
//-------------------------------------------------------------------------------------------------------------------
void usb_cdc_init( void )
{
    // ????????????
    pEP0_RAM_Addr = EP0_DatabufHD;
    pEP1_RAM_Addr = EP1_DatabufHD;
    pEP2_RAM_Addr = EP2_DatabufHD;
    // ???usb???
    USBOTG_RCC_Init();
    // system_delay_ms(100);
    delay(100);
    // usb?��?????
    USBDeviceInit();
    EXTEN->EXTEN_CTR |= EXTEN_USBD_PU_EN;
    // ???usb?��?
    NVIC_EnableIRQ(OTG_FS_IRQn);
}

#endif