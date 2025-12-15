#pragma once

#include "hal/bus/uart/uart.hpp"
#include "hal/gpio/gpio_intf.hpp"

#include "core/clock/clock.hpp"

#include "core/utils/Option.hpp"

#if 0

namespace ymd::drivers{


class CH9141 final{
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

    enum class ErrCode:uint8_t{
        OK,
        BUF,
        PARAM,
        DENIED,
        BUSY
    };

protected:
    struct SetKeeper{
    protected:
        hal::GpioIntf & m_set_gpio;
        void mdelay(){
            clock::delay(1ms);
        }
    public:
        SetKeeper(hal::GpioIntf & set_pin):m_set_gpio(set_pin){
            m_set_gpio.clr();
            mdelay();
        }
        ~SetKeeper(){
            mdelay();
            m_set_gpio.set();
        }
    };

    hal::Uart & uart_;
    Option<hal::GpioIntf &> at_pin_;
    Option<hal::GpioIntf &> slp_pin_;

    bool sendAtCommand(const char * token){
        SetKeeper{at_pin_};

        write("AT");
        if(token[0] != '.'){// string command
            write('+');
            write(token);
        }else{//... command
            write("...");
        }
        write("\r\n");

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
        //         break;
        //     }
        // }


        clock::delay(1us);
        // at_pin_.set_high();
        return is_valid;
    }

    void entryAt(){
        // setAtCommand("...")
        //TODO
    }
public:
    CH9141(
        hal::Uart & uart, 
        Option<hal::GpioIntf &> set_gpio = None, 
        Option<hal::GpioIntf &> slp_gpio = None)
    :uart_(uart), at_pin_(set_gpio), slp_pin_(slp_gpio){;}

    void write(const char data){
        uart_.write(data);
    }

    void write(const char * data){
        uart_.write_chars(data, strlen(data));
    }

    void read(char & data){
        uart_.read_char(data);
    }

    size_t available() const {
        return uart_.available();
    }

    size_t pending() const {
        return uart_.pending();
    }

    void init(){
        // at_pin_.inspect((auto & io){io.outpp(HIGH);});
        // slp_pin_.inspect((auto & io){io.outpp(HIGH);});
    }

    void reset(){sendAtCommand("RESET");}
    void factory(){sendAtCommand("RELOAD");}
    void chipInfo(){sendAtCommand("SHOW");}
    void save(){sendAtCommand("SAVE");}
    void exit(){sendAtCommand("EXIT");}
    void setChannel(const uint16_t _channel) override {}

    // void setMac(){}
    //TODO

    // auto getMacAddress(){return void;}
    //TODO

    int getTxDB(){return -20;}
    void setTxDB(const int db){return;}

    auto getBleStat(){
        return sendAtCommand("BLESTA");
    }
};
};

#endif