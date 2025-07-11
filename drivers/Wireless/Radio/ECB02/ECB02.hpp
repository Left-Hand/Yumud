#pragma once

#include "core/string/string_view.hpp"
#include "hal/bus/uart/uart.hpp"

namespace ymd::drivers{

class AtProtocol{
protected:

    void write_command(const StringView cmd){
        return write("AT+", cmd, "\r\n");
    }

    template<typename TValue>
    void write_keyvalue(const StringView key, TValue && value){
        return write("AT+", key, '=', std::forward<TValue>(value), "\r\n");
    }
public:
    void set_name(const StringView name){
        return write_keyvalue("NAME", name);
    }

    void set_password(const StringView password){
        return write_keyvalue("PASSWORD", password);
    }

    enum class Role:uint8_t{
        DeterminedByGpio = 0,
        Central = 1,
        Peripheral = 2,
    };

    void set_role(const Role role){
        return write_keyvalue("ROLE", uint8_t(role));
    }

    void reset(){
        return write_command("RST");
    }

    void set_baudrate(uint8_t code){
        return write_keyvalue("UART", code);
    }

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

    void validate(){
        return write("AT\r\n");
    }

    void shutdown(){
        return write_command("OFF");
    }

    void disconnect(){
        return write_command("DISC");
    }

    void start_scan(){
        return write_command("SCAN");
    }


    void connect_to(auto && x){
        return write_keyvalue("CONNECT", std::forward<decltype(x)>(x));
    }

    void set_passward(auto && x){
        return write_keyvalue("PASSWARD", std::forward<decltype(x)>(x));
    }

    void set_txpower(TxPower power){
        return write_keyvalue("POWE", uint8_t(power));
    }

    void resume(){};
private:
    template<typename ... Ts>
    void write(Ts && ... args){
        // return 
    }
};


class ECB02{

};
}