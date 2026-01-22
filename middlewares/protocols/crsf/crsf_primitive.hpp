#pragma once

#include "primitive/arithmetic/percentage.hpp"

#include "core/int/uint24_t.hpp"
#include "core/math/realmath.hpp"
#include "core/math/float/fp32.hpp"
#include "core/string/view/string_view.hpp"
#include "core/string/own/char_array.hpp"
#include "core/string/view/uchars_view.hpp"
#include "core/string/utils/optional_uchar_ptr.hpp"

#include "crsf_utils.hpp"

// https://github.com/tbs-fpv/tbs-crsf-spec/blob/main/crsf.md

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

    [[nodiscard]] constexpr bool operator ==(const DeviceAddress & rhs) const {
        return kind() == rhs.kind();
    }

    [[nodiscard]] constexpr bool operator ==(const Kind rhs_kind) const {
        return kind() == rhs_kind;
    }
private:
    Kind kind_;
};

using FrameType = uint8_t;

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

struct [[nodiscard]] AltitudeCode final{
    using Self = AltitudeCode;
    // 高度值取决于MSB（第15位）：
    // MSB = 0：高度以分米表示——10000分米偏移（0代表-1000米;10000代表0米（起始高度）;0x7fff表示2276.7米）;
    // MSB = 1：高度以米为单位。没有任何偏移。

    uint16_t bits;

    static constexpr int32_t ALT_MIN_DM = 10000;                     //minimum altitude in dm
    static constexpr int32_t ALT_THRESHOLD_DM = 0x8000 - ALT_MIN_DM; //altitude of precision
                                            // changing in dm
    static constexpr int32_t ALT_MAX_DM = 0x7ffe * 10 - 5;           //maximum altitude in dm
    static constexpr uint16_t INVALID_VALUE = 0xffff;

    [[nodiscard]] constexpr Option<Self> try_from_u16(const uint16_t b){
        if(b == INVALID_VALUE)
            return None;
        return Some(Self{b});
    }


    [[nodiscard]] constexpr bool is_represented_in_meters() const{
        return (bits & 0x8000) != 0;
    }

    [[nodiscard]] constexpr bool is_invalid() const { 
        return bits == INVALID_VALUE;
    }

    [[nodiscard]] constexpr int32_t to_dm() const {
        if(is_invalid()) __builtin_trap();
        if(bits & 0x8000)
            return static_cast<int32_t>((bits & 0x7fff)) * 10;
        else
            return static_cast<int32_t>(bits) - 10000;
    }

    [[nodiscard]] constexpr int32_t to_meters() const {
        if(is_invalid()) __builtin_trap();
        if(bits & 0x8000)
            return static_cast<int32_t>((bits & 0x7fff));
        else
            return (static_cast<int32_t>(bits) - 10000) / 10;
    }

    static constexpr Self from_dm(const int32_t altitude_dm){
        const auto bits = dm_to_bits(altitude_dm);
        return Self{bits};
    }

    static constexpr Self from_meters(const int32_t altitude_m){
        uint16_t bits = INVALID_VALUE;
        if(altitude_m < -1000) {
            bits = 0;  // minimum
        } else if(altitude_m > 32766) {
            bits = 0xfffe;  // maximum
        } else if(altitude_m < 3276) {  // dm-resolution range
            bits = static_cast<uint16_t>(altitude_m * 10 + ALT_MIN_DM);
        } else {
            bits = static_cast<uint16_t>((altitude_m + 5) / 10) | 0x8000;  // meter-resolution range
        }
        return Self{bits};
    }

    static constexpr uint16_t dm_to_bits(const int32_t altitude_dm){
        if(altitude_dm < -static_cast<int32_t>(ALT_MIN_DM))               //less than minimum altitude
            return 0;                               //minimum
        if(altitude_dm > static_cast<int32_t>(ALT_MAX_DM))                //more than maximum
            return 0xfffe;                          //maximum
        if(altitude_dm < static_cast<int32_t>(ALT_THRESHOLD_DM))          //dm-resolution range
            return static_cast<uint16_t>(altitude_dm + ALT_MIN_DM);
        return static_cast<uint16_t>(((altitude_dm + 5) / 10) | 0x8000);   //meter-resolution range
    } 
};

