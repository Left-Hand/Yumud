#include "SdoSession.hpp"
// #include <iostream>
// #include <cstring>

using namespace ymd::canopen;


enum class SdoCommandType : uint8_t {
    DownloadSegment = 0x00,  // 下载段
    InitiateDownload = 0x01,  // 初始化下载
    InitiateUpload = 0x02,  // 初始化上传
    UploadSegment = 0x03,  // 上传段
    AbortTransfer = 0x04,  // 中止传输
    BlockDownload = 0x05,  // 块下载
    BlockUpload = 0x06,  // 块上传
    BlockEnd = 0x07  // 块结束
};

// enum class SdoAbortCode : uint32_t {
//     NoError = 0x00000000,  // 无错误
//     ToggleBitNotAlternated = 0x05030000,  // 切换位未交替
//     SDOProtocolTimedOut = 0x05040000,  // SDO协议超时
//     CommandSpecifierNotValid = 0x05040001,  // 命令字无效
//     InvalidBlockSize = 0x05040002,  // 无效的块大小
//     InvalidSequenceNumber = 0x05040003,  // 无效的序列号
//     CRCError = 0x05040004,  // CRC校验错误
//     OutOfMemory = 0x05040005,  // 内存不足

//     // 对象字典相关错误
//     UnsupportedAccess = 0x06010000,  // 不支持的访问类型
//     ReadOnlyAccess = 0x06010001,  // 只读访问
//     WriteOnlyAccess = 0x06010002,  // 只写访问
//     ObjectDoesNotExist = 0x06020000,  // 对象不存在
//     ObjectCannotBeMapped = 0x06040041,  // 对象无法映射
//     PDOLengthExceeded = 0x06040042,  // PDO长度超出限制
//     ParameterIncompatibility = 0x06040043,  // 参数不兼容
//     InternalIncompatibility = 0x06040047,  // 内部不兼容
//     HardwareError = 0x06060000,  // 硬件错误
//     DataTypeMismatch = 0x06070010,  // 数据类型不匹配
//     DataTypeLengthMismatch = 0x06070012,  // 数据长度不匹配
//     SubIndexDoesNotExist = 0x06090011,  // 子索引不存在
//     ValueRangeExceeded = 0x06090030,  // 值超出范围
//     ValueTooHigh = 0x06090031,  // 值过高
//     ValueTooLow = 0x06090032,  // 值过低
//     MaximumLessThanMinimum = 0x06090036,  // 最大值小于最小值
//     ResourceNotAvailable = 0x060A0023,  // 资源不可用
//     GeneralError = 0x08000000,  // 通用错误
//     DataCannotBeTransferred = 0x08000020,  // 数据无法传输
//     DataCannotBeTransferredLocalControl = 0x08000021,  // 数据无法传输（本地控制）
//     DataCannotBeTransferredDeviceState = 0x08000022,  // 数据无法传输（设备状态）
//     DataCannotBeTransferredOD = 0x08000023,  // 数据无法传输（对象字典）
//     NoDataAvailable = 0x08000024  // 无可用数据
// };

struct SdoCommandSpecifier {
    uint8_t command : 3;  // 命令类型，占3位
    uint8_t sizeIndicator : 1;  // 大小指示位，占1位
    uint8_t expedited : 1;  // 快速传输指示位，占1位
    uint8_t reserved : 3;  // 保留位，占3位
};

class SdoCommand {
public:
    // 位域结构体
    using CommandSpecifier = SdoCommandSpecifier;
    // 构造函数
    SdoCommand(const CanMsg & msg) {
        specifier = std::bit_cast<CommandSpecifier>(msg[0]);
    }

    auto type() const { return SdoCommandType(specifier.command); }

private:
    CommandSpecifier specifier;
};

