#include "SdoSession.hpp"
// #include <iostream>
// #include <cstring>

using namespace ymd::canopen;


bool SdoSession::processMessage(const CanMessage & msg_) {
    msg = msg_;
    int cmd = extractCmdSpecifier();
    switch(cmd) {
        case 0: // segment download request
            return segmentDownloadRequest();
        case 1: // Download Request
            if(inProgress) {
                // std::cout << "SdoSession.processMessage(), SDO error : Transmission already started." << std::endl;
                sdo_.sendAbort(index, subIndex, SdoProtocol::SDOABT_LOCAL_CTRL_ERROR);
                return false;
            }
            return downloadRequest();
        case 2: // Upload Request
            return uploadRequest();
        case 3: //segmented upload request
            return segmentUploadRequest();
        case 4: // Received SDO abort code
            // std::cout << "recieved abort code" << std::endl;
            return false;
        case 5:
        case 6:
        default:
            // std::cout << "Unimplemented SDO command specifier" << std::endl;
            break;
    }
    return false;
}

int SdoSession::extractCmdSpecifier() {
    return (msg[0] >> 5);
}

int SdoSession::extractN2() {
    return ((msg[0] >> 2) & 0x03);
}

int SdoSession::extractCommandSpecifier() {
    return ((msg[0] >> 5) & 0x03);
}

int SdoSession::extractN3() {
    return ((msg[0] >> 1) & 0x07);
}

int SdoSession::extractExpidited() {
    return ((msg[0] >> 1) & 0x01);
}

int SdoSession::extractSizeInd() {
    return (msg[0] & 0x01);
}

int SdoSession::extractEndTrans() {
    return (msg[0] & 0x01);
}

int SdoSession::extractToggle() {
    return ((msg[0] >> 4) & 0x01);
}

bool SdoSession::segmentDownloadRequest() {
    if (toggle != extractToggle()) {
        // std::cout << "SDO error : Toggle error : " << extractToggle() << std::endl;
        sdo_.sendAbort(index, subIndex, SdoProtocol::SDOABT_TOGGLE_NOT_ALTERNED);
        return false;
    }

    int len = 7 - extractN3();
    if (bbSegSize < len) {
        if (bbSeg != nullptr) {
            delete[] bbSeg;
        }
        bbSeg = new unsigned char[len];
        bbSegSize = len;
    }

    memcpy(bbSeg, msg.begin() + 1, size_t(len));

    unsigned char data[8];
    data[0] = (unsigned char)((1 << 5) | (toggle << 4));
    memset(data + 1, 0, 7);
    // std::cout << "SDO. Send response to download request defined at index 0x120" << std::endl;
    sdo_.sendMessage({txCobId, data, 8});

    toggle = 1 - toggle;

    return false;
}

bool SdoSession::downloadRequest() {
    index = SdoProtocol::extractIndex(msg);
    subIndex = SdoProtocol::extractSubIndex(msg);
    inProgress = true;

    if (extractExpidited() != 0) {
        int len = 4 - extractN2();
        unsigned char data [4];
        memcpy(data, msg.begin() + 4, len);
        unsigned int value = (data[0] | (data[1] << 8) | (data[2] << 16) | (data[3] << 24));
        // std::cout << "SDO downloadRequest is expedited val: " << std::hex << value << std::dec << std::endl;

        // try {
            sub_.put(value);
        // } catch (...) {
        //     // std::cout << "downloadRequest() error : Unable to copy the data in the object dictionary" << std::endl;
        //     sdo_.sendAbort(index, subIndex, SdoProtocol::SDOABT_GENERAL_ERROR);
        // }
    } else {
        if (extractSizeInd() != 0) {
            unsigned int len = (msg[4] | (msg[5] << 8) | (msg[6] << 16) | (msg[7] << 24));
            if (bbSeg != nullptr) {
                delete[] bbSeg;
            }
            bbSeg = new unsigned char[len];
            bbSegSize = len;
        }
    }

    unsigned char data[8];
    data[0] = (unsigned char)(3 << 5);
    data[1] = (unsigned char)(index & 0xFF);
    data[2] = (unsigned char)((index >> 8) & 0xFF);
    data[3] = (unsigned char)(subIndex);
    memset(data + 4, 0, 4);
    sdo_.sendMessage(CanMessage{txCobId, data, 8});

    return false;
}

bool SdoSession::segmentUploadRequest() {
    if (toggle != extractToggle()) {
        // std::cout << "SDO error : Toggle error : " << extractToggle() << std::endl;
        sdo_.sendAbort(index, subIndex, SdoProtocol::SDOABT_TOGGLE_NOT_ALTERNED);
        return false;
    }

    int len = bbSegSize - (bbSeg - bbSeg);
    unsigned char data[8];
    data[0] = (unsigned char)(toggle << 4);
    toggle = 1 - toggle;

    if (len > 7) {
        memcpy(data + 1, bbSeg, 7);
        sdo_.sendMessage(CanMessage{txCobId, data, 8});
        bbSeg += 7;
        return true;
    } else {
        data[0] |= (unsigned char)(1 | ((7 - len) << 1));
        memcpy(data + 1, bbSeg, len);
        memset(data + 1 + len, 0, 7 - len);
        sdo_.sendMessage(CanMessage{txCobId, data, 8});
        bbSeg += len;
        return false;
    }
}

bool SdoSession::uploadRequest() {
    index = SdoProtocol::extractIndex(msg);
    subIndex = SdoProtocol::extractSubIndex(msg);
    if (inProgress) {
        sdo_.sendAbort(index, subIndex, SdoProtocol::SDOABT_LOCAL_CTRL_ERROR);
    }
    inProgress = true;

    auto buffer = sub_.getByteBuffer();
    bbSeg = buffer.data();
    int len = buffer.size();

    if (len > 4) {
        unsigned char data[8];
        data[0] = (unsigned char)((2 << 5) | 1);
        data[1] = (unsigned char)(index & 0xFF);
        data[2] = (unsigned char)((index >> 8) & 0xFF);
        data[3] = (unsigned char)(subIndex);
        data[4] = (unsigned char)(len);
        data[5] = (unsigned char)(len >> 8);
        data[6] = (unsigned char)(len >> 16);
        data[7] = (unsigned char)(len >> 24);
        sdo_.sendMessage(CanMessage{txCobId, data, 8});
        bbSeg = data;
        bbSegSize = len;
        return true;
    } else {
        unsigned char data[8];
        data[0] = (unsigned char)((2 << 5) | ((4 - len) << 2) | 3);
        data[1] = (unsigned char)(index & 0xFF);
        data[2] = (unsigned char)((index >> 8) & 0xFF);
        data[3] = (unsigned char)(subIndex);
        memcpy(data + 4, data, len);
        memset(data + 4 + len, 0, 4 - len);
        sdo_.sendMessage(CanMessage{txCobId, data, 8});
    }
    return false;
}