struct [[nodiscard]] VerticalSpeedCode final{
    using Self = VerticalSpeedCode;
    
    static constexpr int KL = 100;       // linearity constant;
    static constexpr iq16 KR = iq16(.026); // range constant;

    int8_t bits;

    [[nodiscard]] static constexpr int8_t cm_per_seconds_to_bits (int16_t val){
        const iq16 base = math::log(math::abs(static_cast<iq16>(val))/KL + 1)/KR;
        if(val >= 0)
            return static_cast<int8_t>(base);
        else
            return static_cast<int8_t>(-base);
    }

    [[nodiscard]] static constexpr int16_t bits_to_cm_per_seconds(int8_t bits_val){
        const int32_t u_result = static_cast<int32_t>((math::exp(math::abs(static_cast<iq16>(bits_val) * KR)) - 1) * KL);
        if(bits_val >= 0) 
            return static_cast<int16_t>(u_result);
        else 
            return static_cast<int16_t>(-u_result);
    }

    [[nodiscard]] constexpr int16_t to_cm_per_second() const {
        return bits_to_cm_per_seconds(bits);
    }

    static constexpr Self from_cm_per_second(int16_t val) {
        const auto bits = cm_per_seconds_to_bits(val);
        return Self{bits};
    }
};

struct [[nodiscard]] VoltageCode final{
    using Self = VoltageCode;
    
    int16_t bits;  // LSB = 10 µV

    [[nodiscard]] constexpr float to_volts() const {
        return static_cast<float>(bits) * 0.00001f;  // Convert from 10µV units to volts
    }

    [[nodiscard]] constexpr float to_mv() const {
        return bits / 100;  // Convert to millivolts
    }

    static constexpr Self from_volts(float volts) {
        const int16_t bits = static_cast<int16_t>(volts * 100000.0f);
        return Self{bits};
    }

    static constexpr Self from_mv(float mv) {
        const int16_t bits = static_cast<int16_t>(mv * 100);
        return Self{bits};
    }
};

struct [[nodiscard]] CurrentCode final{
    using Self = CurrentCode;
    
    int16_t bits;  // LSB = 10 µA

    [[nodiscard]] constexpr float to_amperes() const {
        return static_cast<float>(bits) * 0.00001f;  // Convert from 10µA units to amperes
    }

    [[nodiscard]] constexpr int16_t to_ma() const {
        return bits / 100;  // Convert to milliamperes
    }

    static constexpr Self from_amperes(float amps) {
        const int16_t bits = static_cast<int16_t>(amps * 100000.0f);
        return Self{bits};
    }

    static constexpr Self from_ma(int32_t ma) {
        const int16_t bits = static_cast<int16_t>(ma * 100);
        return Self{bits};
    }
};

struct [[nodiscard]] RpmCode final{
    using Self = RpmCode;
    
    int24_t bits;

    [[nodiscard]] constexpr int32_t to_rpm() const {
        return static_cast<int32_t>(bits);
    }

    static constexpr Self from_rpm(int32_t rpm) {
        const int24_t bits = static_cast<int24_t>(rpm);
        return Self{.bits = int24_t(bits)};
    }
};

struct [[nodiscard]] TemperatureCode final{
    using Self = TemperatureCode;
    
    int16_t bits;  // deci-degree (tenths of a degree) Celsius

    [[nodiscard]] constexpr iq16 to_celsius() const {
        return static_cast<iq16>(bits) * iq16(0.1);
    }

    static constexpr Self from_celsius(iq16 celsius) {
        const auto bits = static_cast<int16_t>(celsius * 10);
        return Self{bits};
    }
};

