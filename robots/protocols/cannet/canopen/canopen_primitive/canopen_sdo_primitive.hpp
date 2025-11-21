#pragma once

#include "canopen_primitive_base.hpp"
#include "core/tmp/bits/width.hpp"
#include "core/tmp/implfor.hpp"
#include "core/utils/Option.hpp"
#include "core/utils/Result.hpp"
#include "cast.hpp"


namespace ymd::canopen::primitive{

// enum struct [[nodiscard]] SdoCommandKind : uint8_t {
//     DownloadSegment = 0x00,  // 下载段
//     InitiateDownload = 0x01,  // 初始化下载
//     InitiateUpload = 0x02,  // 初始化上传
//     UploadSegment = 0x03,  // 上传段
//     AbortTransfer = 0x04,  // 中止传输
//     BlockDownload = 0x05,  // 块下载
//     BlockUpload = 0x06,  // 块上传
//     BlockEnd = 0x07  // 块结束
// };


enum struct [[nodiscard]] SdoCommandSpecifierKind:uint8_t{
    ExpeditedWrite1B = 0x2f,
    ExpeditedWrite2B = 0x2b,
    ExpeditedWrite3B = 0x27,
    ExpeditedWrite4B = 0x23,

    ExpeditedRead1B = 0x4f,
    ExpeditedRead2B = 0x4b,
    ExpeditedRead3B = 0x47,
    ExpeditedRead4B = 0x43,

    ReadSucceed = 0x40,
    WriteSucceed = 0x60,
    Exception = 0x80
};

static_assert(sizeof(SdoCommandSpecifierKind) == 1);

struct [[nodiscard]] SdoCommandSpecifier { 
    using Self = SdoCommandSpecifier;
    using Kind = SdoCommandSpecifierKind;

    constexpr SdoCommandSpecifier(const Kind kind) : kind_(kind) {}

    static constexpr Option<Self> try_from_bits(const uint8_t bits){
        return Some(Self(static_cast<Kind>(bits)));
    }

    static constexpr Self from_bits(const uint8_t bits){
        return Self(static_cast<Kind>(bits));
    }

    static constexpr Self from_num_write(const size_t size){
        switch(size){
            case 1: return Self(Kind::ExpeditedWrite1B);
            case 2: return Self(Kind::ExpeditedWrite2B);
            case 3: return Self(Kind::ExpeditedWrite3B);
            case 4: return Self(Kind::ExpeditedWrite4B);
        }
        __builtin_trap();
    }

    static constexpr Self from_num_read(const size_t size){
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


    [[nodiscard]] constexpr uint8_t to_bits() const{return static_cast<uint8_t>(kind_);}
    [[nodiscard]] constexpr uint8_t to_u8() const{return to_bits();}
    [[nodiscard]] constexpr Kind kind() const{return kind_;}
    [[nodiscard]] constexpr bool operator ==(const Self & other) const{return kind_ == other.kind_;}

    using enum Kind;
private:
    Kind kind_;
};

static_assert(sizeof(SdoCommandSpecifier) == 1);



struct [[nodiscard]] SdoHeader {
    using Self = SdoHeader;

    uint32_t bits;

    static constexpr Self from_parts(
        SdoCommandSpecifier _cmd_spec,
        OdIndex _idx
    ) {
        //fuck 1-2-1 没有对齐
        return Self{
            .bits = (static_cast<uint32_t>(_cmd_spec.to_bits()) << 24) |      // 高8位
                    (static_cast<uint32_t>( _idx.pre.to_bits()) << 8) | // 中间16位
                    (static_cast<uint32_t>( _idx.sub.to_bits()))        // 低8位
        };
    }

    [[nodiscard]] constexpr uint32_t to_bits() const { return bits; }

    constexpr SdoCommandSpecifier cmd_spec() const {
        return SdoCommandSpecifier::from_bits((bits >> 24) & 0xFF);  // 取高8位
    }

    constexpr OdIndex idx() const {
        return OdIndex{
            pre_idx().to_bits(),
            sub_idx().to_bits()
        };
    }
    
    constexpr OdPreIndex pre_idx() const {
        return OdPreIndex::from_bits((bits >> 8) & 0xFFFF);  // 取中间16位
    }
    
    constexpr OdSubIndex sub_idx() const {
        return OdSubIndex::from_bits(bits & 0xFF);  // 取低8位
    }

