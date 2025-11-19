#pragma once

#include "canopen_primitive_base.hpp"
#include "core/tmp/bits/width.hpp"

namespace ymd::canopen::primitive{

enum class [[nodiscard]] SdoCommandKind : uint8_t {
    DownloadSegment = 0x00,  // 下载段
    InitiateDownload = 0x01,  // 初始化下载
    InitiateUpload = 0x02,  // 初始化上传
    UploadSegment = 0x03,  // 上传段
    AbortTransfer = 0x04,  // 中止传输
    BlockDownload = 0x05,  // 块下载
    BlockUpload = 0x06,  // 块上传
    BlockEnd = 0x07  // 块结束
};

// struct SdoCommandSpecifier {
//     SdoCommandKind command : 3;  // 命令类型，占3位
//     uint8_t size_indicator : 1;  // 大小指示位，占1位
//     uint8_t is_expedited : 1;  // 快速传输指示位，占1位
//     uint8_t __resv__ : 3;  // 保留位，占3位
// };

enum class [[nodiscard]] SdoCommandSpecifierKind:uint8_t{
    ExpeditedWrite1B = 0x2f,
    ExpeditedWrite2B = 0x2b,
    ExpeditedWrite3B = 0x27,
    ExpeditedWrite4B = 0x23,

    ExpeditedRead1B = 0x4f,
    ExpeditedRead2B = 0x4b,
    ExpeditedRead3B = 0x47,
    ExpeditedRead4B = 0x43,

    ReadOk = 0x40,
    WriteOk = 0x60,
    Exception = 0x80
};

struct [[nodiscard]] SdoCommandSpecifier { 
    using Self = SdoCommandSpecifier;
    using Kind = SdoCommandSpecifierKind;

    constexpr SdoCommandSpecifier(const Kind kind) : kind_(kind) {}
    static constexpr Self from_bits(const uint8_t bits){
        return Self(static_cast<Kind>(bits));
    }

    static constexpr Self from_expedited_write_length(const size_t size){
        switch(size){
            case 1: return Self(Kind::ExpeditedWrite1B);
            case 2: return Self(Kind::ExpeditedWrite2B);
            case 3: return Self(Kind::ExpeditedWrite3B);
            case 4: return Self(Kind::ExpeditedWrite4B);
        }
        __builtin_trap();
    }

    static constexpr Self from_expedited_read_length(const size_t size){
        switch(size){
            case 1: return Self(Kind::ExpeditedRead1B);
            case 2: return Self(Kind::ExpeditedRead2B);
            case 3: return Self(Kind::ExpeditedRead3B);
            case 4: return Self(Kind::ExpeditedRead4B);
        }
        __builtin_trap();
    }

    [[nodiscard]] constexpr size_t length() const{
        switch(kind_){
            case Kind::ExpeditedRead1B: return 1;
            case Kind::ExpeditedRead2B: return 2;
            case Kind::ExpeditedRead3B: return 3;
            case Kind::ExpeditedRead4B: return 4;

            case Kind::ExpeditedWrite1B: return 1;
            case Kind::ExpeditedWrite2B: return 2;
            case Kind::ExpeditedWrite3B: return 3;
            case Kind::ExpeditedWrite4B: return 4;
            default: break;
        }
        __builtin_trap();
    }


    [[nodiscard]] constexpr uint8_t as_bits() const{return static_cast<uint8_t>(kind_);}
    constexpr Kind kind() const{return kind_;}
    [[nodiscard]] constexpr bool operator ==(const Self & other) const{return kind_ == other.kind_;}

    using enum Kind;
private:
    Kind kind_;
};

static_assert(sizeof(SdoCommandSpecifier) == 1);



struct SdoHeader {
    using Self = SdoHeader;

    uint32_t bits;

    static constexpr Self from_parts(
        SdoCommandSpecifier _cmd_spec,
        OdPreIndex _pre_idx,
        OdSubIndex _sub_idx
    ) {
        //fuck 1-2-1 没有对齐
        return Self{
            .bits = (static_cast<uint32_t>(_cmd_spec.as_bits()) << 24) |      // 高8位
                    (static_cast<uint32_t>(_pre_idx.as_bits()) << 8) | // 中间16位
                    (static_cast<uint32_t>(_sub_idx.as_bits()))        // 低8位
        };
    }

    constexpr uint32_t as_bits() const { return bits; }

    constexpr SdoCommandSpecifier cmd_spec() const {
        return SdoCommandSpecifier::from_bits((bits >> 24) & 0xFF);  // 取高8位
    }
    
    constexpr OdPreIndex pre_idx() const {
        return OdPreIndex::from_bits((bits >> 8) & 0xFFFF);  // 取中间16位
    }
    
