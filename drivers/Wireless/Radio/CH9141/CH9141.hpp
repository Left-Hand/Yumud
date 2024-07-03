#pragma once

#include <optional>
#include <initializer_list>
#include <type_traits>


#include "hal/bus/uart/uart.hpp"
#include "types/string/String.hpp"
#include "../Radio.hpp"

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

class CH9141:public IOStream, public Radio{
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
        GpioConcept & m_set_gpio;
        void mdelay(){
            delay(1);
        }
    public:
        SetKeeper(GpioConcept & set_pin):m_set_gpio(set_pin){
            m_set_gpio.clr();
            mdelay();
        }
        ~SetKeeper(){
            mdelay();
            m_set_gpio.set();
        }
    };

    Uart & uart;
    GpioConcept & set_gpio;
    // uint16_t timeout = 5;

    void write(const char data) override{
        uart.write(data);
    }

    void read(char & data) override{
        uart.read(data);
    }

    size_t available() const override{
        return uart.available();
    }


    bool sendAtCommand(const char * token){
        SetKeeper{set_gpio};
    
        print("AT");
        if(token[0] != '.'){// string command
            print('+');
            print(token);
        }else{//... command
            print("...");
        }
        println();

        String recv = "";
        recv.reserve(3);

        bool is_valid = false;
        uint32_t begin_ms = millis();
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
        set_gpio = true;
        return is_valid;
    }

    void entryAt(){
        // setAtCommand("...")
        //TODO
    }
public:
    CH9141(Uart & _uart, GpioConcept & _set_pin = GpioNull):uart(_uart), set_gpio(_set_pin){;}

    void init(){;}
    void reset(){sendAtCommand("RESET");}
    void factory(){sendAtCommand("RELOAD");}
    void chipInfo(){sendAtCommand("SHOW");}
    void save(){sendAtCommand("SAVE");}
    void exit(){sendAtCommand("EXIT");}

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