    constexpr void fill_byte(const std::span<uint8_t, 4> bytes){
        bytes[0] = static_cast<uint8_t>(bits >> 0 );
        bytes[1] = static_cast<uint8_t>(bits >> 8 );
        bytes[2] = static_cast<uint8_t>(bits >> 16);
        bytes[3] = static_cast<uint8_t>(bits >> 24);
    }
};

static_assert(sizeof(SdoHeader) == 4);

// https://docs.rs/canopeners/latest/src/canopeners/enums.rs.html#267-301
enum struct SdoAbortError : uint32_t {
    ToggleBitNotAlternated                  = 0x05030000,  // 切换位未交替
    SdoProtocolTimedOut                     = 0x05040000,  // SDO 协议超时
    InvalidClientServerCommandSpecifier     = 0x05040001,  // 无效的客户端/服务器命令指定符
    InvalidBlockSize                        = 0x05040002,  // 无效的块大小
    InvalidSequenceNumber                   = 0x05040003,  // 无效的序列号
    CrcError                                = 0x05040004,  // CRC 错误
    OutOfMemory                             = 0x05040005,  // 内存不足
    UnsupportedAccessToObject               = 0x06010000,  // 不支持的访问类型
    AttemptToReadWriteOnlyObject            = 0x06010001,  // 尝试读取只写对象
    AttemptToWriteReadOnlyObject            = 0x06010002,  // 尝试写入只读对象
    ObjectNotInDictionary                   = 0x06020000,  // 对象不存在于字典中
    ObjectCannotBeMappedToPdo               = 0x06040041,  // 对象无法映射到 PDO
    ExceedPdoLength                         = 0x06040042,  // 超出 PDO 长度
    GeneralParameterIncompatibility         = 0x06040043,  // 通用参数不兼容
    GeneralInternalIncompatibility          = 0x06040047,  // 通用内部不兼容
    HardwareError                           = 0x06060000,  // 硬件错误
    DataTypeMismatchLengthMismatch          = 0x06070010,  // 数据类型不匹配，长度不匹配
    DataTypeMismatchLengthTooHigh           = 0x06070012,  // 数据类型不匹配，长度过高
    DataTypeMismatchLengthTooLow            = 0x06070013,  // 数据类型不匹配，长度过低
    SubIndexDoesNotExist                    = 0x06090011,  // 子索引不存在
    InvalidValueForParameter                = 0x06090030,  // 参数值无效
    ValueTooHigh                            = 0x06090031,  // 值过高
    ValueTooLow                             = 0x06090032,  // 值过低
    MaxLessThanMin                          = 0x06090036,  // 最大值小于最小值
    ResourceNotAvailable                    = 0x060A0023,  // 资源不可用
    GeneralError                            = 0x08000000,  // 一般错误
    DataTransferOrStorageFailed             = 0x08000020,  // 数据传输或存储失败
    LocalControlPreventsDataTransfer        = 0x08000021,  // 本地控制阻止数据传输
    DeviceStatePreventsDataTransfer         = 0x08000022,  // 设备状态阻止数据传输
    ObjectDictionaryGenerationFailed        = 0x08000023,  // 对象字典生成失败
    NoDataAvailable                         = 0x08000024,  // 无可用数据
};

struct [[nodiscard]] SdoAbortCode {
public:
    using Self = SdoAbortCode;

    using Kind = SdoAbortError;
    static constexpr uint32_t NO_ERROR = (0x00000000ul);

    constexpr SdoAbortCode(const SdoAbortError err) : bits_(static_cast<uint32_t>(err)) {;}
    constexpr SdoAbortCode(const Ok<void>) : bits_(NO_ERROR) {;}

    static constexpr Option<SdoAbortCode> try_from_bits(const uint32_t bits){
        if(const auto * str = err_to_str(static_cast<SdoAbortError>(bits)); str != nullptr)
            return Some(SdoAbortCode(static_cast<SdoAbortError>(bits)));
        else
            return None;
    }

    [[nodiscard]] constexpr Option<SdoAbortError> err() const {
        if(bits_ == 0) return None;
        return Some(std::bit_cast<SdoAbortError>(bits_));
    }
    [[nodiscard]] constexpr uint32_t to_u32() const { return std::bit_cast<uint32_t>(bits_); }
    [[nodiscard]] constexpr uint32_t to_bits() const { return to_u32();}
    [[nodiscard]] constexpr bool is_ok() const { return bits_ == NO_ERROR; }
    [[nodiscard]] constexpr bool is_err() const { return bits_ != NO_ERROR; }

