#pragma once


#include "utils.hpp"

namespace ymd::canopen {


// Forward declaration of CanOpenListener
class SubEntry;



class SdoError {
public:
	enum Enum : uint32_t {
        None                        = 0x00000000,          // 无错误
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
        ServiceParameterIncorrect   =0x06070010,          // 服务参数不正确
        ServiceParameterTooLong     = 0x06070012,          // 服务参数过长
        ServiceParameterTooShort    = 0x06070013,          // 服务参数过短
        SubIndexDoesNotExist        = 0x06090011,          // 子索引不存在
        InvalidValue                = 0x06090030,          // 无效的值
        ValueTooHigh                = 0x06090031,          // 值过高
        ValueTooLow                 = 0x06090032,          // 值过低
        MaxLessThanMin              = 0x06090036,          // 最大值小于最小值
        ResourceNotAvailable        = 0x060A0023,          // 资源不可用
        GeneralError                = 0x08000000           // 一般错误
	};

	SdoError(const Enum e) : e_(e) {;}

	operator Enum() const { return e_; }

    operator bool() const { return e_ != Enum::None; }

private:
	Enum e_;
};


enum class EntryAccessType : uint8_t {
    RW = 0,
    WO = 0x01,
    RO = 0x02,
    CONST = 0x03
};

enum class EntryAccessError: uint8_t{
    None = 0,
    InvalidValue = 0x01,
    InvalidLength = 0x02,
    InvalidType = 0x03,
    InvalidSubIndex = 0x04,
    InvalidIndex = 0x05,
    InvalidAccess = 0x06,
    InvalidAccessType = 0x07,
    InvalidAccessError = 0x08,
    InvalidAccessError2 = 0x09,
    ReadOnlyAccess,
    WriteOnlyAccess,
};


class EntryDataType {
public:
    using Item = E_Item<uint8_t>;


    static constexpr Item bit{0x01};
    static constexpr Item int8{0x02};
    static constexpr Item int16{0x03};
    static constexpr Item int32{0x04};
    static constexpr Item uint8{0x05};
    static constexpr Item uint16{0x06};
    static constexpr Item uint32{0x07};
    static constexpr Item real32{0x08};
    static constexpr Item visible_string{0x09};
    static constexpr Item octet_string{0x0A};
    static constexpr Item unicode_string{0x0B};
    static constexpr Item time_of_day{0x0C};
    static constexpr Item time_difference{0x0D};
    static constexpr Item domain{0x0F};
    static constexpr Item int24{0x10};
    static constexpr Item real64{0x11};
    static constexpr Item int40{0x12};
    static constexpr Item int48{0x13};
    static constexpr Item int56{0x14};
    static constexpr Item int64{0x15};
    static constexpr Item uint24{0x16};
    static constexpr Item pdo_mapping{0x21};
    static constexpr Item sdo_parameter{0x22};
    static constexpr Item identity{0x23};

    constexpr EntryDataType(Item e) : e_(e) {}

    // 类型转换操作符
    constexpr operator Item() const { return e_; }

    // 判断是否为整数类型
    constexpr bool is_int() const {return e_ <= real32;}

    // 获取数据类型的大小
    constexpr size_t dsize() const {
        switch (e_.v_) {
            case bit.v_: return 1; // bit 类型通常按 1 字节处理
            case int8.v_: return 1;
            case int16.v_: return 2;
            case int24.v_: return 3;
            case int32.v_: return 4;
            case int40.v_: return 5;
            case int48.v_: return 6;
            case int56.v_: return 7;
            case int64.v_: return 8;
            case uint8.v_: return 1;
            case uint16.v_: return 2;
            case uint24.v_: return 3;
            case uint32.v_: return 4;
            case real32.v_: return 4;
            case real64.v_: return 8;
            default: return 0; // 其他类型（如字符串）大小不确定，返回 0
        }
    }

private:
    Item e_;
};

using OdIndex = uint16_t;
using OdSubIndex = uint8_t;


class SubEntry {

public:
    using AccessType = EntryAccessType;
    using DataType = EntryDataType;

private:

#pragma pack(push, 1)
// #pragma pack(pop)
    StringView name_;
    AccessType access_type_;
    DataType data_type_;
#pragma pack(pop)
    class ObjRef{
    private:
        bool is_ref_ = false;
        union{
            void * pdata_;
            uint32_t data32_;
        };
    public:
        constexpr ObjRef(uint32_t data):is_ref_(false), data32_(data){}
        constexpr ObjRef(void * pdata):is_ref_(true), pdata_(pdata){}

