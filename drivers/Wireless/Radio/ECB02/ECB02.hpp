#pragma once

#include "core/string/string_view.hpp"
#include "hal/bus/uart/uart.hpp"

namespace ymd::drivers{


enum class TxPower:uint8_t{
    _n20dBm,
    _n15dBm,
    _n10dBm,
    _n6dBm,
    _n5dBm,
    _n2dBm,
    _0dBm,
    _3dBm,
    _4dBm,
    _8dBm,
};

class AtProtocol{
protected:

    auto write_command(const StringView cmd){
        return write("AT+", cmd, "\r\n");
    }

    template<typename TValue>
    auto write_keyvalue(const StringView key, TValue && value){
        return write("AT+", key, '=', std::forward<TValue>(value), "\r\n");
    }


public:
    auto set_name(const StringView name){
        return write_keyvalue("NAME", name);
    }

    auto set_password(const StringView password){
        return write_keyvalue("PASSWORD", password);
    }

    enum class Role:uint8_t{
        DeterminedByGpio = 0,
        Central = 1,
        Peripheral = 2,
    };

    auto set_role(const Role role){
        return write_keyvalue("ROLE", uint8_t(role));
    }

    auto reset(){
        return write_command("RST");
    }

    auto set_baudrate(uint8_t code){
        return write_keyvalue("UART", code);
    }



    auto validate(){
        return write("AT\r\n");
    }

    auto shutdown(){
        return write_command("OFF");
    }

    auto disconnect(){
        return write_command("DISC");
    }

    auto start_scan(){
        return write_command("SCAN");
    }

    template<typename T>
    auto connect_to(T && x){
        return write_keyvalue("CONNECT", std::forward<T>(x));
    }
    
    template<typename T>
    auto set_passward(T && x){
        return write_keyvalue("PASSWARD", std::forward<T>(x));
    }

    auto set_txpower(TxPower power){
        return write_keyvalue("POWE", uint8_t(power));
    }

    auto resume(){};
private:
    template<typename ... Ts>
    void write(Ts && ... args){
        return; 
    }
};


class ECB02{

};
}