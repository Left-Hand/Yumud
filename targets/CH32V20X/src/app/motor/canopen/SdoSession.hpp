#pragma once

// #include "Driver.hpp"
#include "SdoProtocol.hpp"
#include "Entry.hpp"

namespace ymd::canopen{
class SdoSession {
public:
    SdoSession(SdoProtocol & sdo, int cobid, SubEntry & sub):
    sdo_(sdo), sub_(sub) {
        
        txCobId = cobid;
        inProgress = false;
        toggle = 0;
        // bbSeg = nullptr;
        bbSegSize = 0;
    }
    
    bool processMessage(const CanMessage & msg_);

private:
    SdoProtocol & sdo_;
    SubEntry& sub_;
    uint32_t txCobId;
    bool inProgress;
    CanMessage msg;
    int index;
    int subIndex;
    uint8_t * bbSeg;
    int bbSegSize;
    int toggle;

    int extractCmdSpecifier();
    int extractN2();
    int extractCommandSpecifier();
    int extractN3();
    int extractExpidited();
    int extractSizeInd();
    int extractEndTrans();
    int extractToggle();

    bool segmentDownloadRequest();
    bool downloadRequest();
    bool segmentUploadRequest();
    bool uploadRequest();
};

}