#include "src/testbench/tb.h"
#include "hal/bus/usb/usbfs/usbfs.hpp"

#include "core/clock/clock.hpp"

using namespace ymd;
using namespace ymd::hal;

void usbcdc_tb(UsbFS & fs){
    fs.init();
    while(true){
        static int i = 0;
        fs.println(i++);
        clock::delay(100ms);
    }
}