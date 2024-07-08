#include "tb.h"
#include "hal/bus/usb/usbfs/usbfs.hpp"

void usbcdc_tb(void){
    // usb_cdc_init();

    usbfs.init();
    while(true){
        static int i = 0;
        usbfs.println(i++);
        delay(100);
    }
}