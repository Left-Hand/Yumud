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

    virtual bool processMessage(const BxCanFrame & frame_) = 0;

    

protected:
    constexpr uint8_t extractCmdSpecifier
        (const BxCanFrame & frame) {return (msg.payload_bytes()[0] >> 5);}
    constexpr uint8_t extractN2
        (const BxCanFrame & frame) {return ((msg.payload_bytes()[0] >> 2) & 0x03);}
    constexpr uint8_t extractCommandSpecifier
        (const BxCanFrame & frame) {return ((msg.payload_bytes()[0] >> 5) & 0x03);}
    constexpr uint8_t extractN3
        (const BxCanFrame & frame) {return ((msg.payload_bytes()[0] >> 1) & 0x07);}
    constexpr uint8_t extractExpidited
        (const BxCanFrame & frame) {return ((msg.payload_bytes()[0] >> 1) & 0x01);}
    constexpr uint8_t extractSizeInd
        (const BxCanFrame & frame) {return (msg.payload_bytes()[0] & 0x01);}
    constexpr uint8_t extractEndTrans
        (const BxCanFrame & frame) {return (msg.payload_bytes()[0] & 0x01);}
    constexpr uint8_t extractToggle
        (const BxCanFrame & frame) {return ((msg.payload_bytes()[0] >> 4) & 0x01);}

    SdoProtocol & sdo_;
    const CobId cobid_;
    SubEntry & sub_;
    // bool inProgress;
    // int index;
    // int subIndex;
    // uint8_t * bbSeg;
    // int bbSegSize;
    // int toggle;

    virtual bool segmentDownloadRequest(const BxCanFrame & frame) = 0;
    virtual bool segmentUploadRequest(const BxCanFrame & frame) = 0;
    virtual bool uploadRequest(const BxCanFrame & frame) = 0;
    // virtual bool processMessage(const BxCanFrame & frame) = 0;
};


class SdoServerSession : public SdoSessionBase{
public:
    SdoServerSession(SdoProtocol & sdo, CobId cobid, SubEntry & sub) : SdoSessionBase(sdo, cobid, sub) {}

    bool uploadRequest(const BxCanFrame & frame);

    bool processMessage(const BxCanFrame & frame);

    void processWriteRequest(const BxCanFrame & frame);
    void processReadRequest(const BxCanFrame & frame);

    void sendWriteResponse(const uint8_t cmd, const OdIndex idx, const OdSubIndex subidx, uint32_t data);
    void sendReadResponse(const uint8_t cmd, const OdIndex idx, const OdSubIndex subidx, uint32_t data);
};

class SdoClientSession : public SdoSessionBase{
public:
    SdoClientSession(SdoProtocol & sdo, CobId cobid, SubEntry & sub) : SdoSessionBase(sdo, cobid, sub) {}

    // bool downloadRequest(const BxCanFrame & frame);
    bool downloadRequest(const BxCanFrame & frame);

    void requestWrite(const OdIndex idx, const OdSubIndex subidx, SubEntry & et);

    void requestRead(const OdIndex idx, const OdSubIndex subidx, SubEntry & et);

    void handleWriteResponse(const BxCanFrame & frame);
    void handleReadResponse(const BxCanFrame & frame);

    bool processMessage(const BxCanFrame & frame);
};

}