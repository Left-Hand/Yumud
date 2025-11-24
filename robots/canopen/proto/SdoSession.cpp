#include "SdoSession.hpp"


using namespace ymd::canopen;



bool SdoServerSession::processMessage(const CanClassicFrame & frame) {
    const auto cmd = SdoCommand(msg).type();
    switch(cmd) {
        case SdoCommandType::DownloadSegment:  // 下载段
            break;
        case SdoCommandType::InitiateDownload:  // 初始化下载
            break;
        case SdoCommandType::InitiateUpload:  // 初始化上传
            return uploadRequest(msg);
            break;
        case SdoCommandType::UploadSegment:  // 上传段
            break;
        case SdoCommandType::AbortTransfer:  // 中止传输
            break;
        case SdoCommandType::BlockDownload:  // 块下载
            break;
        case SdoCommandType::BlockUpload:  // 块上传
            break;
        case SdoCommandType::BlockEnd: // 块结束
            break;
    }
    return false;
}


bool SdoClientSession::processMessage(const CanClassicFrame & frame) {
    const auto cmd = SdoCommand(msg).type();
    switch(cmd) {
        case SdoCommandType::DownloadSegment:  // 下载段
            break;
        case SdoCommandType::InitiateDownload:  // 初始化下载
            return downloadRequest(msg);
            break;
        case SdoCommandType::InitiateUpload:  // 初始化上传
            break;
        case SdoCommandType::UploadSegment:  // 上传段
            break;
        case SdoCommandType::AbortTransfer:  // 中止传输
            break;
        case SdoCommandType::BlockDownload:  // 块下载
            break;
        case SdoCommandType::BlockUpload:  // 块上传
            break;
        case SdoCommandType::BlockEnd: // 块结束
            break;
    }
    return false;
}


#pragma pack(push, 1)
struct SdoPacket{
    uint8_t cmd;
    uint16_t idx;
    uint8_t subidx;
    uint32_t data;

    static constexpr SdoPacket from(const CanClassicFrame & frame){
        return std::bit_cast<SdoPacket>(msg.payload_u64());
    }

    constexpr CanClassicFrame to_canmsg(const CobId id){
        const auto payload = std::bit_cast<std::array<uint8_t, 8>>(*this);
        return CanClassicFrame::from_bytes(
            id.to_stdid(), 
            std::span(std::move(payload))
        );
    }
};
#pragma pack(pop)


void SdoClientSession::requestWrite(const OdIndex idx, const OdSubIndex subidx, SubEntry & et){
    const auto dsize = et.dsize();
    const uint8_t cmd_code = [&]{
        switch(dsize){
            default:
            case 1: return 0x2f;
            case 2: return 0x2B;
            case 3: return 0x27;
            case 4: return 0x23;
        }
    }();

    sdo_.sendMessage(
        SdoPacket{
            .cmd = cmd_code,
            .idx = idx,
            .subidx = subidx,
            .data = uint32_t(et)
        }.to_canmsg(cobid_)
    );
}

void SdoClientSession::requestRead(const OdIndex idx, const OdSubIndex subidx, SubEntry & et){
    // 发送 SDO 读请求
    sdo_.sendMessage(
        SdoPacket{
            .cmd = 0x40, // SDO Read Request
            .idx = idx,
            .subidx = subidx,
            .data = 0 // 读请求不需要数据字段
        }.to_canmsg(cobid_)
    );
}

void SdoClientSession::handleWriteResponse(const CanClassicFrame & frame){

}


void SdoClientSession::handleReadResponse(const CanClassicFrame & frame){

}


void SdoServerSession::processWriteRequest(const CanClassicFrame & frame) {
    // 解析接收到的SDO写请求
    const auto sdo_packet = SdoPacket::from(msg);
    const auto idx = sdo_packet.idx;
    const auto subidx = sdo_packet.subidx;
    // 获取对象字典中的子条目
    const auto may_sub_entry = sdo_.get_sub_entry(idx, subidx);

    if (!may_sub_entry.is_some()) {
        // 如果子条目不存在，发送错误响应
        sdo_.sendMessage(
            SdoPacket{
                .cmd = 0x80, // SDO Abort Transfer
                .idx = idx,
                .subidx = subidx,
                .data = uint32_t(SdoAbortCode::ObjectDoesNotExist) // Object does not exist
            }.to_canmsg(cobid_)
        );
        return;
    }

    auto & sub_entry = may_sub_entry.unwrap();

    // 检查数据大小是否匹配
    const auto dsize = sub_entry.dsize();
    if (dsize != (sdo_packet.cmd & 0x0F)) {
        // 如果数据大小不匹配，发送错误响应
        sdo_.sendMessage(

            SdoPacket{
                .cmd = 0x80, // SDO Abort Transfer
                .idx = idx,
                .subidx = subidx,
                .data = uint32_t(SdoAbortCode::InvalidBlockSize) // Invalid size for object
            }.to_canmsg(cobid_)
        );
        return;
    }

    // 写入数据到对象字典
    TODO(); 
    {
        //sub_entry.put(sdo_packet.data);
    }
    // 发送SDO写响应
    sdo_.sendMessage(
        SdoPacket{
            .cmd = 0x60, // SDO Write Response
            .idx = sdo_packet.idx,
            .subidx = sdo_packet.subidx,
            .data = 0 // No additional data needed
        }.to_canmsg(cobid_)
    );
}


void SdoServerSession::processReadRequest(const CanClassicFrame & frame) {
    // 解析接收到的SDO写请求
    const auto sdo_packet = SdoPacket::from(msg);
    const auto idx = sdo_packet.idx;
    const auto subidx = sdo_packet.subidx;
    // 获取对象字典中的子条目
    const auto may_sub_entry = sdo_.get_sub_entry(idx, subidx);

    if (!may_sub_entry.is_some()) {
        sendReadResponse(0x00, idx, subidx, 0x06020000); // Object does not exist
        return;
    }

    auto & sub_entry = may_sub_entry.unwrap();

    // 检查客户端请求的读取权限
    if (!sub_entry.is_readable()) {
        // 如果没有读取权限，发送 SDO 中止传输响应
        sendReadResponse(0x00, idx, subidx, 0x06010001); // Access denied
        return;
    }

    // 从子条目中读取数据
    uint32_t data = int(sub_entry);

    // 发送 SDO 读响应
    sendReadResponse(0x00, idx, subidx, data);
}

void SdoServerSession::sendWriteResponse(const uint8_t cmd, const OdIndex idx, const OdSubIndex subidx, uint32_t data){

    // 构造 SDO 读响应数据包
    SdoPacket response_packet{
        .cmd = cmd, // SDO Read Response (高 3 位为 0x40，低 5 位为数据大小)
        .idx = idx,
        .subidx = subidx,
        .data = data
    };

    // 发送响应消息
    sdo_.sendMessage(
        response_packet.to_canmsg(cobid_)
    );
}

void SdoServerSession::sendReadResponse(const uint8_t cmd, const OdIndex idx, const OdSubIndex subidx, uint32_t data){
    // 构造 SDO 读响应数据包
    SdoPacket response_packet{
        .cmd = cmd, // SDO Read Response (高 3 位为 0x40，低 5 位为数据大小)
        .idx = idx,
        .subidx = subidx,
        .data = data
    };

    // 发送响应消息
    sdo_.sendMessage(
        response_packet.to_canmsg(cobid_)
    );
}