#include "usbfs.hpp"

#ifdef CH32V30X

using namespace ymd;

void UsbFS::init(){
    // usb_cdc_init();
}

#ifdef ENABLE_USBFS
UsbFS usbfs;
#endif


#endif