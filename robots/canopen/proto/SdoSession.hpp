#pragma once

// #include "Driver.hpp"
#include "SdoProtocol.hpp"
#include "canopen/Entry.hpp"

namespace ymd::canopen{
class SdoSessionBase {
public:
    SdoSessionBase(SdoProtocol & sdo, CobId cobid, SubEntry & sub):
    sdo_(sdo), cobid_(cobid), sub_(sub) {
    }

    virtual bool processMessage(const CanClassicMsg & msg_) = 0;

    

protected:
    constexpr uint8_t extractCmdSpecifier
        (const CanClassicMsg & msg) {return (msg.payload_bytes()[0] >> 5);}
    constexpr uint8_t extractN2
        (const CanClassicMsg & msg) {return ((msg.payload_bytes()[0] >> 2) & 0x03);}
    constexpr uint8_t extractCommandSpecifier
        (const CanClassicMsg & msg) {return ((msg.payload_bytes()[0] >> 5) & 0x03);}
    constexpr uint8_t extractN3
        (const CanClassicMsg & msg) {return ((msg.payload_bytes()[0] >> 1) & 0x07);}
    constexpr uint8_t extractExpidited
        (const CanClassicMsg & msg) {return ((msg.payload_bytes()[0] >> 1) & 0x01);}
    constexpr uint8_t extractSizeInd
        (const CanClassicMsg & msg) {return (msg.payload_bytes()[0] & 0x01);}
    constexpr uint8_t extractEndTrans
        (const CanClassicMsg & msg) {return (msg.payload_bytes()[0] & 0x01);}
    constexpr uint8_t extractToggle
        (const CanClassicMsg & msg) {return ((msg.payload_bytes()[0] >> 4) & 0x01);}

    SdoProtocol & sdo_;
    const CobId cobid_;
    SubEntry & sub_;
    // bool inProgress;
    // int index;
    // int subIndex;
    // uint8_t * bbSeg;
    // int bbSegSize;
    // int toggle;

    virtual bool segmentDownloadRequest(const CanClassicMsg & msg) = 0;
    virtual bool segmentUploadRequest(const CanClassicMsg & msg) = 0;
    virtual bool uploadRequest(const CanClassicMsg & msg) = 0;
    // virtual bool processMessage(const CanClassicMsg & msg) = 0;
};


class SdoServerSession : public SdoSessionBase{
public:
    SdoServerSession(SdoProtocol & sdo, CobId cobid, SubEntry & sub) : SdoSessionBase(sdo, cobid, sub) {}

    bool uploadRequest(const CanClassicMsg & msg);

    bool processMessage(const CanClassicMsg & msg);

    void processWriteRequest(const CanClassicMsg & msg);
    void processReadRequest(const CanClassicMsg & msg);

    void sendWriteResponse(const uint8_t cmd, const OdIndex idx, const OdSubIndex subidx, uint32_t data);
    void sendReadResponse(const uint8_t cmd, const OdIndex idx, const OdSubIndex subidx, uint32_t data);
};

class SdoClientSession : public SdoSessionBase{
public:
    SdoClientSession(SdoProtocol & sdo, CobId cobid, SubEntry & sub) : SdoSessionBase(sdo, cobid, sub) {}

    // bool downloadRequest(const CanClassicMsg & msg);
    bool downloadRequest(const CanClassicMsg & msg);

    void requestWrite(const OdIndex idx, const OdSubIndex subidx, SubEntry & et);

    void requestRead(const OdIndex idx, const OdSubIndex subidx, SubEntry & et);

    void handleWriteResponse(const CanClassicMsg & msg);
    void handleReadResponse(const CanClassicMsg & msg);

    bool processMessage(const CanClassicMsg & msg);
};

}