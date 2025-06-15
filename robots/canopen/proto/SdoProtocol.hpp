#pragma once


#include "protocolBase.hpp"
#include <unordered_map>



namespace ymd::canopen{

class SdoSession;

class SdoProtocol : public ProtocolBase {
public:
    SdoProtocol(Driver & driver, ObjectDict & od1)
        : ProtocolBase("Sdo", driver), driver_(driver), od1_(od1) {
    }


    void sendAbort(int index, int subIndex, SdoAbortCode _abortCode) {
        const auto abortCode = _abortCode.to_u32();
        uint8_t can_data[8] = {
            0x80,
            uint8_t(index & 0xFF),
            uint8_t((index >> 8) & 0xFF),
            uint8_t(subIndex),
            uint8_t(abortCode & 0xFF),
            uint8_t((abortCode >> 8) & 0xFF),
            uint8_t((abortCode >> 16) & 0xFF),
            uint8_t((abortCode >> 24) & 0xFF)
        };

        // int cobid = 0;
        const SubEntry & se = get_sub_entry(0x1200, 2).unwrap();
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