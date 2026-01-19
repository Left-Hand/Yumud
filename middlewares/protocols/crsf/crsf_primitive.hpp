#pragma once

#include <cstdint>

#include "core/int/uint24_t.hpp"
#include "core/math/realmath.hpp"
#include "core/math/float/fp32.hpp"

#include "core/string/string_view.hpp"
#include "core/string/char_array.hpp"
#include "crsf_utils.hpp"

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
    Vtx = 0xCE,
    RemoteControl = 0xEA,
    RcReceiver = 0xEC,
    RcTransmitter = 0xEE,
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


}

namespace ymd::crsf{
using math::int24_t, math::uint24_t;


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

    static constexpr uint32_t ALT_MIN_DM = 10000;                     //minimum altitude in dm
    static constexpr uint32_t ALT_THRESHOLD_DM = 0x8000 - ALT_MIN_DM; //altitude of precision
                                            // changing in dm
    static constexpr uint32_t ALT_MAX_DM = 0x7ffe * 10 - 5;           //maximum altitude in dm

    [[nodiscard]] constexpr bool is_represented_in_meters() const{
        return (bits & 0x8000) != 0;
    }

    [[nodiscard]] constexpr int32_t to_dm() const {
        if(bits & 0x8000)
            return static_cast<int32_t>((bits & 0x7fff)) * 10;
        else
            return static_cast<int32_t>(bits) - 10000;
    }

    [[nodiscard]] constexpr int32_t to_meters() const {
        if(bits & 0x8000)
            return static_cast<int32_t>((bits & 0x7fff));
        else
            return (static_cast<int32_t>(bits) - 10000) / 10;
    }

    constexpr Self& from_dm(const int32_t altitude_dm){
        bits = dm_to_bits(altitude_dm);
        return *this;
    }

    constexpr Self& from_meters(const int32_t altitude_m){
        if(altitude_m < -1000) {
            bits = 0;  // minimum
        } else if(altitude_m > 32766) {
            bits = 0xfffe;  // maximum
        } else if(altitude_m < 3276) {  // dm-resolution range
            bits = static_cast<uint16_t>(altitude_m * 10 + ALT_MIN_DM);
        } else {
            bits = static_cast<uint16_t>((altitude_m + 5) / 10) | 0x8000;  // meter-resolution range
        }
        return *this;
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
    
    static constexpr int   Kl = 100;       // linearity constant;
    static constexpr iq16 Kr = iq16(.026); // range constant;

    int8_t bits;

    [[nodiscard]] static constexpr int8_t cm_per_seconds_to_bits (int16_t val){
        const iq16 base = math::log(math::abs(static_cast<iq16>(val))/Kl + 1)/Kr;
        if(val >= 0)
            return static_cast<int8_t>(base);
        else
            return static_cast<int8_t>(-base);
    }

    [[nodiscard]] static constexpr int16_t bits_to_cm_per_seconds(int8_t bits_val){
        const int32_t u_result = static_cast<int32_t>((math::exp(math::abs(static_cast<iq16>(bits_val) * Kr)) - 1) * Kl);
        if(bits_val >= 0) 
            return static_cast<int16_t>(u_result);
        else 
            return static_cast<int16_t>(-u_result);
    }

    [[nodiscard]] constexpr int16_t to_cm_per_second() const {
        return bits_to_cm_per_seconds(bits);
    }

    constexpr Self& from_cm_per_second(int16_t val) {
        bits = cm_per_seconds_to_bits(val);
        return *this;
    }
};

struct [[nodiscard]] VoltageCode final{
    using Self = VoltageCode;
    
    int16_t bits;  // LSB = 10 µV

    [[nodiscard]] constexpr float to_volts() const {
        return static_cast<float>(bits) * 0.01f;  // Convert from 10µV units to volts
    }

    [[nodiscard]] constexpr int16_t to_mv() const {
        return bits * 10;  // Convert to millivolts
    }

    constexpr Self& from_volts(float volts) {
        bits = static_cast<int16_t>(volts * 100.0f);
        return *this;
    }

    constexpr Self& from_mv(int32_t mv) {
        bits = static_cast<int16_t>(mv / 10);
        return *this;
    }
};

struct [[nodiscard]] CurrentCode final{
    using Self = CurrentCode;
    
    int16_t bits;  // LSB = 10 µA

    [[nodiscard]] constexpr float to_amperes() const {
        return static_cast<float>(bits) * 0.01f;  // Convert from 10µA units to amperes
    }

    [[nodiscard]] constexpr int16_t to_ma() const {
        return bits * 10;  // Convert to milliamperes
    }

    constexpr Self& from_amperes(float amps) {
        bits = static_cast<int16_t>(amps * 100.0f);
        return *this;
    }

    constexpr Self& from_ma(int32_t ma) {
        bits = static_cast<int16_t>(ma / 10);
        return *this;
    }
};

struct [[nodiscard]] RpmCode final{
    using Self = RpmCode;
    
    int24_t bits;

    [[nodiscard]] constexpr int32_t to_rpm() const {
        return static_cast<int32_t>(bits);
    }

    constexpr Self& from_rpm(int32_t rpm) {
        bits = static_cast<int24_t>(rpm);
        return *this;
    }
};

struct [[nodiscard]] TemperatureCode final{
    using Self = TemperatureCode;
    
    int16_t bits;  // deci-degree (tenths of a degree) Celsius

    [[nodiscard]] constexpr float to_celsius() const {
        return static_cast<float>(bits) * 0.1f;
    }

    constexpr Self& from_celsius(float celsius) {
        bits = static_cast<int16_t>(celsius * 10.0f);
        return *this;
    }
};

struct [[nodiscard]] GpsCoordinateCode final{
    using Self = GpsCoordinateCode;
    
