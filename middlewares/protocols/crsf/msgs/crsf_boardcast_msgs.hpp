#pragma once

#include "../crsf_primitive.hpp"
#include "../crsf_packed_code.hpp"

#include "../compatible/crsf_mav_compatible_primitive.hpp"



namespace ymd::crsf::msgs{

// 0x02
struct [[nodiscard]] Gps final{
    static constexpr FrameType FRAME_TYPE = FrameType::from_bits(0x02);
    GpsCoordinateCode latitude;       // degree / 10`000`000
    GpsCoordinateCode longitude;      // degree / 10`000`000
    GpsGroundSpeedCode groundspeed;   // km/h / 100
    GpsHeadingCode heading;           // degree / 100
    uint16_t altitude;            // meter - 1000m offset
    uint8_t satellites;           // # of sats in view
};

// 0x03
struct [[nodiscard]] GpsTime final{
    static constexpr FrameType FRAME_TYPE = FrameType::from_bits(0x03);
    int16_t year;
    uint8_t month;
    uint8_t day;
    uint8_t hour;
    uint8_t minute;
    uint8_t seconds;
    uint16_t millisecond;
};

// 0x06
struct [[nodiscard]] GpsExtended final{
    static constexpr FrameType FRAME_TYPE = FrameType::from_bits(0x06);
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
    uint8_t h_dop;           // Horizontal dilution of precision,Dimensionless in nits of.1.
    uint8_t v_dop;           // vertical dilution of precision, Dimensionless in nits of .1.
};

// 0x07
struct [[nodiscard]] VariometerSensor final{
    static constexpr FrameType FRAME_TYPE = FrameType::from_bits(0x07);
   int16_t v_speed;        // Vertical speed cm/s
};

// 0x08
struct [[nodiscard]] BatterySensor final{
    static constexpr FrameType FRAME_TYPE = FrameType::from_bits(0x08);
    VoltageCode voltage;        // Voltage (LSB = 10 µV)
    CurrentCode current;        // Current (LSB = 10 µA)
    uint24_t capacity_used;      // Capacity used (mAh)
    uint8_t remaining;           // Battery remaining (percent)
};

// 0x09
struct [[nodiscard]] BarometricAltitude final{
    static constexpr FrameType FRAME_TYPE = FrameType::from_bits(0x09);
    AltitudeCode altitude_code;       // Altitude above start (calibration) point
                                    // See description below.
    VerticalSpeedCode   vertical_speed_code; // vertical speed. See description below.
};

//0x0A
struct [[nodiscard]] Airspeed final{
    static constexpr FrameType FRAME_TYPE = FrameType::from_bits(0x0a);
    AirspeedCode speed;             // Airspeed in 0.1 * km/h (hectometers/h)
};

// 0x0B
struct [[nodiscard]] HeartBeat final{
    static constexpr FrameType FRAME_TYPE = FrameType::from_bits(0x0b);
    int16_t origin_address;             // Origin Device address
};

//0x0c
struct [[nodiscard]] Rpm final{
    static constexpr FrameType FRAME_TYPE = FrameType::from_bits(0x0c);
    uint8_t     rpm_source_id;  // Identifies the source of the RPM data (e.g., 0 = Motor 1, 1 = Motor 2, etc.)
    std::span<const RpmCode>    rpm_value;    // 1 - 19 RPM values with negative ones representing the motor spinning in reverse
};

//0x0d
struct [[nodiscard]] Temperature final{
    static constexpr FrameType FRAME_TYPE = FrameType::from_bits(0x0d);
    uint8_t         temp_source_id;       // Identifies the source of the temperature data (e.g., 0 = FC including all ESCs, 1 = Ambient, etc.)
    std::span<const TemperatureCode> temperature;        // up to 20 temperature values in deci-degree (tenths of a degree) Celsius (e.g., 250 = 25.0°C, -50 = -5.0°C)
};

// 0x0E
struct [[nodiscard]] Voltages final{
    static constexpr FrameType FRAME_TYPE = FrameType::from_bits(0x0e);
    uint8_t     voltage_source_id;  // source of the voltages
    std::span<const uint16_t>    voltage_values;   // Up to 29 voltages in millivolts (e.g. 3.850V = 3850)
};

// 0x10
struct [[nodiscard]] VtxTelemetry final{
    static constexpr FrameType FRAME_TYPE = FrameType::from_bits(0x10);
    uint8_t     origin_address;
    uint8_t     power_dBm;          // VTX power in dBm
    uint16_t    frequency_MHz;      // VTX frequency in MHz
    uint8_t     pit_mode:1;         // 0=Off, 1=On
    uint8_t     pitmode_control:2;  // 0=Off, 1=On, 2=Switch, 3=Failsafe
    uint8_t     pitmode_switch:4;   // 0=Ch5, 1=Ch5 Inv, … , 15=Ch12 Inv
};

// 0x11
struct [[nodiscard]] Barometer final{
    static constexpr FrameType FRAME_TYPE = FrameType::from_bits(0x11);
    PressureCode pressure_pa;        // Pascals
    TemperatureCentidegreeCode baro_temp; // centidegrees
};

// 0x12
struct [[nodiscard]] Magnetometer final{
    static constexpr FrameType FRAME_TYPE = FrameType::from_bits(0x12);
    int16_t     field_x;            // milligauss * 3
    int16_t     field_y;            // milligauss * 3
    int16_t     field_z;            // milligauss * 3
};

// 0x13
// NEU机身坐标系下的原始加速度计和陀螺仪数据，样本是在采样间隔内平均的原始数据

// 加速度计：+X轴 = 前进 +Y轴 = 右 +Z轴 = 上 陀螺仪： +X轴 = 左滚 +Y轴 = 俯仰向上 +Z轴 = 顺时针偏航
struct [[nodiscard]] AccelGyro final{
    static constexpr FrameType FRAME_TYPE = FrameType::from_bits(0x13);
    uint32_t sample_time;       // Timestamp of the sample in us
    int16_t gyr_x;             // LSB = INT16_MAX/2000 DPS
    int16_t gyr_y;             // LSB = INT16_MAX/2000 DPS
    int16_t gyr_z;             // LSB = INT16_MAX/2000 DPS
    int16_t acc_x;              // LSB = INT16_MAX/16 G
    int16_t acc_y;              // LSB = INT16_MAX/16 G
    int16_t acc_z;              // LSB = INT16_MAX/16 G
    int16_t gyr_temp;          // centidegrees
};

// 0x14
// 上行链路是从地面到无人机的连接，下行链路则是相反方向
struct [[nodiscard]] LinkStatistics final{
    static constexpr FrameType FRAME_TYPE = FrameType::from_bits(0x14);
    RssiCode            up_rssi_ant1;       // Uplink RSSI Antenna 1 (dBm * -1)
    RssiCode            up_rssi_ant2;       // Uplink RSSI Antenna 2 (dBm * -1)
    LinkQualityCode     up_link_quality; // Uplink Package success rate / Link quality (%)
    SnrCode             up_snr;             // Uplink SNR (dB)
    uint8_t             active_antenna;     // number of currently best antenna
    RfFps             rf_profile;         // enum {4fps = 0 , 50fps, 150fps}
    RfPower             up_rf_power;        // enum {0mW = 0, 10mW, 25mW, 100mW,
                                      // 500mW, 1000mW, 2000mW, 250mW, 50mW}
    RssiCode            down_rssi;          // Downlink RSSI (dBm * -1)
    LinkQualityCode     down_link_quality; // Downlink Package success rate / Link quality (%)
    SnrCode             down_snr;           // Downlink SNR (dB)
};

static_assert(sizeof(RssiCode) == 1);
static_assert(sizeof(LinkQualityCode) == 1);
static_assert(sizeof(SnrCode) == 1);

// 0x16
// 16个通道打包成22字节。如果发生故障保护，此帧将不再发送
// （当故障保护类型设置为"切断"时）。建议在启动FC故障保护程序之前等待1秒。
struct [[nodiscard]] RcChannelsPacked final{
    static constexpr FrameType FRAME_TYPE = FrameType::from_bits(0x16);