    //不要在外部使用这个函数 因为它有可能返回空指针
    static constexpr const char * err_to_str(const SdoAbortError err){
        using Kind = SdoAbortError;
        switch(err){
            case Kind::ToggleBitNotAlternated:
                return "ToggleBitNotAlternated";
            case Kind::SdoProtocolTimedOut:
                return "SdoProtocolTimedOut";
            case Kind::InvalidClientServerCommandSpecifier:
                return "InvalidClientServerCommandSpecifier";
            case Kind::InvalidBlockSize:
                return "InvalidBlockSize";
            case Kind::InvalidSequenceNumber:
                return "InvalidSequenceNumber";
            case Kind::CrcError:
                return "CrcError";
            case Kind::OutOfMemory:
                return "OutOfMemory";
            case Kind::UnsupportedAccessToObject:
                return "UnsupportedAccessToObject";
            case Kind::AttemptToReadWriteOnlyObject:
                return "AttemptToReadWriteOnlyObject";
            case Kind::AttemptToWriteReadOnlyObject:
                return "AttemptToWriteReadOnlyObject";
            case Kind::ObjectNotInDictionary:
                return "ObjectNotInDictionary";
            case Kind::ObjectCannotBeMappedToPdo:
                return "ObjectCannotBeMappedToPdo";
            case Kind::ExceedPdoLength:
                return "ExceedPdoLength";
            case Kind::GeneralParameterIncompatibility:
                return "GeneralParameterIncompatibility";
            case Kind::GeneralInternalIncompatibility:
                return "GeneralInternalIncompatibility";
            case Kind::HardwareError:
                return "HardwareError";
            case Kind::DataTypeMismatchLengthMismatch:
                return "DataTypeMismatchLengthMismatch";
            case Kind::DataTypeMismatchLengthTooHigh:
                return "DataTypeMismatchLengthTooHigh";
            case Kind::DataTypeMismatchLengthTooLow:
                return "DataTypeMismatchLengthTooLow";
            case Kind::SubIndexDoesNotExist:
                return "SubIndexDoesNotExist";
            case Kind::InvalidValueForParameter:
                return "InvalidValueForParameter";
            case Kind::ValueTooHigh:
                return "ValueTooHigh";
            case Kind::ValueTooLow:
                return "ValueTooLow";
            case Kind::MaxLessThanMin:
                return "MaxLessThanMin";
            case Kind::ResourceNotAvailable:
                return "ResourceNotAvailable";
            case Kind::GeneralError:
                return "GeneralError";
            case Kind::DataTransferOrStorageFailed:
                return "DataTransferOrStorageFailed";
            case Kind::LocalControlPreventsDataTransfer:
                return "LocalControlPreventsDataTransfer";
            case Kind::DeviceStatePreventsDataTransfer:
                return "DeviceStatePreventsDataTransfer";
            case Kind::ObjectDictionaryGenerationFailed:
                return "ObjectDictionaryGenerationFailed";
            case Kind::NoDataAvailable:
                return "NoDataAvailable";
            default:
                return nullptr;
        }
    }

    using enum SdoAbortError;
private:
    alignas(4) uint32_t bits_;

    friend OutputStream & operator<<(OutputStream & os, const SdoAbortError err){
        if(const auto str = err_to_str(err); str != nullptr)
            return os << str;

        {
            const auto err_bits = static_cast<uint32_t>(err);
            const auto gaurd = os.create_guard();
            return os << std::hex << std::showbase
                << os.field("Unknown")(err_bits);
        }
    }

    friend OutputStream & operator<<(OutputStream & os, const SdoAbortCode & code) {
        if(code.is_err()) [[unlikely]]
            return os << std::bit_cast<SdoAbortCode>(code.to_bits());
        return os << "None";
    }
};


static_assert(sizeof(SdoAbortCode) == 4);
struct [[nodiscard]] ExpeditedContext{
    using Self = ExpeditedContext;
    using Header = SdoHeader;
    using U8X4 = std::array<uint8_t, 4>;

    alignas(4) Header header;
    alignas(4) U8X4 bytes;

    template <typename T, typename D = tmp::type_to_uint_t<T>>
    requires (sizeof(T) <= 4)
    [[nodiscard]] __always_inline static constexpr 
    Self from_write_req(
        const OdIndex idx,
        const auto int_val
    ){
        static_assert(std::is_same_v<std::decay_t<decltype(int_val)>, T>);
        constexpr auto SPEC = SdoCommandSpecifier::from_num_write(sizeof(T));
        return Self{
            Header::from_parts(SPEC, idx), 
            std::bit_cast<U8X4>(std::bit_cast<D>(int_val))
        };
    }

    template <size_t Extents>
    requires (Extents <= 4)
    [[nodiscard]] __always_inline static constexpr 
    Self from_write_req(
        const OdIndex idx,
        const std::array<uint8_t, Extents> bytes
    ){
        constexpr auto SPEC = SdoCommandSpecifier::from_num_write(Extents);
        return Self{
            Header::from_parts(SPEC, idx), 
            extend_bytes_to_u8x4(std::span(bytes))
        };
    }

