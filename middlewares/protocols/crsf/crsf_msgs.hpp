#pragma once

#include "crsf_primitive.hpp"
#include "core/int/uint24_t.hpp"
#include "core/math/realmath.hpp"

namespace ymd::crsf{
using math::int24_t, math::uint24_t;
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

    [[nodiscard]] constexpr int32_t to_m() const {
        if(bits & 0x8000)
            return static_cast<int32_t>((bits & 0x7fff));
        else
            return (static_cast<int32_t>(bits) - 10000) / 10;
    }

    constexpr Self& from_dm(const int32_t altitude_dm){
        bits = dm_to_bits(altitude_dm);
        return *this;
    }

    constexpr Self& from_m(const int32_t altitude_m){
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

struct [[nodiscard]] GpsCoordinate final{
    using Self = GpsCoordinate;
    
    int32_t bits;  // degree / 10`000`000

    [[nodiscard]] constexpr double to_degrees() const {
        return static_cast<double>(bits) / 10000000.0;
    }

    constexpr Self& from_degrees(double degrees) {
        bits = static_cast<int32_t>(degrees * 10000000.0);
        return *this;
    }
};

struct [[nodiscard]] GpsHeading final{
    using Self = GpsHeading;
    
    uint16_t bits;  // degree / 100

    [[nodiscard]] constexpr float to_degrees() const {
        return static_cast<float>(bits) * 0.01f;
    }

    constexpr Self& from_degrees(float degrees) {
        bits = static_cast<uint16_t>(degrees * 100.0f);
        return *this;
    }
};

struct [[nodiscard]] GpsGroundSpeed final{
    using Self = GpsGroundSpeed;
    
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

struct [[nodiscard]] AttitudeAngle final{
    using Self = AttitudeAngle;
    
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

struct [[nodiscard]] RfPowerCode final{
    using Self = RfPowerCode;
    
    uint8_t bits;  // rf power in dBm

    [[nodiscard]] constexpr int8_t to_dbm() const {
        return static_cast<int8_t>(bits);
    }

    constexpr Self& from_dbm(int8_t dbm) {
        bits = static_cast<uint8_t>(dbm);
        return *this;
    }
};

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

struct [[nodiscard]] TemperatureCentidegree final{
    using Self = TemperatureCentidegree;
    
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

namespace ymd::crsf::msgs{
using math::int24_t, math::uint24_t;

// 0x02
struct [[nodiscard]] Gps final{
    GpsCoordinate latitude;       // degree / 10`000`000
    GpsCoordinate longitude;      // degree / 10`000`000
    GpsGroundSpeed groundspeed;   // km/h / 100
    GpsHeading heading;           // degree / 100
    uint16_t altitude;            // meter - 1000m offset
    uint8_t satellites;           // # of sats in view
};

// 0x03
struct [[nodiscard]] GpsTime final{
    int16_t year;
    uint8_t month;
    uint8_t day;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
    uint16_t millisecond;
};

// 0x06
struct [[nodiscard]] GpsExtended final{
    uint8_t fix_type;       // Current GPS fix quality
    int16_t n_speed;        // Northward (north = positive) Speed [cm/sec]
    int16_t e_speed;        // Eastward (east = positive) Speed [cm/sec]
    int16_t v_speed;        // Vertical (up = positive) Speed [cm/sec]
    int16_t h_speed_acc;    // Horizontal Speed accuracy cm/sec
    int16_t track_acc;      // Heading accuracy in degrees Coded with 1e-1 degrees times 10)
    int16_t alt_ellipsoid;  // Meters Height above GPS Ellipsoid (not MSL)
    int16_t h_acc;          // horizontal accuracy in cm
    int16_t v_acc;          // vertical accuracy in cm
    uint8_t reserved;
    uint8_t hDOP;           // Horizontal dilution of precision,Dimensionless in nits of.1.
    uint8_t vDOP;           // vertical dilution of precision, Dimensionless in nits of .1.
};

// 0x07
struct [[nodiscard]] VariometerSensor final{
   int16_t v_speed;        // Vertical speed cm/s
};

// 0x08
struct [[nodiscard]] BatterySensor final{
    VoltageCode voltage;        // Voltage (LSB = 10 µV)
    CurrentCode current;        // Current (LSB = 10 µA)
    uint24_t capacity_used;      // Capacity used (mAh)
    uint8_t remaining;           // Battery remaining (percent)
};

// 0x09
struct [[nodiscard]] BarometricAltitude final{
    uint16_t altitude_packed;       // Altitude above start (calibration) point
                                    // See description below.
    int8_t   vertical_speed_packed; // vertical speed. See description below.
};

//0x0A
struct [[nodiscard]] Airspeed final{
    AirspeedCode speed;             // Airspeed in 0.1 * km/h (hectometers/h)
};

// 0x0B
struct [[nodiscard]] HeartBeat final{
    int16_t origin_address;             // Origin Device address
};

//0x0c
struct [[nodiscard]] Rpm final{
    uint8_t     rpm_source_id;  // Identifies the source of the RPM data (e.g., 0 = Motor 1, 1 = Motor 2, etc.)
    RpmCode    rpm_value[];    // 1 - 19 RPM values with negative ones representing the motor spinning in reverse
};

//0x0d
struct [[nodiscard]] Temperature final{
    uint8_t         temp_source_id;       // Identifies the source of the temperature data (e.g., 0 = FC including all ESCs, 1 = Ambient, etc.)
    TemperatureCode temperature[];        // up to 20 temperature values in deci-degree (tenths of a degree) Celsius (e.g., 250 = 25.0°C, -50 = -5.0°C)
};

// 0x0E
struct [[nodiscard]] Voltages final{
    uint8_t     voltage_source_id;  // source of the voltages
    uint16_t    voltage_values[];   // Up to 29 voltages in millivolts (e.g. 3.850V = 3850)
};

// 0x10
struct [[nodiscard]] VtxTelemetry final{
    uint8_t     origin_address;
    uint8_t     power_dBm;          // VTX power in dBm
    uint16_t    frequency_MHz;      // VTX frequency in MHz
    uint8_t     pit_mode:1;         // 0=Off, 1=On
    uint8_t     pitmode_control:2;  // 0=Off, 1=On, 2=Switch, 3=Failsafe
    uint8_t     pitmode_switch:4;   // 0=Ch5, 1=Ch5 Inv, … , 15=Ch12 Inv
};

// 0x11
struct [[nodiscard]] Barometer final{
    PressureCode pressure_pa;        // Pascals
    TemperatureCentidegree baro_temp; // centidegrees
};

// 0x12
struct [[nodiscard]] Magnetometer final{
    int16_t     field_x;            // milligauss * 3
    int16_t     field_y;            // milligauss * 3
    int16_t     field_z;            // milligauss * 3
};

// 0x13
struct [[nodiscard]] AccelGyro final{
    uint32_t sample_time;       // Timestamp of the sample in us
    int16_t gyro_x;             // LSB = INT16_MAX/2000 DPS
    int16_t gyro_y;             // LSB = INT16_MAX/2000 DPS
    int16_t gyro_z;             // LSB = INT16_MAX/2000 DPS
    int16_t acc_x;              // LSB = INT16_MAX/16 G
    int16_t acc_y;              // LSB = INT16_MAX/16 G
    int16_t acc_z;              // LSB = INT16_MAX/16 G
    int16_t gyro_temp;          // centidegrees
};

// 0x14
struct [[nodiscard]] LinkStatistics final{
    RssiCode     up_rssi_ant1;       // Uplink RSSI Antenna 1 (dBm * -1)
    RssiCode     up_rssi_ant2;       // Uplink RSSI Antenna 2 (dBm * -1)
    LinkQualityCode up_link_quality; // Uplink Package success rate / Link quality (%)
    SnrCode      up_snr;             // Uplink SNR (dB)
    uint8_t       active_antenna;     // number of currently best antenna
    uint8_t       rf_profile;         // enum {4fps = 0 , 50fps, 150fps}
    uint8_t       up_rf_power;        // enum {0mW = 0, 10mW, 25mW, 100mW,
                                      // 500mW, 1000mW, 2000mW, 250mW, 50mW}
    RssiCode     down_rssi;          // Downlink RSSI (dBm * -1)
    LinkQualityCode down_link_quality; // Downlink Package success rate / Link quality (%)
    SnrCode      down_snr;           // Downlink SNR (dB)
};

// 0x16
struct [[nodiscard]] RcChannelsPacked final{
    struct{
        int channel_01: 11;
        int channel_02: 11;
        int channel_03: 11;
        int channel_04: 11;
        int channel_05: 11;
        int channel_06: 11;
        int channel_07: 11;
        int channel_08: 11;
        int channel_09: 11;
        int channel_10: 11;
        int channel_11: 11;
        int channel_12: 11;
        int channel_13: 11;
        int channel_14: 11;
        int channel_15: 11;
        int channel_16: 11;
    } channels;
};

// 0x1C
struct [[nodiscard]] LinkStatisticsRx final{
    RssiCode      rssi_db;        // RSSI (dBm * -1)
    uint8_t        rssi_percent;   // RSSI in percent
    LinkQualityCode link_quality; // Package success rate / Link quality (%)
    SnrCode       snr;            // SNR (dB)
    RfPowerCode   rf_power_db;    // rf power in dBm
};

// 0x1D
struct [[nodiscard]] LinkStatisticsTx final{
    RssiCode      rssi_db;        // RSSI (dBm * -1)
    uint8_t        rssi_percent;   // RSSI in percent
    LinkQualityCode link_quality; // Package success rate / Link quality (%)
    SnrCode       snr;            // SNR (dB)
    RfPowerCode   rf_power_db;    // rf power in dBm
    FpsCode       fps;            // rf frames per second (fps / 10)
};

// 0x1E
struct [[nodiscard]] Attitude final{
    AttitudeAngle pitch;  // Pitch angle (LSB = 100 µrad)
    AttitudeAngle roll;   // Roll angle  (LSB = 100 µrad)
    AttitudeAngle yaw;    // Yaw angle   (LSB = 100 µrad)
};

// 0x1F
struct [[nodiscard]] MavlinkFc final{
    int16_t     airspeed;
    uint8_t     base_mode;      // vehicle mode flags, defined in MAV_MODE_FLAG enum
    uint32_t    custom_mode;    // autopilot-specific flags
    uint8_t     autopilot_type; // FC type; defined in MAV_AUTOPILOT enum
    uint8_t     firmware_type;  // vehicle type; defined in MAV_TYPE enum
};

// 0x21
struct [[nodiscard]] FlightMode final{
    char flight_mode[];  // Null-terminated string
};

// 0x22
struct [[nodiscard]] EspNowMessages final{
    uint8_t VAL1;           // Used for Seat Position of the Pilot
    uint8_t VAL2;           // Used for the Current Pilots Lap
    char    VAL3[15];       // 15 characters for the lap time current/split
    char    VAL4[15];       // 15 characters for the lap time current/split
    char    free_text[20];  // Free text of 20 character at the bottom of the screen
};

// 0x28
struct [[nodiscard]] ParameterPingDevices final{
    // The frame has no payload
};

// 0x29
struct [[nodiscard]] ParameterDeviceInfo final{
    char        device_name[];        // Null-terminated string
    uint32_t    serial_number;
    uint32_t    hardware_id;
    uint32_t    firmware_id;
    uint8_t     parameters_total;   // Total amount of parameters
    uint8_t     parameter_version_number;
};

// 0x2B
struct [[nodiscard]] ParameterSettingsEntry final{
    uint8_t         parameter_number;           // starting from 0
    uint8_t         parameter_chunks_remaining; // Chunks remaining count
    uint8_t         data_type_payload_chunk[];  // Part of Parameter settings payload, up to 56 bytes
};

// 0x2C
struct [[nodiscard]] ParameterSettingsRead final{
    uint8_t parameter_number;
    uint8_t parameter_chunk_number; // Chunk number to request, starts with 0
};

// 0x2D
struct [[nodiscard]] ParameterValueWrite final{
    uint8_t parameter_number;
    uint8_t data[];  // New value payload; size depends on data type
};

// 0x32 Command frame
struct [[nodiscard]] DirectCommand final{
    uint8_t     command_id;
    uint8_t     payload[];        // depending on Command ID
};

// 0x3A.0x10 Timing Correction
struct [[nodiscard]] TimingCorrection final{
    uint32_t    update_interval;    // LSB = 100ns
    int32_t     offset;             // LSB = 100ns, positive values = data came too early,
                                    // negative = late.
};

// 0x34 Logging
struct [[nodiscard]] Logging final{
    uint16_t logtype;
    uint32_t timestamp;
    uint32_t para1;
    // ...
    uint32_t paraN[];
};

// 0x7A MSP Request
struct [[nodiscard]] MspRequest final{
    uint8_t status_byte;
    uint8_t msp_payload[];  // MSP frame body without header and CRC
};

// 0x7B MSP Response
struct [[nodiscard]] MspResponse final{
    uint8_t status_byte;
    uint8_t msp_payload[];  // MSP frame body without header and CRC
};

// 0x80 ArduPilot Passthrough Frame (Single packet)
struct [[nodiscard]] ArduPilotPassthroughSingle final{
    uint8_t sub_type = 0xF0;  // Always 0xF0 for single packet
    uint16_t appid;
    uint32_t data;
};

// 0x80 ArduPilot Passthrough Frame (Multi-packet)
struct [[nodiscard]] ArduPilotPassthroughMulti final{
    uint8_t sub_type = 0xF2;  // Always 0xF2 for multi-packet
    uint8_t size;
    PassthroughTelemetryPacket packets[9];
};

// 0x80 ArduPilot Passthrough Frame (Status text)
struct [[nodiscard]] ArduPilotPassthroughStatus final{
    uint8_t sub_type = 0xF1;  // Always 0xF1 for status text
    uint8_t severity;
    char text[50];  // (Null-terminated string)
};

} // namespace ymd::crsf::msgs