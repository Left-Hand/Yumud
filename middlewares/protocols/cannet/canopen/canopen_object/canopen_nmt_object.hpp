#pragma once

#include "../canopen_primitive/canopen_primitive.hpp"
#include "core/string/view/string_view.hpp"
#include "core/container/ring_memento.hpp"
// https://winshton.gitbooks.io/canopen-ds301-cn/content/chapter7.5.html
namespace ymd::canopen{

#ifndef CANOPEN_MAX_PERDEF_ERROR
static constexpr size_t NUM_CANOPEN_MAX_PERDEF_ERROR = 8u;
#endif

template<uint16_t NUM_PRE_INDEX, uint8_t NUM_SUB_INDEX>
struct _ob_t{
    using type = void;
};


struct PredefinedError{
    uint16_t additive_error;
};


};

namespace ymd::canopen::basic{
using namespace canopen;
using namespace canopen::primitive;


// 此对象提供有关设备类型的信息。该对象描述了逻辑设备类型及其功能。它由两个16位域组成，一个描述所用设备协议或应用协议，
// 另一个给出逻辑设备的附加功能信息。附加的信息参数为设备协议和应用协议所指定。其说明不属于本文范围，定义于相应的设备协议和应用协议。
// 值定义
// 该值为0000h表示逻辑设备不遵守标准设备协议。在这种情况下附加的信息应为0000h(如果没有更多的逻辑设备)或FFFFh(如果还有其它逻辑设备)。
// 多逻辑设备其附加信息应为FFFFh且其设备协议应为对象字典中第一逻辑设备。
// 所有其他逻辑设备模块的协议标识于对象67FFh + x * 800h且x = 逻辑设备内部编号(从1到8)减去1。这些对象将描述逻辑设备的设备类型，
// 与对象1000h具有相同的值定义。
struct ControlWordReg{
    //控制字寄存器 只读32位
    static constexpr uint16_t NUM_PRE_IDX = 0x1000;
    static constexpr uint8_t NUM_UNIQUE_SUB_IDX = 0x00;

    uint16_t protocol_version;
    uint16_t extra_msg;
};

// 此对象提供错误信息记录，CANopen设备将内部错误记录映射到该对象，此为应急对象的一部分。
// https://blog.csdn.net/qq_15181569/article/details/106191562
struct ErrorReg{
    //错误寄存器 只读8位
    static constexpr uint16_t NUM_PRE_IDX = 0x1001;
    static constexpr uint8_t NUM_UNIQUE_SUB_IDX = 0x00;

    uint8_t generic:1;
    uint8_t current:1;
    uint8_t volage:1;
    uint8_t temperature:1;
    uint8_t communication:1;
    uint8_t device_specifiec:1;
    uint8_t __resv__:1;
    uint8_t manufacturer_specifiec:1;
};

struct ManufacturerStatusReg{
    //厂商信息 只读32位
    static constexpr uint16_t NUM_PRE_IDX = 0x1002;
    static constexpr uint8_t NUM_UNIQUE_SUB_IDX = 0x0;
    
    uint32_t manufacturer_id;

};

struct PerdefErrFieldReg{
    static constexpr uint16_t NUM_PRE_IDX = 0x1003;

    constexpr void push_error(const PredefinedError & error){
        error_queue_.push_front(error);
    }
    
    [[nodiscard]] constexpr size_t get_error_count(){
        return error_queue_.size();
    }

    [[nodiscard]] constexpr size_t get_subentries_count(){
        return error_queue_.size();
    }

    constexpr Result<PredefinedError, SdoAbortCode> get_error(size_t idx){
        if(idx >= error_queue_.size())
            return Err(SdoAbortCode::NoDataAvailable);
        return Ok();
    }

    #if 0
    SdoAbortCode read(const std::span<uint8_t> pbuf, const OdSubIndex sidx) const {
        static constexpr OdSubIndex base_idx = 1;

        if(unlikely(sidx) < 1){
            pbuf[0] = uint8_t(getErrorCnt());
            return Ok();
        }

        const auto offset = size_t(sidx) - size_t(base_idx);
        const auto may_err = getError(offset);

        if(may_err.has_value()){
            *(reinterpret_cast<Error *>(pbuf.data())) = may_err.value();
            return Ok();
        }else{
            //企图获取超界的错误
            return SdoAbortCode::NoValidData;
        }
    }
    #endif
private:
    // RingBuf<PredefinedError, NUM_CANOPEN_MAX_PERDEF_ERROR> error_queue_;
    RingMemento<PredefinedError, NUM_CANOPEN_MAX_PERDEF_ERROR> error_queue_;
};

struct CobidSyncMsgReg{
    static constexpr uint16_t NUM_PRE_IDX = 0x1005;
    static constexpr uint8_t NUM_UNIQUE_SUB_IDX = 0x0;

