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
    int extractCmdSpecifier(const CanMsg & msg) {return (msg[0] >> 5);}
    int extractN2(const CanMsg & msg) {return ((msg[0] >> 2) & 0x03);}
    int extractCommandSpecifier(const CanMsg & msg) {return ((msg[0] >> 5) & 0x03);}
    int extractN3(const CanMsg & msg) {return ((msg[0] >> 1) & 0x07);}
    int extractExpidited(const CanMsg & msg) {return ((msg[0] >> 1) & 0x01);}
    int extractSizeInd(const CanMsg & msg) {return (msg[0] & 0x01);}
    int extractEndTrans(const CanMsg & msg) {return (msg[0] & 0x01);}
    int extractToggle(const CanMsg & msg) {return ((msg[0] >> 4) & 0x01);}

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