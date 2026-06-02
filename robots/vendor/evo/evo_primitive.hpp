#pragma once


#include <cstdint>
#include "robots/vendor/mit/mit_primitive.hpp"


// reference:
// https://github.com/Luo1imasi/motors/blob/master/src/drivers/evo/evo_motor_driver.cpp

namespace ymd::robots::evo{

#if 0
enum class [[nodiscard]] Error: uint8_t {
    None = 0x00,
    OverVoltage = 0x01,
    UnderVoltage = 0x02,
    OverCurrent = 0x03,
    MosOverTemp = 0x09,
    CoilOverTemp = 0x0A,
    EncoderError = 0x0B,
    Overload = 0x0F,
    CommLost = 0x10,
    OverSpeed = 0x0E,       // over-speed (new CAN-FD spec bit 4)
    PosOverLimit = 0x0D,   // position over-limit (new CAN-FD spec bit 12)
    Unknown = 0xFF
};
#endif

enum class [[nodiscard]] RegAddr : uint8_t {
    // --- System & Core Information ---
    FirmwareVer    = 10,  // Firmware Version (Int32)
    MotorId        = 36,  // Motor CAN ID (Int32)
    Npp             = 44,  // Number of Pole Pairs (Int32)
    GearRatio      = 45,  // Gear Reduction Ratio (Float32)

    // --- Current Loop Control (FOC) ---
    CurKpD        = 12,  // D-axis Current Loop Kp (Float32)
    CurKiD        = 13,  // D-axis Current Loop Ki (Float32)
    CurKpQ        = 14,  // Q-axis Current Loop Kp (Float32)
    CurKiQ        = 15,  // Q-axis Current Loop Ki (Float32)

    // --- Control Deadzones ---
    DeadzoneCur    = 16,  // Current Deadzone (Float32)
    DeadzoneVel    = 17,  // Velocity Deadzone (Float32)
    DeadzonePos    = 18,  // Position Deadzone (Float32)

    // --- Motion Dynamics & Limits ---
    TorqueLimit    = 21,  // Torque Protection Limit (Float32)
    ZeroOffset     = 23,  // Mechanical Zero Offset (Float32)
    Acceleration    = 42,  // Servo Mode Acceleration (Float32)
    TorqueSlope    = 43,  // Torque Rise Rate / Slope (Float32)

    // --- CAN Communication Range Config (CRITICAL for range_map alignment) ---
    CanThetaMin   = 24,  // Position Mapping Minimum (rad)
    CanThetaMax   = 25,  // Position Mapping Maximum (rad)
    CanVelMin     = 26,  // Velocity Mapping Minimum (rad/s)
    CanVelMax     = 27,  // Velocity Mapping Maximum (rad/s)
    CanKpMax      = 29,  // Kp Mapping Maximum Value
    CanKdMax      = 31,  // Kd Mapping Maximum Value
    CanTorqueMin  = 34,  // Torque Mapping Minimum (Nm)
    CanTorqueMax  = 35,  // Torque Mapping Maximum (Nm)
    CanTimeout     = 37,  // CAN Communication Timeout Threshold (Int32/ms)

    // --- Safety & Protection Thresholds ---
    OvLock         = 49,  // Over-Voltage (Bus) Lockout (Float32)
    UvLock         = 50,  // Under-Voltage (Bus) Lockout (Float32)
    OcLock         = 51,  // Over-Current (Phase) Lockout (Float32)
    OtLock         = 52,  // Over-Temperature (Board) Lockout (Float32)
    StuckTime      = 55,  // Stall/Stuck Protection Time (Float32)
    ProtectSwitch  = 56  // Protection Feature Switch Bits (Int32)
};

enum class [[nodiscard]] Command:uint8_t {
    Enable = 0xFC,      ///< Enable motor mode and reset motor state
    Disable = 0xFD,      ///< Reset motor and clear errors
    SetZero = 0xFE,        ///< Set current position as zero point
    StartFlash = 0x67,      ///< Start flash operation
    EndFlash = 0x76,      ///< End flash operation
    ReadFlash = 0x04,      ///< Read flash operation
    WriteFlash = 0x15,      ///< Write flash operation
    SaveFlash = 0x00      ///< Save flash operation
};

enum class [[nodiscard]] FlashParamId:uint8_t {
    QMax = 0x00,         ///< Maximum position limit
    QMin = 0x01,         ///< Minimum position limit
    DqMax = 0x02,        ///< Maximum velocity limit
    DqMin = 0x03,        ///< Minimum velocity limit
    TauMax = 0x04,       ///< Maximum torque/current limit
    TauMin = 0x05,       ///< Minimum torque/current limit
    OkpMax = 0x06,       ///< Maximum outer Kp
    OkpMin = 0x07,       ///< Minimum outer Kp
    OkdMax = 0x08,       ///< Maximum outer Kd
    OkdMin = 0x09,       ///< Minimum outer Kd
    IkpMax = 0x0A,       ///< Maximum inner Kp
    IkpMin = 0x0B,       ///< Minimum inner Kp
    IkiMax = 0x0C,       ///< Maximum inner Ki
    IkiMin = 0x0D,       ///< Minimum inner Ki
    CurMax = 0x0E,       ///< Maximum current
    CurMin = 0x0F        ///< Minimum current
};


enum class [[nodiscard]] Package:uint8_t { 
    EVO431040,
    EVO811825,
    EVO811832
};

struct NodeId{
    uint8_t count;
};

struct MitPosParams{
    std::array<uint8_t, 8> bytes;
};

struct MitSpdParams{
    std::array<uint8_t, 8> bytes;
};


}