#pragma once

#include "hal/bus/can/can.hpp"
#include <variant>
#include <unordered_map>
#include <tuple>


namespace ymd::canopen{

using Can = hal::Can;
using BxCanFrame = hal::BxCanFrame;

class CanDriver {
public:
    virtual ~CanDriver() = default;
    virtual bool write(const BxCanFrame & frame) = 0;
    virtual bool read(BxCanFrame & frame) = 0;
};


class SubEntry;

class CanOpenListener {
public:
    virtual void onObjDictChange(SubEntry & subEntry) = 0;
    virtual void onMessage(const BxCanFrame & frame) = 0;
    virtual ~CanOpenListener() = default;
};

template<typename T>
struct E_Item {
    T v_;

    constexpr E_Item(T v) : v_(v) {}

    // 添加比较操作符，方便枚举值的比较
    constexpr bool operator==(const E_Item& other) const { return v_ == other.v_; }
    constexpr bool operator!=(const E_Item& other) const { return v_ != other.v_; }
    constexpr bool operator<=(const E_Item& other) const { return v_ <= other.v_; }
    constexpr bool operator<(const E_Item& other) const { return v_ < other.v_; }
    constexpr bool operator>=(const E_Item& other) const { return v_ >= other.v_; }
    constexpr bool operator>(const E_Item& other) const { return v_ > other.v_; }
};



struct CobId{
    uint16_t nodeid:7;
    uint16_t fcode:4;
    uint16_t __resv__ : 5;


    static constexpr CobId from_u16(const uint16_t id){
        return CobId(id);
    }

    static constexpr CobId from_stdid(const hal::CanStdId id){
        return CobId(id.to_u11());
    }

    constexpr hal::CanStdId to_stdid() const {
        return hal::CanStdId(nodeid | fcode << 7);
    }

    constexpr uint16_t to_u16() const {
        return std::bit_cast<uint16_t>(*this);
    }

private:
    constexpr CobId(const uint16_t id):
        nodeid(id & 0x7F),
        fcode((id >> 7) & 0x0f){;}

};

static_assert(sizeof(CobId) == sizeof(uint16_t));

using OdIndex = uint16_t;
using OdSubIndex = uint8_t;


// struct OdIndex{
//     uint16_t count;
// };

// struct OdSubIndex{
//     uint8_t count;
// };

struct Didx{
    OdIndex idx;
    OdSubIndex subidx;

    constexpr Didx(const std::pair<OdIndex, OdSubIndex> didx):
        idx(didx.first),
        subidx(didx.second){;}

    constexpr Didx(const OdIndex _idx, const OdSubIndex _subidx):
        idx(_idx),
        subidx(_subidx){;}

    constexpr bool operator==(const Didx& other) const { 
        return idx == other.idx && subidx == other.subidx; }
};

class SdoAbortCode {
public:
    enum Kind : uint32_t {
        OK                        = 0x00000000,          // 无错误
        ToggleBitNotAlternated      = 0x05030000,          // 切换位未交替
        SdoProtocolTimedOut         = 0x05040000,          // SDO 协议超时
        CommandSpecifierNotValid    = 0x05040001,          // 命令指定符无效
        InvalidBlockSize            = 0x05040002,          // 无效的块大小
        InvalidSequenceNumber       = 0x05040003,          // 无效的序列号
        CRCError                    = 0x05040004,          // CRC 错误
        OutOfMemory                 = 0x05040005,          // 内存不足
        UnsupportedAccess           = 0x06010000,          // 不支持的访问类型
        ReadOnlyAccess              = 0x06010001,          // 只读访问
        WriteOnlyAccess             = 0x06010002,          // 只写访问
        ObjectDoesNotExist          = 0x06020000,          // 对象不存在
        ObjectCannotBeMapped        = 0x06040041,          // 对象无法映射
        PdoLengthExceeded           = 0x06040042,          // PDO 长度超出
        ParameterIncompatibility    = 0x06040043,          // 参数不兼容
        InternalIncompatibility     = 0x06040047,          // 内部不兼容
        HardwareError               = 0x06060000,          // 硬件错误
        ServiceParameterIncorrect   = 0x06070010,          // 服务参数不正确
        ServiceParameterTooLong     = 0x06070012,          // 服务参数过长
        ServiceParameterTooShort    = 0x06070013,          // 服务参数过短
        SubIndexDoesNotExist        = 0x06090011,          // 子索引不存在
        InvalidValue                = 0x06090030,          // 无效的值
        ValueTooHigh                = 0x06090031,          // 值过高
        ValueTooLow                 = 0x06090032,          // 值过低
        MaxLessThanMin              = 0x06090036,          // 最大值小于最小值
        ResourceNotAvailable        = 0x060A0023,          // 资源不可用
        GeneralError                = 0x08000000,           // 一般错误
        NoValidData                 = 0x08000024          // 无可用数据  
    };

    constexpr SdoAbortCode(const Kind e) : e_(e) {;}

    constexpr Kind kind() const { return e_; }
    constexpr uint32_t to_u32() const { return std::bit_cast<uint32_t>(e_); }
    constexpr bool is_ok() const { return e_ == OK; }
    constexpr bool is_err() const { return e_ != OK; }
private:
    Kind e_;
};

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

struct SdoCommandSpecifier {
    uint8_t command : 3;  // 命令类型，占3位
    uint8_t sizeIndicator : 1;  // 大小指示位，占1位
    uint8_t expedited : 1;  // 快速传输指示位，占1位
    uint8_t reserved : 3;  // 保留位，占3位
};

struct PdoMapping{
    uint8_t bits;
    uint8_t subindex;
    uint8_t index;
    static constexpr PdoMapping from_u32(const uint32_t map){
        PdoMapping self;
        self.bits = (map & 0xFF);
        self.subindex = ((map >> 8) & 0xFF);
        self.index = ((map >> 16) & 0xFF);
        return self;
    }

    constexpr uint32_t to_u32(const PdoMapping & map){ 
        return (map.bits | (map.subindex << 8) | (map.index << 16));
    }

};

class SdoCommand {
public:
    // 位域结构体
    using CommandSpecifier = SdoCommandSpecifier;
    // 构造函数
    SdoCommand(const BxCanFrame & frame) {
        specifier = std::bit_cast<CommandSpecifier>(msg.payload_bytes()[0]);
    }

    auto type() const { return SdoCommandType(specifier.command); }

private:
    CommandSpecifier specifier;
};



}