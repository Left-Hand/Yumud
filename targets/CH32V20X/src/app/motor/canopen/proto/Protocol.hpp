#pragma once

#include "canopen/CanOpenNode.hpp"
#include "canopen/ObjectDict.hpp"
#include "canopen/utils.hpp"

namespace ymd::canopen {
    
struct CobId{
    uint16_t nodeid:7;
    uint16_t fcode:4;

    CobId(const uint16_t id):
        nodeid(id & 0x7F),
        fcode((id >> 7) & 0x0f){;}
    constexpr operator uint16_t() const {return nodeid | fcode << 7;}
};

class Protocol {
public:
    using Driver = CanDriver;
    // Function Codes defined in the CANopen DS301
    static constexpr int NMT = 0x0;
    static constexpr int SYNC = 0x1;
    static constexpr int TIME_STAMP = 0x2;
    static constexpr int PDO1tx = 0x3;
    static constexpr int PDO1rx = 0x4;
    static constexpr int PDO2tx = 0x5;
    static constexpr int PDO2rx = 0x6;
    static constexpr int PDO3tx = 0x7;
    static constexpr int PDO3rx = 0x8;
    static constexpr int PDO4tx = 0x9;
    static constexpr int PDO4rx = 0xA;
    static constexpr int SDOtx = 0xB;
    static constexpr int SDOrx = 0xC;
    static constexpr int NODE_GUARD = 0xE;
    static constexpr int LSS = 0xF;

    Protocol(const StringView name, Driver& driver,  ObjectDictionary& od1)
        : name_(name), driver_(driver),  od_(od1) {
    }

    virtual bool start() {
        if (isEnabled) {
            return false;
        }

        isEnabled = true;
        return true;
    }

    virtual bool stop() {
        if (!isEnabled) {
            return false;
        }

        isEnabled = false;
        return true;
    }

    StringView name() const{return StringView(name_);}

    virtual bool processMessage(const CanMsg& msg) {
        if (!isEnabled) {
            return false;
        }

        return true;
    }

    void sendMessage(const CanMsg& msg) {
        driver_.write(msg);
    }

    auto getSubEntry(OdIndex index, OdSubIndex subindex) {
        return od_[index].value().operator[](subindex);
    }

    auto getEntry(OdIndex index) {
        return od_[index];
    }

    virtual void run(){}
protected:

    bool isEnabled = false;
private:
    const String name_;
    Driver& driver_;
    ObjectDictionary& od_;
};

}