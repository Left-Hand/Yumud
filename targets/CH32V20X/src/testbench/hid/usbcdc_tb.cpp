#include "src/testbench/tb.h"
#include "hal/usb/usbfs/usbfs.hpp"

#include "core/clock/clock.hpp"

using namespace ymd;


void usbcdc_tb(UsbFS & fs){
    fs.init();
    while(true){
        static int i = 0;
        fs.println(i++);
        clock::delay(100ms);
    }
}