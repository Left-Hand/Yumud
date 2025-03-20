#include "usbfs.hpp"

#ifdef CH32V30X

using namespace ymd;

void UsbFS::init(){
    // usb_cdc_init();
}
void UsbFS::write(const char data){
    // usb_cdc_send_pack((const uint8_t *)&data, 1);
}

void UsbFS::write(const char * data_ptr, const size_t len){
    size_t i = 0;
    size_t length = len;
    while(length){
        if(length >= 63){
            // usb_cdc_send_pack((const uint8_t *)data_ptr + i, 63);
            i += 63;
            length -= 63;
        }
        else{
            // usb_cdc_send_pack((const uint8_t *)data_ptr + i, length);
            length = 0;
        }
    }
}


#ifdef ENABLE_USBFS
UsbFS usbfs;
#endif


#endif