    constexpr OdSubIndex sub_idx() const {
        return OdSubIndex::from_bits(bits & 0xFF);  // 取低8位
    }
};

static_assert(sizeof(SdoHeader) == 4);


struct [[nodiscard]] SdoExpeditedLayout{
    using Self = SdoExpeditedLayout;
    using Header = SdoHeader;
    Header header;
    uint32_t payload_bits;

    template <typename T, typename D = tmp::type_to_uint_t<T>>
    requires (sizeof(T) <= 4)
    [[nodiscard]] __always_inline static constexpr 
    Self from_expedited_write(
        const OdPreIndex _pre_idx, 
        const OdSubIndex _sub_idx, 
        const auto val
    ){
        constexpr auto SPEC = SdoCommandSpecifier::from_expedited_write_length(sizeof(T));
        return Self{Header::from_parts(SPEC, _pre_idx, _sub_idx), static_cast<uint32_t>(std::bit_cast<D>(val))};
    }


    [[nodiscard]] __always_inline static constexpr 
    Self from_write_ok(
        const OdPreIndex _pre_idx, 
        const OdSubIndex _sub_idx
    ){
        constexpr auto SPEC = SdoCommandSpecifier(SdoCommandSpecifier::Kind::WriteOk);
        return Self{Header::from_parts(SPEC, _pre_idx, _sub_idx), static_cast<uint32_t>(0)};
    }

    template <typename T, typename D = tmp::type_to_uint_t<T>>
    requires (sizeof(T) <= 4)
    [[nodiscard]] __always_inline static constexpr 
    Self from_expedited_read(
        const OdPreIndex _pre_idx, 
        const OdSubIndex _sub_idx, 
        const auto val
    ){
        constexpr auto SPEC = SdoCommandSpecifier::from_expedited_read_length(sizeof(T));
        return Self{Header::from_parts(SPEC, _pre_idx, _sub_idx), static_cast<uint32_t>(std::bit_cast<D>(val))};
    }

    [[nodiscard]] __always_inline static constexpr 
    Self from_read_ok(
        const OdPreIndex _pre_idx, 
        const OdSubIndex _sub_idx
    ){
        constexpr auto SPEC = SdoCommandSpecifier(SdoCommandSpecifier::Kind::ReadOk);
        return Self{Header::from_parts(SPEC, _pre_idx, _sub_idx), static_cast<uint32_t>(0)};
    }

    [[nodiscard]] __always_inline static constexpr 
    Self from_exception(
        const OdPreIndex _pre_idx, 
        const OdSubIndex _sub_idx
    ){
        constexpr auto SPEC = SdoCommandSpecifier(SdoCommandSpecifier::Kind::Exception);
        return Self{Header::from_parts(SPEC, _pre_idx, _sub_idx), static_cast<uint32_t>(0)};
    }

    [[nodiscard]] __always_inline constexpr 
    Self from_u64(const uint64_t int_val){
        return std::bit_cast<Self>(int_val);
    }

    [[nodiscard]] __always_inline constexpr 
    uint64_t as_u64() const {
        return std::bit_cast<uint64_t>(*this);
    }

    [[nodiscard]] std::span<const uint8_t, 8> as_bytes() const {
        return std::span<const uint8_t, 8>(
            reinterpret_cast<const uint8_t*>(this),
            sizeof(SdoExpeditedLayout)
        );
    }

    [[nodiscard]] hal::CanClassicMsg with_cobid(const CobId cobid){
        return hal::CanClassicMsg::from_id_and_u64(cobid.to_stdid(), as_u64());
    }
};

static_assert(sizeof(SdoExpeditedLayout) == 8);



enum class SdoAbortError : uint32_t {
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

class [[nodiscard]] SdoAbortCode {
public:
    using enum SdoAbortError;
    static constexpr uint32_t NO_ERROR = (0x00000000ul);
    constexpr SdoAbortCode(const SdoAbortError err) : bits_(static_cast<uint32_t>(err)) {;}
    constexpr SdoAbortCode(Ok<void>) : bits_(NO_ERROR) {;}

    [[nodiscard]] constexpr Option<SdoAbortError> err() const {
        if(bits_ == 0) return None;
        return Some(std::bit_cast<SdoAbortError>(bits_));
    }
    [[nodiscard]] constexpr uint32_t to_u32() const { return std::bit_cast<uint32_t>(bits_); }
    [[nodiscard]] constexpr uint32_t as_bits() const { return to_u32();}
    [[nodiscard]] constexpr bool is_ok() const { return bits_ == NO_ERROR; }
    [[nodiscard]] constexpr bool is_err() const { return bits_ != NO_ERROR; }
private:
    uint32_t bits_;

    friend OutputStream & operator<<(OutputStream & os, const SdoAbortError err);

    friend OutputStream & operator<<(OutputStream & os, const SdoAbortCode & code) {
        if(code.is_err()) [[unlikely]]
            return os << std::bit_cast<SdoAbortCode>(code.as_bits());
        return os << "None";
    }
};




}