    uint32_t cobid:29;
    uint32_t frame:1;//0:11位CANID,1:29位CANID
    uint32_t gen:1;//所在CANOPEN设备是否发出同步消息
    uint32_t :1;
};

struct CommCyclicPeriodReg{
    static constexpr uint16_t NUM_PRE_IDX = 0x1006;
    static constexpr uint8_t NUM_UNIQUE_SUB_IDX = 0x0;

    uint32_t period_us;//单位us
};

struct SyncWindowLengthReg{
    static constexpr uint16_t NUM_PRE_IDX = 0x1007;
    static constexpr uint8_t NUM_UNIQUE_SUB_IDX = 0x0;

    uint32_t length;
};



struct DeviceNameReg{
    // 设备名称寄存器 只读 字符串类型
    static constexpr uint16_t NUM_PRE_IDX = 0x1008;
    static constexpr uint8_t NUM_UNIQUE_SUB_IDX = 0x0;

};

struct HardwareVersionReg{
    static constexpr uint16_t NUM_PRE_IDX = 0x1009;
    static constexpr uint8_t NUM_UNIQUE_SUB_IDX = 0x0;

};

struct SoftwareVersionReg{
    static constexpr uint16_t NUM_PRE_IDX = 0x1010;
    static constexpr uint8_t NUM_UNIQUE_SUB_IDX = 0x0;

};

struct NodeGuardingPeriodReg{
    // 节点守护时间寄存器 可读写 16位无符号整数
    static constexpr uint16_t NUM_PRE_IDX = 0x100C;
    static constexpr uint8_t NUM_UNIQUE_SUB_IDX = 0x0;

    uint8_t val;
};

struct NodeGuardingPeriodFracReg{
    // 节点守护时间寄存器 可读写 16位无符号整数
    static constexpr uint16_t NUM_PRE_IDX = 0x100D;
    static constexpr uint8_t NUM_UNIQUE_SUB_IDX = 0x0;

    uint16_t val;
};


struct TimeStampReg{
    static constexpr uint16_t NUM_PRE_IDX = 0x1012;
    static constexpr uint8_t NUM_UNIQUE_SUB_IDX = 0x0;

    uint32_t cobid:29;
    uint32_t frame:1;
    uint32_t produce:1;
    uint32_t consume:1;
};

struct GpTimeStampReg{
    static constexpr uint16_t NUM_PRE_IDX = 0x1013;
    static constexpr uint8_t NUM_UNIQUE_SUB_IDX = 0x0;

    uint32_t timestamp;
};


struct EmcyCobidReg{
    static constexpr uint16_t NUM_PRE_IDX = 0x1014;
    static constexpr uint8_t NUM_UNIQUE_SUB_IDX = 0x0;

    uint32_t canid:29;
    uint32_t frame:1;
    const uint32_t __resv__:1 = 0;
    uint32_t valid:1;
};


struct EmcyDepressTimeReg{
    static constexpr uint16_t NUM_PRE_IDX = 0x1015;
    static constexpr uint8_t NUM_UNIQUE_SUB_IDX = 0x0;

    uint16_t time;
};

struct ConsumerHeartbeatOverTimeReg{
    //心跳时间寄存器 可读写 32位无符号整数
    static constexpr uint16_t NUM_PRE_IDX = 0x1016;
    static constexpr uint8_t NUM_UNIQUE_SUB_IDX = 0x0;

    uint16_t time;
    uint8_t node_id;
    const uint8_t __resv__ = 0;
};

struct ProducterHeartbeatOverTimeReg{
    //心跳时间寄存器 可读写 32位无符号整数
    static constexpr uint16_t NUM_PRE_IDX = 0x1016;
    static constexpr uint8_t NUM_UNIQUE_SUB_IDX = 0x0;

    uint16_t time;
};

struct IdentificationReg{
    static constexpr uint16_t NUM_PRE_IDX = 0x1023;
};

}