#pragma once

#include <cstdint>
#include "core/string/string_view.hpp"
#include "core/string/char_array.hpp"
#include "core/math/float/fp32.hpp"

// https://github.com/tbs-fpv/tbs-crsf-spec/blob/main/crsf.md

namespace ymd::crsf{

static constexpr size_t MAX_PACKET_BYTES = 64;

enum class DeviceAddress:uint8_t{
    Broadcast = 0x00,
    Cloud = 0x0E,
    UsbDevice = 0x10,
    BluetoothModule = 0x12,
    WiFiReceiver = 0x13,
    VideoReceiver = 0x14,
    OSD_TBS_CORE_PNP_PRO = 0x80,
    ESC1 = 0x90,
    ESC2 = 0x91,
    ESC3 = 0x92,
    ESC4 = 0x93,
    ESC5 = 0x94,
    ESC6 = 0x95,
    ESC7 = 0x96,
    ESC8 = 0x97,
    VoltageSensor = 0xC0,
    GPS_PNP_PRO_GPS = 0xC2,
    TBS_Blackbox = 0xC4,
    FlightController = 0xC8,
    RaceTag = 0xCC,
    VTX = 0xCE,
    RemoteControl = 0xEA,
    RcReceiver = 0xEC,
    RcTransmitter = 0xEE,
};



struct [[nodiscard]] DataType final{
    using Self = DataType;

    enum class [[nodiscard]] Kind:uint8_t{
        U8              = 0,  // 0x00 // !deprecated
        I8              = 1,  // 0x01 // !deprecated
        U16             = 2,  // 0x02 // !deprecated
        I16             = 3,  // 0x03 // !deprecated
        U32             = 4,  // 0x04 // !deprecated
        I32             = 5,  // 0x05 // !deprecated
        Float           = 8,  // 0x08
        TextSelection   = 9,  // 0x09
        String          = 10, // 0x0A
        Folder          = 11, // 0x0B
        Info            = 12, // 0x0C
        Command         = 13, // 0x0D

        // This type will be sent if a parameter number out of 
        // the device parameter range will be requested. 
        // It will be also sent as the last parameter to 
        // let the host know the end of the parameter list 
        // on a Parameters settings list (read request).
        OutOfRange      = 0x7f,
    };

    static constexpr uint8_t MAX_VALID_KIND = static_cast<uint8_t>(DataType::Kind::Command);


    uint8_t bits;

    constexpr DataType(const Kind kind): bits(static_cast<uint8_t>(kind)){;}
    constexpr Option<Self> try_from_u8(const uint8_t b){
        if(b == static_cast<uint8_t>(Kind::OutOfRange))
            return Option<Self>(Self{static_cast<Kind>(b)});
        if(b > MAX_VALID_KIND)
            return None;
        return Option<Self>(Self{static_cast<Kind>(b)});
    }

    [[nodiscard]] constexpr Kind kind_unchecked() const {
        return static_cast<Kind>(bits);
    }

    [[nodiscard]] constexpr bool is_valid() const {
        return bits <= MAX_VALID_KIND || bits == static_cast<uint8_t>(Kind::OutOfRange);
    }


    [[nodiscard]] constexpr bool is(const Kind kind) const {
        return kind_unchecked() == kind;
    }

    [[nodiscard]] constexpr bool is_float() const {return is(Kind::Float);}
    [[nodiscard]] constexpr bool is_string() const {return is(Kind::String);}
    [[nodiscard]] constexpr bool is_folder() const {return is(Kind::Folder);}


    using enum Kind;
};

enum class CommandStatus:uint8_t{

    Ready = 0, //--> 反馈
    Start = 1, //<-- 输入
    Progress = 2, //--> 反馈
    ConfirmationNeeded = 3, //--> 反馈
    Confirm = 4, //<-- 输入
    Cancel = 5, //<-- 输入
    Poll = 6, //<-- 输入
};

struct FloatContext{
    math::fp32         value;
    math::fp32         minimal;
    math::fp32         maximum;
    math::fp32         default_value;
    uint8_t         decimal_point;
    int32_t         step_size;
    StringView      unit;                       // 空终止测量单位字符串
};

struct StringContext{
    StringView      value;                      // 空终止字符串
};

struct FolderContext{
    std::span<const uint8_t>      list_of_children;
    // 用0xFF条目标记列表末尾
};

}