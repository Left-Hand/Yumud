#pragma once

#include <cstdint>

namespace ymd::usb{
struct [[nodiscard]] InterfaceNumber{
    uint8_t count;
};

struct [[nodiscard]] StringIndex{
    uint8_t count;
};

}


namespace ymd::usb{


enum class BcdUsb:uint16_t{
    V1_0 = 0x0100,
    V1_1 = 0x0110,
    V1_12 = 0x0112,
    V2_0 = 0x0200,
    V2_1 = 0x0210,
    V3_0 = 0x0300,
    V3_1 = 0x0310,
    V3_2 = 0x0320,
    V3_3 = 0x0330,
};



enum class UsbClassCode : uint8_t {
    Audio = 0x01,
    CDCControl = 0x02,          // 通信设备类 - 控制接口
    HID = 0x03,                 // 人类接口设备类
    Physical = 0x05,            // 物理接口设备类
    Image = 0x06,               // 图像类
    Printer = 0x07,             // 打印机类
    MassStorage = 0x08,         // 大容量存储类
    Hub = 0x09,                 // 集线器类
    CDCData = 0x0A,             // 通信设备类 - 数据接口
    SmartCard = 0x0B,           // 智能卡类
    ContentSecurity = 0x0D,     // 内容安全类
    Video = 0x0E,               // 视频类
    PersonalHealthcare = 0x0F,  // 个人健康护理类
    AudioVideo = 0x10,          // 音频/视频设备类
    Billboard = 0x11,           // 广告牌类
    USBTypeCBridge = 0x12,      // USB 类型 C 桥接类
    USBBulkDisplay = 0x13,     // USB 显示类
    MCTPOverUSB = 0x14,        // MCTP over USB 类
    I3C = 0x3c,                 // I3C 类
    DiagnosticDevice = 0xDC,    // 诊断设备类
    WirelessController = 0xE0,  // 无线控制器类
    Miscellaneous = 0xEF,       // 杂项类
    ApplicationSpecific = 0xFE, // 应用特定类
    VendorSpecific = 0xFF       // 厂商特定类
};

enum class HidSubClassCode : uint8_t {
    None = 0x00,
    Boot = 0x01,
    //Reserverd
};

enum class HidProtocolCode : uint8_t {
    None = 0x00,
    Keyboard = 0x01,
    Mouse = 0x02,
    // Resrvered
};

struct UsbInterfaceDescrPayload{
    // 接口编号，用于标识设备中的不同接口
    uint8_t bInterfaceNumber;

    // 备用设置值，用于标识接口的不同备用设置
    uint8_t bAlternateSetting;

    // 该接口使用的端点数量
    uint8_t bNumEndpoints;

    // 接口类代码，标识接口所属的类
    UsbClassCode bInterfaceClass;

    // 接口子类代码，标识接口所属的具体子类
    HidSubClassCode bInterfaceSubClass;

    // 接口协议代码，标识接口使用的具体协议
    HidProtocolCode bInterfaceProtocol;

    // 接口字符串描述符的索引，用于提供接口的描述性字符串
    // 为0使用默认描述
    uint8_t iInterface;
};
}