struct [[nodiscard]] GpsCoordinateCode final{
    using Self = GpsCoordinateCode;
    
    // degree / 10`000`000
    int32_t bits;

    template<typename T>
    [[nodiscard]] constexpr T to_degrees() const {
        if constexpr(std::is_floating_point_v<T>){
            return static_cast<T>(bits) / 10000000.0;
        }

        __builtin_unreachable();
    }

    template<typename T>
    [[nodiscard]] constexpr T to_turns() const {
        if constexpr(std::is_floating_point_v<T>){
            return static_cast<T>(bits) / 10000000.0;
        }else if constexpr(std::is_same_v<T, iq16>){ 
            static constexpr uint64_t FACTOR = 
                static_cast<long double>(1ull << (16 + 32)) / (1E7 * 360);
            const auto ret_bits = 
                static_cast<int32_t>((static_cast<int64_t>(bits) * FACTOR) >> 32);
            return iq16::from_bits(ret_bits);
        }

        __builtin_unreachable();
    }

    template<typename T>
    static constexpr Self from_degrees(T degrees) {
        if constexpr(std::is_floating_point_v<T>){
            const int32_t bits = static_cast<int32_t>(degrees * 10000000.0);
            return Self{bits};
        }

        __builtin_unreachable();
    }

    template<typename T>
    static constexpr Self from_turns(T turns) {
        if constexpr(std::is_floating_point_v<T>){
            const int32_t bits = static_cast<int32_t>(turns * 10000000.0 / 360.0);
            return Self{bits};
        }else if constexpr(std::is_same_v<T, iq16>){
            //TODO
            const int32_t bits = 0;
            __builtin_trap();
            return Self{bits};
        }

        __builtin_unreachable();
    }
};

struct [[nodiscard]] GpsHeadingCode final{
    using Self = GpsHeadingCode;
    
    uint16_t bits;  // degree / 100

    [[nodiscard]] constexpr float to_degrees() const {
        return static_cast<float>(bits) * 0.01f;
    }

    static constexpr Self from_degrees(float degrees) {
        const auto bits = static_cast<uint16_t>(degrees * 100.0f);
        return Self{bits};
    }
};

struct [[nodiscard]] GpsGroundSpeedCode final{
    using Self = GpsGroundSpeedCode;
    
    uint16_t bits;  // km/h / 100

    [[nodiscard]] constexpr float to_kmh() const {
        return static_cast<float>(bits) * 0.01f;
    }

    static constexpr Self from_kmh(float kmh) {
        const auto bits = static_cast<uint16_t>(kmh * 100.0f);
        return Self{bits};
    }

};

struct [[nodiscard]] AirspeedCode final{
    using Self = AirspeedCode;
    
    uint16_t bits;  // Airspeed in 0.1 * km/h (hectometers/h)

    [[nodiscard]] constexpr float to_kmh() const {
        return static_cast<float>(bits) * 0.1f;
    }


    static constexpr Self from_kmh(float kmh) {
        const auto bits = static_cast<uint16_t>(kmh * 10.0f);
        return Self{bits};
    }
};

struct [[nodiscard]] AttitudeAngleCode final{
    using Self = AttitudeAngleCode;
    
    int16_t bits;  // LSB = 100 µrad

    [[nodiscard]] constexpr float to_radians() const {
        return static_cast<float>(bits) * 0.0001f;  // Convert from 100µrad to radians
    }

    [[nodiscard]] constexpr float to_degrees() const {
        return static_cast<float>(bits) * 0.0001f * RAD2DEG_RATIO;  // radians to degrees
    }

    static constexpr Self from_radians(float radians) {
        const auto bits = static_cast<int16_t>(radians * 10000.0f);
        return Self{bits};
    }

    static constexpr Self from_degrees(float degrees) {
        const auto bits = static_cast<int16_t>(degrees * 10000.0f * DEG2RAD_RATIO);
        return Self{bits};
    }
};

