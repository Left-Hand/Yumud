#pragma once

#include "../canopen_primitive/canopen_primitive.hpp"
#include "core/string/string_view.hpp"
#include "core/container/ringbuf.hpp"

namespace ymd::canopen{

#ifndef CANOPEN_MAX_PERDEF_ERROR
static constexpr size_t NUM_CANOPEN_MAX_PERDEF_ERROR = 8u;
#endif

template<uint16_t NUM_PRE_INDEX, uint8_t NUM_SUB_INDEX>
struct _ob_t{
    using type = void;
};


struct PredefinedError{
    uint32_t bits;
};

};

namespace ymd::canopen::basic{
using namespace canopen;
using namespace canopen::primitive;


template<typename T>
std::span<const uint8_t, sizeof(T)> as_le_bytes(const T * p_obj){
    return std::span(reinterpret_cast<const uint8_t *>(p_obj), sizeof(T));
}

template<typename T>
std::span<uint8_t, sizeof(T)> as_mut_le_bytes(T * p_obj){
    return std::span(reinterpret_cast<uint8_t *>(p_obj), sizeof(T));
}

struct ControlWordReg{
    //控制字寄存器 只读32位
    static constexpr uint16_t NUM_PRE_IDX = 0x1000;
    static constexpr uint8_t NUM_UNIQUE_SUB_IDX = 0x00;

    uint16_t protocol_version;
    uint16_t extra_msg;
};

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

// struct Ringlist
struct PerdefErrFieldReg{
    static constexpr uint16_t NUM_PRE_IDX = 0x1003;
    static constexpr uint8_t NUM_UNIQUE_SUB_IDX = 0x0;

    SdoAbortCode push_error(const PredefinedError & error){
        if(error_queue_.writable_size() <= 0) 
            return SdoAbortCode(SdoAbortError::OutOfMemory);
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
    RingBuf<PredefinedError, NUM_CANOPEN_MAX_PERDEF_ERROR> error_queue_;
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