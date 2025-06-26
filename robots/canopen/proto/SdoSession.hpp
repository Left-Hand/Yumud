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

    virtual bool processMessage(const CanMsg & msg_) = 0;

    

protected:
    constexpr uint8_t extractCmdSpecifier
        (const CanMsg & msg) {return (msg.payload()[0] >> 5);}
    constexpr uint8_t extractN2
        (const CanMsg & msg) {return ((msg.payload()[0] >> 2) & 0x03);}
    constexpr uint8_t extractCommandSpecifier
        (const CanMsg & msg) {return ((msg.payload()[0] >> 5) & 0x03);}
    constexpr uint8_t extractN3
        (const CanMsg & msg) {return ((msg.payload()[0] >> 1) & 0x07);}
    constexpr uint8_t extractExpidited
        (const CanMsg & msg) {return ((msg.payload()[0] >> 1) & 0x01);}
    constexpr uint8_t extractSizeInd
        (const CanMsg & msg) {return (msg.payload()[0] & 0x01);}
    constexpr uint8_t extractEndTrans
        (const CanMsg & msg) {return (msg.payload()[0] & 0x01);}
    constexpr uint8_t extractToggle
        (const CanMsg & msg) {return ((msg.payload()[0] >> 4) & 0x01);}

    SdoProtocol & sdo_;
    const CobId cobid_;
    SubEntry & sub_;
    // bool inProgress;
    // int index;
    // int subIndex;
    // uint8_t * bbSeg;
    // int bbSegSize;
    // int toggle;

    virtual bool segmentDownloadRequest(const CanMsg & msg) = 0;
    virtual bool segmentUploadRequest(const CanMsg & msg) = 0;
    virtual bool uploadRequest(const CanMsg & msg) = 0;
    // virtual bool processMessage(const CanMsg & msg) = 0;
};


class SdoServerSession : public SdoSessionBase{
public:
    SdoServerSession(SdoProtocol & sdo, CobId cobid, SubEntry & sub) : SdoSessionBase(sdo, cobid, sub) {}

    bool uploadRequest(const CanMsg & msg);

    bool processMessage(const CanMsg & msg);

    void processWriteRequest(const CanMsg & msg);
    void processReadRequest(const CanMsg & msg);

    void sendWriteResponse(const uint8_t cmd, const OdIndex idx, const OdSubIndex subidx, uint32_t data);
    void sendReadResponse(const uint8_t cmd, const OdIndex idx, const OdSubIndex subidx, uint32_t data);
};

class SdoClientSession : public SdoSessionBase{
public:
    SdoClientSession(SdoProtocol & sdo, CobId cobid, SubEntry & sub) : SdoSessionBase(sdo, cobid, sub) {}

    // bool downloadRequest(const CanMsg & msg);
    bool downloadRequest(const CanMsg & msg);

    void requestWrite(const OdIndex idx, const OdSubIndex subidx, SubEntry & et);

    void requestRead(const OdIndex idx, const OdSubIndex subidx, SubEntry & et);

    void handleWriteResponse(const CanMsg & msg);
    void handleReadResponse(const CanMsg & msg);

    bool processMessage(const CanMsg & msg);
};

}