struct [[nodiscard]] RssiCode final{
    using Self = RssiCode;
    
    uint8_t bits;  // RSSI (dBm * -1)

    [[nodiscard]] constexpr int8_t to_dbm() const {
        return static_cast<int8_t>(-bits);
    }

    [[nodiscard]] constexpr uint8_t to_percent() const {
        // Convert RSSI to percentage (approximate mapping)
        if (bits >= 127) return 0;
        return static_cast<uint8_t>((127 - bits) * 100 / 127);
    }

    static constexpr Self from_dbm(int8_t dbm) {
        const auto bits = static_cast<uint8_t>(-dbm);
        return Self{bits};
    }
};

struct [[nodiscard]] LinkQualityCode final{
    using Self = LinkQualityCode;
    
    uint8_t bits;  // Link quality (%)

    [[nodiscard]] constexpr uint8_t to_percents() const {
        return bits;
    }

    static constexpr Self from_percents(uint8_t percents) {
        if(percents > 100) __builtin_trap();
        return Self{static_cast<uint8_t>(percents)};
    }

    static constexpr Option<Self> try_from_bits(uint8_t b) {
        if(b > 100) return None;
        return Some(Self{static_cast<uint8_t>(b)});
    }
};

struct [[nodiscard]] SnrCode final{
    using Self = SnrCode;
    
    int8_t bits;  // SNR (dB)

    [[nodiscard]] constexpr int8_t to_db() const {
        return bits;
    }

    static constexpr Self from_db(int8_t db) {
        const auto bits = db;
        return Self{bits};
    }
};

struct [[nodiscard]] RfPowerDbmCode final{
    using Self = RfPowerDbmCode;
    
    uint8_t bits;  // rf power in dBm

    [[nodiscard]] constexpr int8_t to_dbm() const {
        return static_cast<int8_t>(bits);
    }

    static constexpr Self from_dbm(int8_t dbm) {
        const auto bits = static_cast<uint8_t>(dbm);
        return Self{bits};
    }
};

enum class [[nodiscard]] RfFps:uint8_t{
    _4 = 0,
    _50 = 1,
    _150 = 2,
};

static_assert(sizeof(RfFps) == 1);

enum class [[nodiscard]] RfPower:uint8_t{
    _0mW = 0,
    _10mW = 1,
    _25mW = 2,
    _100mW = 3,
    _500mW = 4,
    _1000mW = 5,
    _2000mW = 6,
    _250mW = 7,
    _50mW = 8,
};

static_assert(sizeof(RfPower) == 1);

struct [[nodiscard]] FpsCode final{
    using Self = FpsCode;
    
    uint8_t bits;  // rf frames per second (fps / 10)

    [[nodiscard]] constexpr uint16_t to_fps() const {
        return static_cast<uint16_t>(bits) * 10;
    }

    static constexpr Self from_fps(uint16_t fps) {
        const uint8_t bits = static_cast<uint8_t>(fps / 10u);
        return Self{bits};
    }
};

struct [[nodiscard]] PressureCode final{
    using Self = PressureCode;
    
    int32_t bits;  // Pascals

    [[nodiscard]] constexpr float to_pa() const {
        return static_cast<float>(bits);
    }

    static constexpr Self from_pa(float pa) {
        const auto bits = static_cast<int32_t>(pa);
        return Self{bits};
    }

};

struct [[nodiscard]] TemperatureCentidegreeCode final{
    using Self = TemperatureCentidegreeCode;
    
    int32_t bits;  // centidegrees

    [[nodiscard]] constexpr float to_celsius() const {
        return static_cast<float>(bits) * 0.01f;
    }

    static constexpr Self from_celsius(float celsius) {
        const auto bits = static_cast<int32_t>(celsius * 100.0f);
        return Self{bits};
    }
};

struct [[nodiscard]] PassthroughTelemetryPacket final{
    uint16_t appid;
    uint32_t data;
};

}