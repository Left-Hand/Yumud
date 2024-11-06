#include "sys/core/platform.h"
#include "sys/clock/clock.h"

#include "ch32v30x_usbotg_device.h"
#include "zf_driver_usb_cdc.h"


#ifdef CH32V30X

void usb_cdc_send_pack(const uint8_t *p, const unsigned int length){
    if(length == 0) return;
    uint32_t i = 0;

    for(i=0; i<length; i++)
    {
       pEP2_IN_DataBuf[i] = p[i];
    }
    DevEP2_IN_Deal(length);

    while(!(USBOTG_FS->UEP2_TX_CTRL&USBHD_UEP_T_RES1));

    // bUEP_T_RES1 & bUEP_T_RES0: handshake response type for USB endpoint X transmittal (IN)
    // 00: DATA0 or DATA1 then expecting ACK (ready)
    // 01: DATA0 or DATA1 then expecting no response, time out from host, for non-zero endpoint isochronous transactions
    // 10: NAK (busy)
    // 11: TALL (error)
}


void usb_cdc_init( void )
{
    pEP0_RAM_Addr = EP0_DatabufHD;
    pEP1_RAM_Addr = EP1_DatabufHD;
    pEP2_RAM_Addr = EP2_DatabufHD;
    USBOTG_RCC_Init();
    delay(100);
    USBDeviceInit();
    EXTEN->EXTEN_CTR |= EXTEN_USBD_PU_EN;
    NVIC_EnableIRQ(OTG_FS_IRQn);
}

#endif