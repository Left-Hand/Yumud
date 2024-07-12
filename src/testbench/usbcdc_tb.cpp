#include "tb.h"
#include "../hal/bus/usb/usbfs/usbfs.hpp"

void usbcdc_tb(UsbFS & fs){
    fs.init();
    while(true){
        static int i = 0;
        fs.println(i++);
        delay(100);
    }
}