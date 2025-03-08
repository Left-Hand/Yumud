#include "src/testbench/tb.h"
#include "hal/bus/usb/usbfs/usbfs.hpp"

#include "sys/clock/clock.h"

void usbcdc_tb(UsbFS & fs){
    fs.init();
    while(true){
        static int i = 0;
        fs.println(i++);
        delay(100);
    }
}