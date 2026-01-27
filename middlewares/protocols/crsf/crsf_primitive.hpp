#pragma once

#include "primitive/arithmetic/percentage.hpp"

#include "core/int/uint24_t.hpp"
#include "core/math/realmath.hpp"
#include "core/math/float/fp32.hpp"
#include "core/string/view/string_view.hpp"
#include "core/string/owned/char_array.hpp"
#include "core/string/view/uchars_view.hpp"
#include "core/string/utils/optional_uchar_ptr.hpp"

#include "crsf_utils.hpp"

// https://github.com/tbs-fpv/tbs-crsf-spec/blob/main/crsf.md
// https://github.com/elodin-sys/elodin/blob/1d1c42139e18fe6d5b425b64ca8b55b1d38741f2/fsw/sensor-fw/src/crsf.rs

namespace ymd::crsf{

static constexpr size_t MAX_PACKET_BYTES = 64;

struct [[nodiscard]] DeviceAddress final{
    using Self = DeviceAddress;

    enum class [[nodiscard]] Kind:uint8_t{
        Broadcast = 0x00,
        Cloud = 0x0E,
        UsbDevice = 0x10,
        BluetoothModule = 0x12,
        WiFiReceiver = 0x13,
        VideoReceiver = 0x14,
        OSD_TBS_CORE_PNP_PRO = 0x80,
        Esc1 = 0x90,
        Esc2 = 0x91,
        Esc3 = 0x92,
        Esc4 = 0x93,
        Esc5 = 0x94,
        Esc6 = 0x95,
        Esc7 = 0x96,
        Esc8 = 0x97,
        VoltageSensor = 0xC0,
        GPS_PNP_PRO_GPS = 0xC2,
        TBS_Blackbox = 0xC4,
        FlightController = 0xC8,
        RaceTag = 0xCC,
        Vtx = 0xCE,
        RemoteControl = 0xEA,
        RcReceiver = 0xEC,
        RcTransmitter = 0xEE,
    };

    using enum Kind;

    constexpr DeviceAddress(const Kind kind): kind_((kind)){;}

    static constexpr Option<Self> try_from_bits(const uint8_t b){ 
        switch(std::bit_cast<Kind>(b)){
            case Broadcast:
            case Cloud:
            case UsbDevice:
            case BluetoothModule:
            case WiFiReceiver:
            case VideoReceiver:
            case OSD_TBS_CORE_PNP_PRO:
            case Esc1:
            case Esc2:
            case Esc3:
            case Esc4:
            case Esc5:
            case Esc6:
            case Esc7:
            case Esc8:
            case VoltageSensor:
            case GPS_PNP_PRO_GPS:
            case TBS_Blackbox:
            case FlightController:
            case RaceTag:
            case Vtx:
            case RemoteControl:
            case RcReceiver:
            case RcTransmitter:
                return Some(Self{std::bit_cast<Kind>(b)});
        }
        return None;
    }

    [[nodiscard]] constexpr Kind kind() const {
        return kind_;
    }

    [[nodiscard]] constexpr bool is_esc() const {
        switch(kind()){
            case Kind::Esc1 ... Kind::Esc8: return true;
            default: return false;
        }
    }

    [[nodiscard]] constexpr bool is_boardcast() const {
        return kind() == Kind::Broadcast;
    }

    [[nodiscard]] constexpr bool is_rc_receiver() const {
        return kind() == Kind::RcReceiver;
    }

    [[nodiscard]] constexpr bool is_rc_transmitter() const {
        return kind() == Kind::RcTransmitter;
    }

    [[nodiscard]] constexpr bool operator ==(const DeviceAddress & rhs) const {
        return kind() == rhs.kind();
    }

    [[nodiscard]] constexpr bool operator ==(const Kind rhs_kind) const {
        return kind() == rhs_kind;
    }
private:
    Kind kind_;
};

enum class [[nodiscard]] FrameType:uint8_t{
    GpsFrame = 0x02,
    BatteryFrame = 0x08,
    LinkStatistics = 0x14,
    RcChannelsPacked = 0x16,
    SubsetRcChannelsPacked = 0x17,
    LinkStatisticsRx = 0x1C,
    LinkStatisticsTx = 0x1D,
    Attitude = 0x1E,
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

    [[nodiscard]] constexpr bool is_integral() const {
        switch(bits){
            case static_cast<uint8_t>(Kind::U8) ... static_cast<uint8_t>(Kind::I32):
                return true;
            default: return false;
        }
    }

    [[nodiscard]] constexpr bool is_float() const {return is(Kind::Float);}
    [[nodiscard]] constexpr bool is_string() const {return is(Kind::String);}
    [[nodiscard]] constexpr bool is_folder() const {return is(Kind::Folder);}
    [[nodiscard]] constexpr bool is_command() const {return is(Kind::Command);}
    [[nodiscard]] constexpr bool is_info() const {return is(Kind::Info);}
    [[nodiscard]] constexpr bool is_text_selection() const {return is(Kind::TextSelection);}


    using enum Kind;
};


enum class [[nodiscard]] CommandStatus:uint8_t{

    Ready = 0, //--> 反馈
    Start = 1, //<-- 输入
    Progress = 2, //--> 反馈
    ConfirmationNeeded = 3, //--> 反馈
    Confirm = 4, //<-- 输入
    Cancel = 5, //<-- 输入
    Poll = 6, //<-- 输入
};


}

namespace ymd::crsf{
using math::int24_t, math::uint24_t;

template<size_t N>
using ustr = str::UCharsView<N>;


[[nodiscard]] static constexpr uint16_t TICKS_TO_US(uint16_t ticks) {
    return (ticks - 992) * 5 / 8 + 1500;
}

[[nodiscard]] static constexpr uint16_t US_TO_TICKS(uint16_t us) {
    return (us - 1500) * 8 / 5 + 992;
}


}