    U11X16Owned channels;
};


// 0x17 RC通道子集打包
// [!警告] 不推荐实现此帧。正在修订中。

// 0x18 RC通道打包11位（未使用）
// 与0x16相同，但使用与0x17相同的转换样式

// 0x19 - 0x1B 保留Crossfire

// 0x1C
struct [[nodiscard]] LinkStatisticsRx final{
    static constexpr FrameType FRAME_TYPE = FrameType::from_bits(0x1c);
    RssiCode            rssi_dbm;        // RSSI (dBm * -1)
    uint8_t             rssi_percent;   // RSSI in percent
    LinkQualityCode     link_quality; // Package success rate / Link quality (%)
    SnrCode             snr;            // SNR (dB)
    RfPowerDbmCode      rf_power_dbm;    // rf power in dBm
};

// 0x1D
struct [[nodiscard]] LinkStatisticsTx final{
    static constexpr FrameType FRAME_TYPE = FrameType::from_bits(0x1d);
    RssiCode            rssi_dbm;        // RSSI (dBm * -1)
    uint8_t             rssi_percent;   // RSSI in percent
    LinkQualityCode     link_quality; // Package success rate / Link quality (%)
    SnrCode             snr;            // SNR (dB)
    RfPowerDbmCode      rf_power_dbm;    // rf power in dBm
    FpsCode             fps;            // rf frames per second (fps / 10)
};

// 0x1E
struct [[nodiscard]] Attitude final{
    static constexpr FrameType FRAME_TYPE = FrameType::from_bits(0x1e);
    AttitudeAngleCode pitch;  // Pitch angle (LSB = 100 µrad)
    AttitudeAngleCode roll;   // Roll angle  (LSB = 100 µrad)
    AttitudeAngleCode yaw;    // Yaw angle   (LSB = 100 µrad)
};

// 0x1F
struct [[nodiscard]] MavlinkFc final{
    static constexpr FrameType FRAME_TYPE = FrameType::from_bits(0x1f);
    int16_t                         airspeed;
    mavlink::MavModeFlagBitfields   base_mode;      // vehicle mode flags, defined in MAV_MODE_FLAG enum
    uint32_t                        custom_mode;    // autopilot-specific flags
    mavlink::MavAutopilot           autopilot_type; // FC type; defined in MAV_AUTOPILOT enum
    mavlink::MavComponentType       firmware_type;  // vehicle type; defined in MAV_TYPE enum
};

// 0x21
struct [[nodiscard]] FlightMode final{
    static constexpr FrameType FRAME_TYPE = FrameType::from_bits(0x21);
    ustr<MAX_STR_LENGTH> flight_mode;  // Null-terminated string
};

// 0x22
struct [[nodiscard]] EspNowMessages final{
    static constexpr FrameType FRAME_TYPE = FrameType::from_bits(0x22);
    uint8_t arg1;           // Used for Seat Position of the Pilot
    uint8_t arg2;           // Used for the Current Pilots Lap
    CharsSlice<15>    arg3;       // 15 characters for the lap time current/split
    CharsSlice<15>    arg4;       // 15 characters for the lap time current/split
    CharsSlice<20>    free_text;  // Free text of 20 character at the bottom of the screen
};

} // namespace ymd::crsf::msgs