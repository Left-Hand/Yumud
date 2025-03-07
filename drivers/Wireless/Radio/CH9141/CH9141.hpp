#pragma once

#include <optional>
#include <initializer_list>
#include <type_traits>


#include "hal/bus/uart/uart.hpp"
#include "sys/string/string.hpp"
#include "sys/clock/clock.h"
#include "../Radio.hpp"


namespace ymd::drivers{
struct MacAddress{
protected:
    using DataType = std::array<uint8_t, 6>; // Define a type alias for the underlying buffer
    DataType buf; // Buffer to hold the 6 bytes of the MAC address

public:
    // Default constructor: Initializes the MAC address with all zeros
    MacAddress() : buf{} {}

    // // Constructor with initializer list: Allows initialization with a list of bytes
    // explicit MacAddress(const std::initializer_list<uint8_t> &list)
    //     : buf{list} {;}
    //TODO

    // Copy constructor: Initializes the MAC address from another DataType instance
    MacAddress(const DataType &data) : buf(data) {}

    // Access operator for read: Returns the byte at the specified index
    uint8_t operator [](const size_t index) const{return buf[index];}

    // Access operator for write: Allows modifying the byte at the specified index
    uint8_t & operator [](const size_t index) {return buf[index];}
};

class CH9141: public Radio{
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
            delay(1);
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
    hal::GpioIntf & at_gpio_;
    hal::GpioIntf & slp_gpio_;

    bool sendAtCommand(const char * token){
        SetKeeper{at_gpio_};

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


        delayMicroseconds(1);
        at_gpio_ = true;
        return is_valid;
    }

    void entryAt(){
        // setAtCommand("...")
        //TODO
    }
public:
    CH9141(
        hal::Uart & uart, 
        hal::GpioIntf & set_gpio = hal::NullGpio, 
        hal::GpioIntf & slp_gpio = hal::NullGpio)
    :uart_(uart), at_gpio_(set_gpio), slp_gpio_(slp_gpio){;}

    void write(const char data){
        uart_.write(data);
    }

    void write(const char * data){
        uart_.writeN(data, strlen(data));
    }

    void read(char & data){
        uart_.read1(data);
    }

    size_t available() const {
        return uart_.available();
    }

    size_t pending() const {
        return uart_.pending();
    }

    void init(){
        at_gpio_.outpp(HIGH);
        slp_gpio_.outpp(HIGH);
    }

    void reset(){sendAtCommand("RESET");}
    void factory(){sendAtCommand("RELOAD");}
    void chipInfo(){sendAtCommand("SHOW");}
    void save(){sendAtCommand("SAVE");}
    void exit(){sendAtCommand("EXIT");}
    // void setChannel(const uint16_t _channel) override {sendAtCommand(("BCCH=" + String((uint8_t)_channel)).c_str());}
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