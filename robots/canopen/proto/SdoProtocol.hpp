#pragma once


#include "protocolBase.hpp"
#include <unordered_map>



namespace ymd::canopen{

class SdoSession;

class SdoProtocol : public ProtocolBase {
public:
    SdoProtocol(Driver & driver, ObjectDictIntf & od1)
        : ProtocolBase("Sdo", driver), driver_(driver), od1_(od1) {
    }


    void send_abort(int index, int subIndex, SdoAbortCode _abortCode) {
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

    bool processMessage(const CanMsg & msg);

    static constexpr OdIndex extractIndex(const CanMsg & msg) {
        const auto payload = msg.payload();
        return (payload[2] << 8) | payload[1] ;
    }

    static constexpr OdSubIndex extractSubIndex(const CanMsg& msg) {
        return msg.payload()[3];
    }


private:
    Driver & driver_;
    ObjectDictIntf & od1_;
    std::unordered_map<uint16_t, SdoSession * > sessions;
};


}