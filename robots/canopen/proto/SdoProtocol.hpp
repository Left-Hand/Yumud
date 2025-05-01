#pragma once


#include "protocolBase.hpp"
#include <unordered_map>



namespace ymd::canopen{

class SdoSession;

class SdoProtocol : public ProtocolBase {
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

    SdoProtocol(Driver & driver, ObjectDict & od1)
        : ProtocolBase("Sdo", driver), driver_(driver), od1_(od1) {
    }


    void sendAbort(int index, int subIndex, int abortCode) {
        uint8_t can_data[8] = {
            0x80,
            (uint8_t)(index & 0xFF),
            (uint8_t)((index >> 8) & 0xFF),
            (uint8_t)(subIndex),
            (uint8_t)(abortCode & 0xFF),
            (uint8_t)((abortCode >> 8) & 0xFF),
            (uint8_t)((abortCode >> 16) & 0xFF),
            (uint8_t)((abortCode >> 24) & 0xFF)
        };

        // int cobid = 0;
        const SubEntry & se = getSubEntry(0x1200, 2).value();
        const auto cobid = CobId::from_u16(int(se));

        sendMessage(
            CanMsg::from_bytes(
                cobid.to_stdid(), 
                std::span(can_data, 8)
            )
        );
    }

    bool processMessage(const CanMsg & msg) override;

    static int extractIndex(const CanMsg & msg) {
        return (msg[2] << 8) | msg[1] ;
    }

    static int extractSubIndex(const CanMsg& msg) {
        return msg[3];
    }


private:
    Driver & driver_;
    ObjectDict & od1_;
    std::unordered_map<uint16_t, SdoSession * > sessions;
};


}