bool SdoServerSession::processMessage(const CanMsg & msg) {
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


bool SdoClientSession::processMessage(const CanMsg & msg) {
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

    static SdoPacket from(const CanMsg & msg){return std::bit_cast<SdoPacket>(msg.data64());}

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
        CanMsg(cobid_,
        std::make_tuple(SdoPacket{
            .cmd = cmd_code,
            .idx = idx,
            .subidx = subidx,
            .data = et.data32()
        }))
    );
}

void SdoClientSession::requestRead(const OdIndex idx, const OdSubIndex subidx, SubEntry & et){
    // 发送 SDO 读请求
    sdo_.sendMessage(
        CanMsg(cobid_,
        std::make_tuple(SdoPacket{
            .cmd = 0x40, // SDO Read Request
            .idx = idx,
            .subidx = subidx,
            .data = 0 // 读请求不需要数据字段
        }))
    );
}

void SdoClientSession::handleWriteResponse(const CanMsg & msg){

}


void SdoClientSession::handleReadResponse(const CanMsg & msg){

}


void SdoServerSession::processWriteRequest(const CanMsg & msg) {
    // 解析接收到的SDO写请求
    const auto sdo_packet = SdoPacket::from(msg);
    const auto idx = sdo_packet.idx;
    const auto subidx = sdo_packet.subidx;
    // 获取对象字典中的子条目
    const auto sub_entry_opt = sdo_.getSubEntry(idx, subidx);

    if (!sub_entry_opt.has_value()) {
        // 如果子条目不存在，发送错误响应
        sdo_.sendMessage(CanMsg(cobid_, 
        std::make_tuple(SdoPacket{
            .cmd = 0x80, // SDO Abort Transfer
            .idx = idx,
            .subidx = subidx,
            .data = uint32_t(SdoAbortCode::ObjectDoesNotExist) // Object does not exist
        })));
        return;
    }

    auto & sub_entry = sub_entry_opt.value();

    // 检查数据大小是否匹配
    const auto dsize = sub_entry.dsize();
    if (dsize != (sdo_packet.cmd & 0x0F)) {
        // 如果数据大小不匹配，发送错误响应
        sdo_.sendMessage(CanMsg(cobid_, std::make_tuple(SdoPacket{
            .cmd = 0x80, // SDO Abort Transfer
            .idx = idx,
            .subidx = subidx,
            .data = uint32_t(SdoAbortCode::InvalidBlockSize) // Invalid size for object
        })));
        return;
    }

    // 写入数据到对象字典
    sub_entry.put(sdo_packet.data);

    // 发送SDO写响应
    sdo_.sendMessage(CanMsg(cobid_, std::make_tuple(SdoPacket{
        .cmd = 0x60, // SDO Write Response
        .idx = sdo_packet.idx,
        .subidx = sdo_packet.subidx,
        .data = 0 // No additional data needed
    })));
}


void SdoServerSession::processReadRequest(const CanMsg & msg) {
    // 解析接收到的SDO写请求
    const auto sdo_packet = SdoPacket::from(msg);
    const auto idx = sdo_packet.idx;
    const auto subidx = sdo_packet.subidx;
    // 获取对象字典中的子条目
    const auto sub_entry_opt = sdo_.getSubEntry(idx, subidx);

    if (!sub_entry_opt.has_value()) {
        sendReadResponse(0x00, idx, subidx, 0x06020000); // Object does not exist
        return;
    }

    auto & sub_entry = sub_entry_opt.value();

    // 检查客户端请求的读取权限
    if (!sub_entry.readable()) {
        // 如果没有读取权限，发送 SDO 中止传输响应
        sendReadResponse(0x00, idx, subidx, 0x06010001); // Access denied
        return;
    }

    // 从子条目中读取数据
    uint32_t data = sub_entry.data32();

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
    sdo_.sendMessage(CanMsg(cobid_, std::make_tuple(response_packet)));
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
    sdo_.sendMessage(CanMsg(cobid_, std::make_tuple(response_packet)));
}