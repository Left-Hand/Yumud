

#include "../crsf_primitive.hpp"
#include "../crsf_packed_code.hpp"
#include "../ser/crsf_ser.hpp"

#include "../compatible/crsf_mav_compatible_primitive.hpp"


namespace ymd::crsf::msgs{

// 0x28
struct [[nodiscard]] ParameterPingDevices final{
    static constexpr FrameType FRAME_TYPE = FrameType{0x28};
    // The frame has no payload
};

// 0x29
struct [[nodiscard]] ParameterDeviceInfo final{
    static constexpr FrameType FRAME_TYPE = FrameType{0x29};
    ustr<MAX_STR_LENGTH>        device_name;        // Null-terminated string
    uint32_t    serial_number;
    uint32_t    hardware_id;
    uint32_t    firmware_id;
    uint8_t     parameters_total;   // Total amount of parameters
    uint8_t     parameter_version_number;

    // template<typename Receiver>
    constexpr Result<void, SerError> sink_to(SerializeReceiver & receiver) const {
        if(const auto res = receiver.recv_zero_terminated_uchars(device_name.uchars());  
            res.is_err()) return Err(res.unwrap_err());
        if(const auto res = receiver.recv_be_int<uint32_t>(serial_number); 
            res.is_err()) return Err(res.unwrap_err());
        if(const auto res = receiver.recv_be_int<uint32_t>(hardware_id); 
            res.is_err()) return Err(res.unwrap_err());
        if(const auto res = receiver.recv_be_int<uint32_t>(firmware_id); 
            res.is_err()) return Err(res.unwrap_err());
        if(const auto res = receiver.recv_be_int<uint8_t>(parameter_version_number); 
            res.is_err()) return Err(res.unwrap_err());
        if(const auto res = receiver.recv_be_int<uint8_t>(parameters_total); 
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }
};

// 0x2B
struct [[nodiscard]] ParameterSettingsEntry final{
    static constexpr FrameType FRAME_TYPE = FrameType{0x2B};
    uint8_t         parameter_number;           // starting from 0
    uint8_t         parameter_chunks_remaining; // Chunks remaining count
    std::span<const uint8_t>         data_type_payload_chunk;  // Part of Parameter settings payload, up to 56 bytes
};

// 0x2C
struct [[nodiscard]] ParameterSettingsRead final{
    static constexpr FrameType FRAME_TYPE = FrameType{0x2C};
    uint8_t parameter_number;
    uint8_t parameter_chunk_number; // Chunk number to request, starts with 0

    constexpr Result<void, SerError> sink_to(SerializeReceiver & receiver) const {
        if(const auto res = receiver.recv_be_int<uint8_t>(parameter_number); 
            res.is_err()) return Err(res.unwrap_err());
        if(const auto res = receiver.recv_be_int<uint8_t>(parameter_chunk_number); 
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }
};

// 0x2D
struct [[nodiscard]] ParameterValueWrite final{
    static constexpr FrameType FRAME_TYPE = FrameType{0x2D};
    uint8_t parameter_number;
    std::span<const uint8_t> data;  // New value payload; size depends on data type
};

// 0x32 Command frame
struct [[nodiscard]] DirectCommand final{
    static constexpr FrameType FRAME_TYPE = FrameType{0x32};
    uint8_t     command_id;
    std::span<const uint8_t>     payload;        // depending on Command ID
};

// 0x3A.0x10 Timing Correction
struct [[nodiscard]] TimingCorrection final{
    static constexpr FrameType FRAME_TYPE = FrameType{0x03A};
    uint32_t    update_interval;    // LSB = 100ns
    int32_t     offset;             // LSB = 100ns, positive values = data came too early,
                                    // negative = late.
};

// 0x34 Logging
struct [[nodiscard]] Logging final{
    static constexpr FrameType FRAME_TYPE = FrameType{0x34};
    uint16_t logtype;
    uint32_t timestamp;
    std::span<const uint32_t> params;
};

// 0x7A MSP Request
struct [[nodiscard]] MspRequest final{
    static constexpr FrameType FRAME_TYPE = FrameType{0x7A};
    uint8_t status_byte;
    std::span<const uint8_t> msp_payload;  // MSP frame body without header and CRC

};

// 0x7B MSP Response
struct [[nodiscard]] MspResponse final{
    static constexpr FrameType FRAME_TYPE = FrameType{0x7B};
    uint8_t status_byte;
    std::span<const uint8_t> msp_payload;  // MSP frame body without header and CRC
};

// 0x80 ArduPilot Passthrough Frame (Single packet)
struct [[nodiscard]] ArduPilotPassthroughSingle final{
    static constexpr FrameType FRAME_TYPE = FrameType{0x80};
    uint8_t sub_type = 0xF0;  // Always 0xF0 for single packet
    uint16_t appid;
    uint32_t data;
};

// 0x80 ArduPilot Passthrough Frame (Multi-packet)
struct [[nodiscard]] ArduPilotPassthroughMulti final{
    static constexpr FrameType FRAME_TYPE = FrameType{0X80};
    uint8_t sub_type;  // Always 0xF2 for multi-packet
    uint8_t size;
    std::span<const PassthroughTelemetryPacket> packets;
};

// 0x80 ArduPilot Passthrough Frame (Status text)
struct [[nodiscard]] ArduPilotPassthroughStatus final{
    static constexpr FrameType FRAME_TYPE = FrameType{0X80};
    uint8_t sub_type;  // Always 0xF1 for status text
    uint8_t severity;
    CharsSlice<50> text;  // (Null-terminated string)
};


}