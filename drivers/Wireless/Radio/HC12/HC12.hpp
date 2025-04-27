#pragma once

#include "hal/bus/uart/uart.hpp"
#include "core/string/string.hpp"
#include "core/clock/clock.hpp"
#include "../Radio.hpp"


namespace ymd::drivers{
class HC12: public Radio{
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

    void write(const char data){
        uart.write(data);
    }
    void read(char & data){
        uart.read1(data);
    }
    size_t available() const{
        return uart.available();
    }
    bool sendAtCommand(const char * token){
        return sendAtCommand(String(token));
    }

    bool sendAtCommand(const String & token){
        set_pin.clr();
        udelay(1);
        uart.writeN("AT", 2);
        if(token.length()){
            uart.write1('+');
            // uart.write1(token);
        }
        uart.writeN("\r\n", 2);

        String recv = "";
        recv.reserve(3);

        bool is_valid = false;

        // uint32_t begin_ms = millis();

        // while((millis() - begin_ms < timeout)){
        //     if(!available()) continue;
        //     char chr;
        //     read(chr);
        //     if(!chr) break;
        //     recv += chr;
        //     if(recv == "OK"){
        //         // while((millis() - begin_ms < timeout)){
        //         //     if(available() && !read()){
        //         //         is_valid = true;
        //         //         goto end_process;
        //         //     }
        //         // }
        //         goto end_process;
        //     }
        // }

        // end_process:
        udelay(1);
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