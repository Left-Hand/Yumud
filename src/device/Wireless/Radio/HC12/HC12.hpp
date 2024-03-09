#ifndef __HC12_HPP__

#define __HC12_HPP__

#include "src/bus/uart/uart.hpp"
#include "types/string/String.hpp"
#include "../Radio.hpp"

class HC12:public Printer, public Radio{
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
    Uart & uart;
    GpioBase & set_pin;
    uint16_t timeout = 5;

    void _write(const char & data) override{uart.write(data);}
    void _read(char & data) override{uart.read(data);}

    bool sendAtCommand(const char * token){
        return sendAtCommand(String(token));
    }

    bool sendAtCommand(const String & token){
        set_pin = false;
        delayMicroseconds(1);
        uart.print("AT");
        if(token.length()){
            uart.print('+');
            uart.print(token);
        }
        uart.println();

        String recv = "";
        recv.reserve(3);

        bool is_valid = false;

        uint32_t begin_ms = millis();
        while((millis() - begin_ms < timeout)){
            if(!uart.available()) continue;
            char chr = uart.read();
            if(!chr) break;
            recv += chr;
            if(recv == "OK"){
                while((millis() - begin_ms < timeout)){
                    if(uart.available() && !uart.read()){
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
    HC12(Uart & _uart, GpioBase & _set_pin):uart(_uart), set_pin(_set_pin){;}

    size_t available() override {return uart.available();}
    void init(){;}
    void sleep(){sendAtCommand("SLEEP");}
    void setPower(const Power power){sendAtCommand("P" + String((uint8_t)power));}
    void setPowerMode(const PowerMode power_mode){sendAtCommand("FU" + String((uint8_t)power_mode));}
    void setChannel(const uint16_t _channel) override {sendAtCommand("C" + String((uint8_t)_channel));}
    void setCommBaudRate(const uint16_t baudrate){sendAtCommand("B" + String(baudrate));}
    bool isValid(){return sendAtCommand("");}
};

#endif