        template<typename T>
        requires (sizeof(T) <= 4)
        constexpr void write(const T val){
            if(likely(is_ref_)){
                *reinterpret_cast<T*>(pdata_) = val;
            }else{
                data32_ = std::bit_cast<T>(val);
            }
        }

        template<typename T>
        constexpr void read(T & val) const{
            if(likely(is_ref_)){
                val = *reinterpret_cast<T*>(pdata_);
            }else{
                val = std::bit_cast<T>(data32_);
            }
        }

        template<typename T>
        constexpr T read() const {
            if(likely(is_ref_)){
                return *reinterpret_cast<T*>(pdata_);
            }else{
                return std::bit_cast<T>(data32_);
            }
        }

        constexpr const void * data() const{
            return likely(is_ref_) ? pdata_ : (&data32_);
        }

        constexpr void * data(){
            return likely(is_ref_) ? pdata_ : (&data32_);
        }

        constexpr ObjRef & operator =(const auto val){this->write(val); return *this;}
        
        template<typename T>
        explicit constexpr operator T(){return this->read<T>();}

    };

    ObjRef obj_;


    // SubEntry(const SubEntry &) = default;
public:
    SubEntry(const SubEntry &) = default;
    // SubEntry(const SubEntry &) = delete;
    SubEntry(SubEntry &&) = default;

    SubEntry & operator = (const SubEntry &) = default;
    SubEntry & operator = (SubEntry &&) = default;

    constexpr SubEntry(const StringView name, auto & val, AccessType access_type = AccessType::RW, DataType data_type = DataType::int32)
        : name_(name), access_type_(access_type), data_type_(data_type), obj_(&val){}

    operator int() const ;

    EntryAccessError write(const std::span<const uint8_t> pdata){
        if(unlikely(!is_writeable())) return EntryAccessError::WriteOnlyAccess;
        if(unlikely(pdata.size() != dsize())) return EntryAccessError::InvalidLength;
        if(unlikely(pdata.size() > 4)) return EntryAccessError::InvalidLength;
        memcpy(obj_.data(), pdata.data(), pdata.size());
        return EntryAccessError::None;
    }


    EntryAccessError write_any(const void * pdata){
        memcpy(obj_.data(), pdata, dsize());
        return EntryAccessError::None;
    }

    template<typename T>
    requires ((sizeof(T) <= 4) and (!std::is_pointer_v<T>))
    EntryAccessError write_any(const T pdata){
        return write_any((&pdata));
    }

    template<typename T>
    requires ((sizeof(T) <= 4) and (!std::is_pointer_v<T>))
    EntryAccessError read_any(T & pdata){
        return read_any((&pdata));
    }


    EntryAccessError read(std::span<uint8_t> pdata) const{
        if(unlikely(!is_readable())) return EntryAccessError::ReadOnlyAccess;
        if(unlikely(pdata.size() != dsize())) return EntryAccessError::InvalidLength;
        if(unlikely(pdata.size() > 4)) return EntryAccessError::InvalidLength;
        memcpy(pdata.data(), obj_.data(), pdata.size());
        return EntryAccessError::None;
    }

    EntryAccessError read_any(void * pdata){
        memcpy(pdata, obj_.data(), dsize());
        return EntryAccessError::None;
    }


    SubEntry copy() const{return *this;}


    EntryAccessError set(int val);

    EntryAccessError put(const std::span<const uint8_t> val) ;
    EntryAccessError put(const CanMsg & msg){
        return this->put(std::span<const uint8_t>(msg.begin(), msg.size()));
    }

	size_t dsize() const {return data_type_.dsize();}
	size_t size() const {return data_type_.dsize();}

    bool is_readable() const {return access_type_ != AccessType::WO;}
    bool is_writeable() const {return access_type_ == AccessType::RW || access_type_ == AccessType::WO;}
    StringView name() const {return StringView(name_);}

};


class OdEntry{
private:
    using Index = OdIndex; 
    using SubIndex = OdSubIndex; 


	StringView name_;
	std::vector<SubEntry> subentries_ = {};
public:
    OdEntry(const OdEntry &) = default;
    OdEntry(OdEntry &&) = default;

    OdEntry & operator=(const OdEntry &) = default;
    OdEntry & operator=(OdEntry &&) = default;
    
	OdEntry(const StringView name):
        name_(name){}

	OdEntry():
        name_(std::nullopt){}
    
	size_t size(){return(subentries_.size());}

	void add(SubEntry && sub){
		subentries_.push_back(sub);
	}

    std::optional<SubEntry> operator [](const SubIndex idx){
    // optref<SubEntry> operator [](const SubIndex idx){
        return (subentries_[idx]);
    }

    StringView name() const {
        return StringView(name_);
    }
};


} // namespace ymd::canopen