    int32_t bits;  // degree / 10`000`000

    [[nodiscard]] constexpr double to_degrees() const {
        return static_cast<double>(bits) / 10000000.0;
    }

    constexpr Self& from_degrees(double degrees) {
        bits = static_cast<int32_t>(degrees * 10000000.0);
        return *this;
    }
};

struct [[nodiscard]] GpsHeadingCode final{
    using Self = GpsHeadingCode;
    
    uint16_t bits;  // degree / 100

    [[nodiscard]] constexpr float to_degrees() const {
        return static_cast<float>(bits) * 0.01f;
    }

    constexpr Self& from_degrees(float degrees) {
        bits = static_cast<uint16_t>(degrees * 100.0f);
        return *this;
    }
};

struct [[nodiscard]] GpsGroundSpeedCode final{
    using Self = GpsGroundSpeedCode;
    
    uint16_t bits;  // km/h / 100

    [[nodiscard]] constexpr float to_kmh() const {
        return static_cast<float>(bits) * 0.01f;
    }

    [[nodiscard]] constexpr float to_mph() const {
        return to_kmh() * 0.621371f;
    }

    constexpr Self& from_kmh(float kmh) {
        bits = static_cast<uint16_t>(kmh * 100.0f);
        return *this;
    }

    constexpr Self& from_mph(float mph) {
        bits = static_cast<uint16_t>(mph * 100.0f / 0.621371f);
        return *this;
    }
};

struct [[nodiscard]] AirspeedCode final{
    using Self = AirspeedCode;
    
    uint16_t bits;  // Airspeed in 0.1 * km/h (hectometers/h)

    [[nodiscard]] constexpr float to_kmh() const {
        return static_cast<float>(bits) * 0.1f;
    }

    [[nodiscard]] constexpr float to_mph() const {
        return to_kmh() * 0.621371f;
    }

    constexpr Self& from_kmh(float kmh) {
        bits = static_cast<uint16_t>(kmh * 10.0f);
        return *this;
    }

    constexpr Self& from_mph(float mph) {
        bits = static_cast<uint16_t>(mph * 10.0f / 0.621371f);
        return *this;
    }
};

struct [[nodiscard]] AttitudeAngleCode final{
    using Self = AttitudeAngleCode;
    
    int16_t bits;  // LSB = 100 µrad

    [[nodiscard]] constexpr float to_radians() const {
        return static_cast<float>(bits) * 0.0001f;  // Convert from 100µrad to radians
    }

    [[nodiscard]] constexpr float to_degrees() const {
        return to_radians() * 57.2958f;  // radians to degrees
    }

    constexpr Self& from_radians(float radians) {
        bits = static_cast<int16_t>(radians * 10000.0f);
        return *this;
    }

    constexpr Self& from_degrees(float degrees) {
        bits = static_cast<int16_t>(degrees * 10000.0f / 57.2958f);
        return *this;
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

    constexpr Self& from_dbm(int8_t dbm) {
        bits = static_cast<uint8_t>(-dbm);
        return *this;
    }
};

struct [[nodiscard]] LinkQualityCode final{
    using Self = LinkQualityCode;
    
    uint8_t bits;  // Link quality (%)

    [[nodiscard]] constexpr uint8_t to_percent() const {
        return bits;
    }

    constexpr Self& from_percent(uint8_t percent) {
        bits = (percent > 100) ? 100 : percent;
        return *this;
    }
};

struct [[nodiscard]] SnrCode final{
    using Self = SnrCode;
    
    int8_t bits;  // SNR (dB)

    [[nodiscard]] constexpr int8_t to_db() const {
        return bits;
    }

    constexpr Self& from_db(int8_t db) {
        bits = db;
        return *this;
    }
};

struct [[nodiscard]] RfPowerDbmCode final{
    using Self = RfPowerDbmCode;
    
    uint8_t bits;  // rf power in dBm

    [[nodiscard]] constexpr int8_t to_dbm() const {
        return static_cast<int8_t>(bits);
    }

    constexpr Self& from_dbm(int8_t dbm) {
        bits = static_cast<uint8_t>(dbm);
        return *this;
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

    [[nodiscard]] constexpr float to_fps() const {
        return static_cast<float>(bits) * 10.0f;
    }

    constexpr Self& from_fps(float fps) {
        bits = static_cast<uint8_t>(fps / 10.0f);
        return *this;
    }
};

struct [[nodiscard]] PressureCode final{
    using Self = PressureCode;
    
    int32_t bits;  // Pascals

    [[nodiscard]] constexpr float to_pa() const {
        return static_cast<float>(bits);
    }

    [[nodiscard]] constexpr float to_hpa() const {
        return static_cast<float>(bits) / 100.0f;
    }

    constexpr Self& from_pa(float pa) {
        bits = static_cast<int32_t>(pa);
        return *this;
    }

    constexpr Self& from_hpa(float hpa) {
        bits = static_cast<int32_t>(hpa * 100.0f);
        return *this;
    }
};

struct [[nodiscard]] TemperatureCentidegreeCode final{
    using Self = TemperatureCentidegreeCode;
    
    int32_t bits;  // centidegrees

    [[nodiscard]] constexpr float to_celsius() const {
        return static_cast<float>(bits) * 0.01f;
    }

    constexpr Self& from_celsius(float celsius) {
        bits = static_cast<int32_t>(celsius * 100.0f);
        return *this;
    }
};

struct [[nodiscard]] PassthroughTelemetryPacket final{
    uint16_t appid;
    uint32_t data;
};

}