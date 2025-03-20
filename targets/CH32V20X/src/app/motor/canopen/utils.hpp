#pragma once

#include "hal/bus/can/can.hpp"
#include <variant>
#include <unordered_map>
#include <tuple>
// #include <pair>

#include "core/string/String.hpp"

namespace ymd::canopen{

using Can = hal::Can;
using CanMsg = hal::CanMsg;

class CanDriver {
public:
    // using Can = hal::Can;
    // using CanMsg = hal::CanMsg;

    virtual ~CanDriver() = default;
    virtual bool write(const CanMsg & frame) = 0;
    virtual bool read(CanMsg & frame) = 0;
};


template<typename T>
class optref{
private:
    T * ptr_;

    void check() const{
        if(ptr_ == nullptr){
            HALT;
        }
    }
public:
    optref(T & ptr): ptr_(&ptr){}
    optref(T * ptr): ptr_(ptr){}
    optref(std::nullopt_t): ptr_(nullptr){}

    bool has_value() const{return ptr_ != nullptr;}

    T & value() const{check();return *ptr_;}
    // T & value() const{check();return *ptr_;}
};

class SubEntry;

class CanOpenListener {
public:
    virtual void onObjDictChange(SubEntry & subEntry) = 0;
    virtual void onMessage(const CanMsg & msg) = 0;
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

    CobId(const uint16_t id):
        nodeid(id & 0x7F),
        fcode((id >> 7) & 0x0f){;}
    constexpr operator uint16_t() const {return nodeid | fcode << 7;}
};

using OdIndex = uint16_t;
using OdSubIndex = uint8_t;

struct Didx{
    OdIndex idx;
    OdSubIndex subidx;

    constexpr Didx(const std::pair<OdIndex, OdSubIndex> didx):
        idx(didx.first),
        subidx(didx.second){;}

    constexpr Didx(const OdIndex _idx, const OdSubIndex _subidx):
        idx(_idx),
        subidx(_subidx){;}

    constexpr bool operator==(const Didx& other) const { return idx == other.idx && subidx == other.subidx; }
    constexpr bool operator!=(const Didx& other) const { return !(*this == other); }

};

// enum class EntryAccessError: uint8_t{
//     None = 0,
//     InvalidValue = 0x01,
//     InvalidLength = 0x02,
//     InvalidType = 0x03,
//     InvalidSubIndex = 0x04,
//     InvalidIndex = 0x05,
//     InvalidAccess = 0x06,
//     InvalidAccessType = 0x07,
//     InvalidAccessError = 0x08,
//     InvalidAccessError2 = 0x09,
//     ReadOnlyAccess,
//     WriteOnlyAccess,
// };



class SdoAbortCode {
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
        GeneralError                = 0x08000000,           // 一般错误
        NoValidData                 = 0x08000024          // 无可用数据  
    };

    constexpr SdoAbortCode(const Enum e) : e_(e) {;}
    // constexpr SdoAbortCode(const EntryAccessError e) : e_(SdoAbortCode(e)){;}
    constexpr operator Enum() const { return e_; }
    constexpr operator bool() const { return e_ != Enum::None; }

private:
    Enum e_;
};


template<typename Ret, typename Error>
class Result_t {
private:
    std::variant<Ret, Error> result_;

public:
    // 构造函数，用于成功情况
    Result_t(Ret value) : result_(std::move(value)) {}

    // 构造函数，用于错误情况
    Result_t(Error error) : result_(std::move(error)) {}

    // 检查是否成功
    bool is_ok() const {
        return std::holds_alternative<Ret>(result_);
    }

    // 检查是否出错
    bool is_err() const {
        return std::holds_alternative<Error>(result_);
    }

    // 获取成功值，如果当前是错误状态则抛出异常
    Ret unwrap() {
        if (is_ok()) {
            return std::get<Ret>(result_);
        } else {
            HALT
        }
    }

    // 获取错误值，如果当前是成功状态则抛出异常
    Error unwrap_err() {
        if (is_err()) {
            return std::get<Error>(result_);
        } else {
            HALT
        }
    }

    // 获取成功值，如果当前是错误状态则返回默认值
    Ret unwrap_or(Ret default_value) {
        if (is_ok()) {
            return std::get<Ret>(result_);
        } else {
            return default_value;
        }
    }

    // 获取成功值，如果当前是错误状态则调用提供的函数生成默认值
    Ret unwrap_or_else(std::function<Ret()> func) {
        if (is_ok()) {
            return std::get<Ret>(result_);
        } else {
            return func();
        }
    }

    // 获取错误值，如果当前是成功状态则调用提供的函数生成错误值
    Error unwrap_err_or_else(std::function<Error()> func) {
        if (is_err()) {
            return std::get<Error>(result_);
        } else {
            return func();
        }
    }

    // 如果是成功状态，则调用提供的函数处理成功值
    template<typename Func>
    void and_then(Func func) {
        if (is_ok()) {
            func(std::get<Ret>(result_));
        }
    }

    // 如果是错误状态，则调用提供的函数处理错误值
    template<typename Func>
    void or_else(Func func) {
        if (is_err()) {
            func(std::get<Error>(result_));
        }
    }
};

}