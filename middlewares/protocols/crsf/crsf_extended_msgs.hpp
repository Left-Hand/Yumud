

#include "crsf_primitive.hpp"

#include "compatible/crsf_mav_compatible_primitive.hpp"


namespace ymd::crsf{

// 0x28
struct [[nodiscard]] ParameterPingDevices final{
    // The frame has no payload
};

// 0x29
struct [[nodiscard]] ParameterDeviceInfo final{
    CharsNullTerminated        device_name;        // Null-terminated string
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
    std::span<const uint8_t>         data_type_payload_chunk;  // Part of Parameter settings payload, up to 56 bytes
};

// 0x2C
struct [[nodiscard]] ParameterSettingsRead final{
    uint8_t parameter_number;
    uint8_t parameter_chunk_number; // Chunk number to request, starts with 0
};

// 0x2D
struct [[nodiscard]] ParameterValueWrite final{
    uint8_t parameter_number;
    std::span<const uint8_t> data;  // New value payload; size depends on data type
};

// 0x32 Command frame
struct [[nodiscard]] DirectCommand final{
    uint8_t     command_id;
    std::span<const uint8_t>     payload;        // depending on Command ID
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
    std::span<uint32_t> params;
};

// 0x7A MSP Request
struct [[nodiscard]] MspRequest final{
    uint8_t status_byte;
    std::span<const uint8_t> msp_payload;  // MSP frame body without header and CRC
};

// 0x7B MSP Response
struct [[nodiscard]] MspResponse final{
    uint8_t status_byte;
    std::span<const uint8_t> msp_payload;  // MSP frame body without header and CRC
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
    CharsSlice<50> text;  // (Null-terminated string)
};


}