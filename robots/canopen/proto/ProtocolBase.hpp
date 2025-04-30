#pragma once

#include "canopen/ObjectDict.hpp"
#include "canopen/utils.hpp"
#include "core/debug/debug.hpp"

namespace ymd::canopen {
    


class ProtocolBase {
public:
    using Driver = CanDriver;
    // Function Codes defined in the CANopen DS301

    enum class FuncCode:uint8_t{
        NMT = 0x0,
        SYNC = 0x1,
        TIME_STAMP = 0x2,

        PDO1_TX = 0x3,
        PDO1_RX = 0x4,
        PDO2_TX = 0x5,
        PDO2_RX = 0x6,
        PDO3_TX = 0x7,
        PDO3_RX = 0x8,
        PDO4_TX = 0x9,
        PDO4_RX = 0xa,


        SDO_TX = 0xb,
        SDO_RX = 0xc,

        NODE_GUARD = 0x0e,
        LSS = 0x0f
    };

    ProtocolBase(const StringView name, Driver& driver)
        : name_(name), driver_(driver) {
    }

    virtual bool start(){return true;}

    virtual bool stop(){return true;}

    StringView name() const{return StringView(name_);}

    virtual bool processMessage(const CanMsg& msg){return true;}

    void sendMessage(const CanMsg& msg) {
        driver_.write(msg);
    }

    virtual void run(){}

private:
    const String name_;
    Driver& driver_;
};

}