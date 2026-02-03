#pragma once

#include <cstdint>

namespace ymd::crsf{

struct [[nodiscard]] CommandType final {
    enum class [[nodiscard]] Kind:uint8_t{
        Fc = 0x01,
        BlueTooth = 0x03,
        Osd = 0x05,
        Vtx = 0x08,
        Led = 0x09,
        Generic = 0x0a,
        CrossFire = 0x10,
        FlowControl = 0x20,
        ScreenControl = 0x22,
        Log = 0x34,
        COnfirm = 0xff
    };
};

namespace commands{
struct [[nodiscard]] Fc final{
    enum class [[nodiscard]] SubCommand:uint8_t{
        ForceShutDown,
        ScaleChannel
    };

    SubCommand sub_command;
};


// - 0x01 重置
// - 0x02 启用
//   - uint8_t Enable (0 = 禁用, 1 = 启用)
// - 0x64 回声
struct [[nodiscard]] BlueTooth final{
    enum class [[nodiscard]] SubCommand:uint8_t{
        Reset = 0x01,
        Activation = 0x02,
        Echo = 0x03
    };

    SubCommand sub_command;
};

// 0x32.0x05 OSD命令
// - 0x01 发送按钮:
//   - uint8_t Buttons 按位 (位7=确定, 6=上, 5=下, 4=左, 3=右)
struct [[nodiscard]] Osd final{
    enum class [[nodiscard]] SubCommand:uint8_t{
        Button = 0x01
    };

    SubCommand sub_command;

    struct [[nodiscard]] Buttons final{
        uint8_t :3;
        uint8_t right:1;
        uint8_t left:1;
        uint8_t down:1;
        uint8_t up:1;
        uint8_t enter:1;
    };

    static_assert(sizeof(Buttons) == 1);
};

// 0x32.0x08 VTX命令
// - 0x01 已弃用 VTX 更改频道
//   - EVO, PRO32 HV, PRO32 NANO 仍支持此功能。
// - 0x02 设置频率
//   - uint16_t 频率 (5000 - 6000 MHz)
// - 0x03 已弃用 VTX 更改功率（移到0x08）
// - 0x04 开启PitMode（上电时）
//   - uint8_t PitMode:1 枚举 (0 = 关闭, 1 = 开启)
//   - uint8_t pitmode_control:2; (0=关闭, 1=开启, 2=激活, 3=失效保护)
//   - uint8_t pitmode_switch:4; (0=Ch5, 1=Ch5反向, … , 15=Ch12反向)
// - 0x05 从PitMode唤醒（纯命令）
// - 0x06 设置动态功率 (2020年5月15日在EVO, PRO32 HV, PRO32 NANO中)
//     注意：需以1Hz频率发送。如果3秒内未收到，VTX
//           将恢复到"0x08 设置功率"的功率设置
//   - uint8_t 功率 (dBm) (0dBm可视为PitMode功率)
// - 0x08 设置功率
//   - uint8_t 功率 (dBm) (0dBm可视为PitMode功率)
struct [[nodiscard]] Vtx final{
    enum class [[nodiscard]] SubCommand:uint8_t{ 
        ChangeChannel = 0x01,
        SetFrequency = 0x02,
        ChangePower = 0x03,
        EnablePitMode = 0x04,
        WakeUpFromPitMode = 0x05,
        SetDynamicPower = 0x06,
        SetPower = 0x08
    };

