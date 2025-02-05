#pragma once

#include "Protocol.hpp"
#include <unordered_map>



namespace ymd::canopen{

class SdoSession;

class SdoProtocol : public Protocol {
public:
    static constexpr int SDOABT_TOGGLE_NOT_ALTERNED = 0x05030000;
    static constexpr int SDOABT_TIMED_OUT = 0x05040000;
    static constexpr int SDOABT_OUT_OF_MEMORY = 0x05040005;
    static constexpr int SDOABT_GENERAL_ERROR = 0x08000000;
    static constexpr int SDOABT_LOCAL_CTRL_ERROR = 0x08000021;

    static constexpr int OD_READ_NOT_ALLOWED = 0x06010001;
    static constexpr int OD_WRITE_NOT_ALLOWED = 0x06010002;
    static constexpr int OD_NO_SUCH_OBJECT = 0x06020000;
    static constexpr int OD_NOT_MAPPABLE = 0x06040041;
    static constexpr int OD_LENGTH_DATA_INVALID = 0x06070010;
    static constexpr int OD_NO_SUCH_SUBINDEX = 0x06090011;
    static constexpr int OD_VALUE_RANGE_EXCEEDED = 0x06090030;
    static constexpr int OD_VALUE_TOO_LOW = 0x06090031;
    static constexpr int OD_VALUE_TOO_HIGH = 0x06090032;

    SdoProtocol(Driver & driver, ObjectDictionary & od1)
        : Protocol(driver, "SDOProtocol", od1), driver_(driver), od1_(od1) {
        addCobId(0x1200, 1);
        addCobId(0x1200, 2);
    }

    // ~SdoProtocol() {
    //     for (auto& session : sessions) {
    //         delete session.second;
    //     }
    //     sessions.clear();
    // }

    void sendAbort(int index, int subIndex, int abortCode) {
        unsigned char can_data[8];

        can_data[0] = 0x80;
        can_data[1] = (unsigned char)(index & 0xFF);
        can_data[2] = (unsigned char)((index >> 8) & 0xFF);
        can_data[3] = (unsigned char)(subIndex);
        can_data[4] = (unsigned char)(abortCode & 0xFF);
        can_data[5] = (unsigned char)((abortCode >> 8) & 0xFF);
        can_data[6] = (unsigned char)((abortCode >> 16) & 0xFF);
        can_data[7] = (unsigned char)((abortCode >> 24) & 0xFF);

        // int cobid = 0;
        SubEntry& se = getSubEntry(0x1200, 2);
        uint32_t cobid = se.getInt();

        sendMessage({cobid, can_data, 8});
    }

    // void send(int cobid, unsigned char data[]) {
    //     CanMessage msg(cobid, 0, data);
    //     sendMessage(&msg);
    // }

    bool processMessage(const CanMessage & msg) override;

    static int extractIndex(const CanMessage & msg) {
        return (msg[1] & 0xFF) | ((msg[2] & 0xFF) << 8);
    }

    static int extractSubIndex(const CanMessage& msg) {
        return msg[3];
    }


private:
    Driver & driver_;
    ObjectDictionary & od1_;
    std::unordered_map<uint16_t, SdoSession * > sessions;


    SubEntry& getSubEntry(int index, int subIndex) {
        // Assuming ObjectDictionary has a method getSubEntry
        return od1_.getSubEntry(index, subIndex);
    }
};


}