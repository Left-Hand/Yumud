#pragma once

#include "hal/bus/uart/hw_singleton.hpp"
#include "hal/gpio/gpio_intf.hpp"

#include "core/clock/clock.hpp"


#if 0

namespace ymd::drivers{
class HC12 final{
public:
    enum class PowerMode{
        Low,
        Quiest,
        Normal,
        LongDist
    };

    enum class Power{
        dBmN1 = 1,
        dBm2, dBm5, dBm8, dBm11, dBm14, dBm17, dBm20
    };

protected:
    hal::Uart & uart;
    hal::GpioIntf & set_pin;
    uint16_t timeout = 5;

    size_t available() const{
        return uart.available();
    }
    bool sendAtCommand(const char * token){
        return sendAtCommand(String(token));
    }

    bool sendAtCommand(const String & token){
        set_pin.clr();
        clock::delay(1us);
        uart.write_chars("AT", 2);
        if(token.length()){
            uart.write_char('+');
            // uart.write_char(token);
        }
        uart.write_chars("\r\n", 2);

        String recv = "";
        recv.reserve(3);

        bool is_valid = false;

        // uint32_t begin_ms = millis();



        // end_process:
        clock::delay(1us);
        set_pin.set();
        return is_valid;
    }
public:
    HC12(
        hal::Uart & _uart, 
        hal::GpioIntf & _set_pin = hal::NullGpio)
    :uart(_uart), set_pin(_set_pin){;}

    void init(){;}
    void sleep(){sendAtCommand("SLEEP");}
    // void setPower(const Power power){sendAtCommand("P" + String((uint8_t)power));}
    // void setPowerMode(const PowerMode power_mode){sendAtCommand("FU" + String((uint8_t)power_mode));}
    // void setChannel(const uint16_t _channel) {sendAtCommand("C" + String((uint8_t)_channel));}
    // void setCommBaudRate(const uint16_t baudrate){sendAtCommand("B" + String(baudrate));}
    bool isValid(){return sendAtCommand("");}
};


};

#endif