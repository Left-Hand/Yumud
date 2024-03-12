#ifndef __HC12_HPP__

#define __HC12_HPP__

#include "src/bus/uart/uart.hpp"
#include "types/string/String.hpp"
#include "../Radio.hpp"

class HC12:public Uart, public Radio{
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
    GpioBase & set_pin;
    uint16_t timeout = 5;

    bool sendAtCommand(const char * token){
        return sendAtCommand(String(token));
    }

    bool sendAtCommand(const String & token){
        set_pin = false;
        delayMicroseconds(1);
        print("AT");
        if(token.length()){
            print('+');
            print(token);
        }
        println();

        String recv = "";
        recv.reserve(3);

        bool is_valid = false;

        uint32_t begin_ms = millis();
        while((millis() - begin_ms < timeout)){
            if(!available()) continue;
            char chr = read();
            if(!chr) break;
            recv += chr;
            if(recv == "OK"){
                while((millis() - begin_ms < timeout)){
                    if(available() && !read()){
                        is_valid = true;
                        goto end_process;
                    }
                }
                goto end_process;
            }
        }

        end_process:
        delayMicroseconds(1);
        set_pin = true;
        return is_valid;
    }
public:
    HC12(Uart & _uart, GpioBase & _set_pin):Uart(_uart), set_pin(_set_pin){;}

    void init(){;}
    void sleep(){sendAtCommand("SLEEP");}
    void setPower(const Power power){sendAtCommand("P" + String((uint8_t)power));}
    void setPowerMode(const PowerMode power_mode){sendAtCommand("FU" + String((uint8_t)power_mode));}
    void setChannel(const uint16_t _channel) override {sendAtCommand("C" + String((uint8_t)_channel));}
    void setCommBaudRate(const uint16_t baudrate){sendAtCommand("B" + String(baudrate));}
    bool isValid(){return sendAtCommand("");}
};

#endif