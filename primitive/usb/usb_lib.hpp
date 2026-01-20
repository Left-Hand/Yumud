#pragma once


#include <cstdint>
#include "core/utils/Option.hpp"
#include "usb_control.hpp"
#include "usb_types.hpp"


// 参考实现
// MIT License
// https://github.com/embassy-rs/embassy/blob/4c1112b3df8058bdc9cbddaad9981478fad74601/embassy-usb/src/lib.rs

namespace ymd::usb{

enum class UsbDeviceState:uint8_t{
    /// The USB device has no power.
    Unpowered,

    /// The USB device is disabled.
    Disabled,

    /// The USB device has just been enabled or reset.
    Default,

    /// The USB device has received an address from the host.
    Addressed,

    /// The USB device has been configured and is fully functional.
    Configured,
};

enum class RemoteWakeupError{
    /// The USB device is not suspended, or remote wakeup was not enabled.
    InvalidState,
    /// The underlying driver doesn't support remote wakeup.
    Unsupported,
};


struct events{
    struct [[nodiscard]] Enabled final{
        bool is_enabled;
    };
    
    struct [[nodiscard]] Reset final{
    };

    struct [[nodiscard]] Addressed final{
        uint8_t address;
    };

    struct [[nodiscard]] Configured final{
        uint8_t config_value;
    };

    struct [[nodiscard]] Suspended final{
        bool is_suspended;
    };

    struct [[nodiscard]] RemoteWakeupEnabled final{
        bool is_enabled;
    };

    struct [[nodiscard]] SetAltermateSetting final{
        InterfaceNumber iface; 
        uint8_t alternate_setting;
    };
};


class HandlerIntf {
public:
    virtual ~HandlerIntf() = 0;
    
    // 设备启用/禁用回调
    virtual void enabled(bool enabled) = 0;
    
    // 总线重置事件回调
    virtual void reset() = 0;
    
    // 设备地址分配回调
    virtual void addressed(uint8_t addr) = 0;
    
    // 配置状态更改回调
    virtual void configured(bool configured) = 0;
    
    // 挂起状态更改回调
    virtual void suspended(bool suspended) = 0;
    
    // 远程唤醒功能启用/禁用回调
    virtual void remote_wakeup_enabled(bool enabled) = 0;
    
    // 设置接口的备用设置
    virtual void set_alternate_setting(InterfaceNumber iface, uint8_t alternate_setting) = 0;
    
    // 控制端点OUT事务处理
    virtual Option<OutResponse> control_out(const Request & req, std::span<const uint8_t> bytes) = 0;
    
    // 控制端点IN事务处理
    virtual Option<InResponse> control_in(const Request & req, std::span<uint8_t> bytes) = 0;

    // 获取字符串描述符
    virtual Option<StringView> get_string(StringIndex index,uint16_t lang_id) = 0;
};

}