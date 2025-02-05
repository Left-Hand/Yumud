#pragma once

#include "ObjectDict.hpp"
#include "utils.hpp"

namespace ymd::canopen {

using CanMessage = CanMsg;

using CobId = uint16_t;

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

    Protocol(Driver& driver, const StringView name, ObjectDictionary& od1)
        : busDriver(driver), name_(name), objDict(od1) {

        cobIdList.clear();
        messageListeners.clear();
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

    virtual bool processMessage(const CanMessage& msg) {
        if (!isEnabled) {
            return false;
        }
        bool retval = isValidCobId(msg.id());
        return retval;
    }

    void sendMessage(const CanMessage& msg) {
        busDriver.write(msg);
    }

    void addCobId(int index, int subIndex) {
        OdEntry od = objDict.getEntry(index);
        addCobId(od, subIndex);
    }

    void addCobId(OdEntry& od, int subentry) {
        // std::any ref = od.getSub(subentry).getIntReference();
        // if (std::holds_alternative<CobId*>(ref)) {
        //     addCobId(*std::any_cast<CobId*>(ref));
        // } else {
        //     std::cerr << "Invalid reference type" << std::endl;
        // }
    }

    void addCobId(CobId cobId) {
        cobIdList.push_back(cobId);
    }

    SubEntry& getSubEntry(int index, int subindex) {
        return objDict.getSubEntry(index, subindex);
    }

    bool isValidCobId(int cobId) {
        for (const auto id : cobIdList) {
            if (cobId == id) {
                return true;
            }
        }
        return false;
    }

    void addListener(CanOpenListener* coListener) {
        messageListeners.push_back(coListener);
    }

    void removeListener(CanOpenListener* coListener) {
        messageListeners.erase(std::remove(messageListeners.begin(), messageListeners.end(), coListener), messageListeners.end());
    }

    void notifyListeners(const CanMessage& msg) {
        for (auto* listener : messageListeners) {
            listener->onMessage(msg);
        }
    }

private:
    Driver& busDriver;
    const String name_;
    ObjectDictionary& objDict;
    bool isEnabled = false;
    std::vector<CobId> cobIdList;
    std::vector<CanOpenListener*> messageListeners;


    static int extractIndex(const CanMessage& msg) {
        int retval = ((static_cast<int>(msg[2]) << 8) & 0x0000FF00) | (static_cast<int>(msg[1]) & 0x000000FF);
        return retval;
    }

    static int extractSubIndex(const CanMessage& msg) {
        return static_cast<int>(msg[3]) & 0x000000FF;
    }
};

} // namespace ymd::canopen