    [[nodiscard]] __always_inline static constexpr 
    Self from_write_succeed(
        const OdIndex idx
    ){
        constexpr auto SPEC = SdoCommandSpecifier(SdoCommandSpecifier::Kind::WriteSucceed);
        return Self{
            Header::from_parts(SPEC, idx), 
            std::bit_cast<U8X4>(0)
        };
    }


    [[nodiscard]] __always_inline static constexpr 
    Self from_read_req(
        const OdIndex idx
    ){
        constexpr auto SPEC = SdoCommandSpecifier(SdoCommandSpecifier::Kind::ReadSucceed);
        return Self{
            Header::from_parts(SPEC, idx), 
            std::bit_cast<U8X4>(0)
        };
    }

    template <typename T, typename D = tmp::type_to_uint_t<T>>
    requires (sizeof(T) <= 4)
    [[nodiscard]] __always_inline static constexpr 
    Self from_read_resp(
        const OdIndex idx,
        const auto int_val
    ){
        static_assert(std::is_same_v<std::decay_t<decltype(int_val)>, T>);
        constexpr auto SPEC = SdoCommandSpecifier::from_num_read(sizeof(T));
        return Self{
            Header::from_parts(SPEC, idx), 
            std::bit_cast<U8X4>(std::bit_cast<D>(int_val))
        };
    }

    template <size_t Extents>
    requires (Extents <= 4)
    [[nodiscard]] __always_inline static constexpr 
    Self from_read_resp(
        const OdIndex idx,
        const std::array<uint8_t, Extents> bytes
    ){
        constexpr auto SPEC = SdoCommandSpecifier::from_num_read(Extents);
        return Self{
            Header::from_parts(SPEC, idx), 
            extend_bytes_to_u8x4(std::span(bytes))
        };
    }

    [[nodiscard]] __always_inline static constexpr 
    Self from_exception(
        const OdIndex idx,
        const SdoAbortCode code
    ){
        constexpr auto SPEC = SdoCommandSpecifier(SdoCommandSpecifier::Kind::Exception);
        return Self{
            Header::from_parts(SPEC, idx), 
            std::bit_cast<U8X4>(code.to_u32())
        };
    }

    [[nodiscard]] __always_inline static constexpr 
    Self from_u64(const uint64_t int_val){
        return std::bit_cast<Self>(int_val);
    }

    [[nodiscard]] __always_inline constexpr
    uint32_t bytes_u32() const {
        return std::bit_cast<uint32_t>(bytes);
    }

    [[nodiscard]] __always_inline constexpr 
    uint64_t to_u64() const {
        return std::bit_cast<uint64_t>(*this);
    }

    [[nodiscard]] std::span<const uint8_t, 8> as_bytes() const {
        return std::span<const uint8_t, 8>(
            reinterpret_cast<const uint8_t*>(this),
            sizeof(ExpeditedContext)
        );
    }

    [[nodiscard]] constexpr 
    CanMsg to_canmsg(const CobId cobid) const {
        return CanMsg(cobid.to_stdid(), CanPayload::from_u64(this->to_u64()));
    }

private:
    template<size_t Extents>
    requires (Extents <= 4 || Extents == std::dynamic_extent)
    __always_inline static constexpr U8X4 
    extend_bytes_to_u8x4(const std::span<const uint8_t, Extents> bytes){
        U8X4 ret = {0, 0 ,0, 0};

        for(size_t i = 0; i < bytes.size(); i++){
            ret[i] = bytes[i];
        }

        return ret;
    }
};
static_assert(sizeof(ExpeditedContext) == 8);


}


namespace ymd{

template<>
struct ImplFor<convert::TryFrom<uint32_t>, canopen::primitive::SdoAbortError>{
    using Self = canopen::primitive::SdoAbortError;
    using Error = void;
    static constexpr Result<Self, Error> try_from(uint32_t int_val){
        if(canopen::primitive::SdoAbortCode::err_to_str(
            static_cast<Self>(int_val))
        )
            return Ok(static_cast<Self>(int_val));
        return Err();
    }
};

template<>
struct ImplFor<convert::TryFrom<uint32_t>, canopen::primitive::SdoAbortCode>{
    using Self = canopen::primitive::SdoAbortCode;
    using Error = void;
    static constexpr Result<Self, Error> try_from(uint32_t int_val){
        const auto may_code = Self::try_from_bits(int_val);
        if(may_code.is_none()) return Err();
        return Ok(may_code.unwrap());
    }
};


}