    SubCommand sub_command;
};


// 0x32.0x09 LED
// - 0x01 恢复默认设置（还原到目标特定设置）
// - 0x02 覆盖LED颜色（打包）
//   - 9位H (0-359°)
//   - 7位S (0-100%)
//   - 8位V (0-100%)
// - 0x03 覆盖脉冲（打包）
//   - uint16 持续时间（从起始颜色到停止颜色的毫秒数）
//   - 9位H_起始 (0-359°)
//   - 7位S_起始 (0-100%)
//   - 8位V_起始 (0-100%)
//   - 9位H_停止 (0-359°)
//   - 7位S_停止 (0-100%)
//   - 8位V_停止 (0-100%)
// - 0x04 覆盖闪烁（打包）
//   - uint16 间隔
//   - 9位H_起始 (0-359°)
//   - 7位S_起始 (0-100%)
//   - 8位V_起始 (0-100%)
//   - 9位H_停止 (0-359°)
//   - 7位S_停止 (0-100%)
//   - 8位V_停止 (0-100%)
// - 0x05 覆盖偏移（打包）
//   - uint16 间隔
//   - 9位H (0-359°)
//   - 7位S (0-100%)
//   - 8位V (0-100%)

struct [[nodiscard]] H9S7V8 final{
    uint16_t h:9;
    uint16_t s:7;
    uint16_t v:8;
};

struct [[nodiscard]] Led final{ 
    enum class [[nodiscard]] SubCommand:uint8_t{
        RestoreDefaults = 0x01,
        OverwriteColor = 0x02,
        OverwritePulse = 0x03,
        OverwriteFlash = 0x04,
        OverwriteOffset = 0x05
    };

    SubCommand sub_command;

    struct [[nodiscard]] OverwritePulse final{
        uint16_t duration;
        H9S7V8 start_color;
        H9S7V8 stop_color;
    };

    struct [[nodiscard]] OverwriteFlash final{
        uint16_t interval;
        H9S7V8 start_color;
        H9S7V8 stop_color;
    };

    struct [[nodiscard]] OverwriteOffset final{
        uint16_t interval;
        H9S7V8 color;
    };
};

// 0x32.0x0A 通用
// - 0x04 - 0x61 保留
// - 0x70 CRSF协议速度提议
//   - uint8_t 端口号
//   - uint32  建议的波特率
// - 0x71 CRSF协议速度提议响应
//   - uint8_t 端口号
//   - bool    响应 // (1 = 接受 / 0 = 拒绝)
struct [[nodiscard]] Generic final{
    enum class [[nodiscard]] SubCommand:uint8_t{
        ProposeSpeed = 0x70,
        ProposeSpeedResponse = 0x71
    };


    SubCommand sub_command;

    struct [[nodiscard]] ProposeSpeed final{
        uint8_t port;
        uint32_t baudrate;
    };

    struct [[nodiscard]] ProposeSpeedResponse final{
        uint8_t port;
        bool accepted;
    };
};

// 0x32.0x10 Crossfire
// - 0x01 将接收器设为绑定模式
// - 0x02 取消绑定模式
// - 0x03 设置绑定ID
// - 0x05 模型选择（选择模型/接收器的命令）
//   - uint8_t 模型编号
// - 0x06 当前模型选择（当前选择的查询帧）
// - 0x07 回复当前模型选择（当前选择的回复帧）
//   - uint8_t 模型编号
// - 0x08 保留
// - 0x09 保留
struct [[nodiscard]] CrossFire final{ 
    enum class [[nodiscard]] SubCommand:uint8_t{
        SetBindingMode = 0x01,
        CancelBindingMode = 0x02,
        SetBindingId = 0x03,
        ModelSelection = 0x05,
        CurrentModelSelection = 0x06,
        ReplyCurrentModelSelection = 0x07,
        Reserved = 0x08,
        Reserved2 = 0x09
    };

    SubCommand sub_command;
};

// 0x32.0x20 流控制帧
// 设备可以限制数据速率或订阅特定帧。
// - 0x01 订阅
//   - uint8_t  帧类型
//   - uint16_t 最大间隔时间 // 毫秒
// - 0x02 取消订阅
//   - uint8_t  帧类型
struct [[nodiscard]] FlowControl final{
    enum class [[nodiscard]] SubCommand:uint8_t{
        Subscribe = 0x01,
        Unsubscribe = 0x02
    };

    SubCommand sub_command;
};


}
}