#pragma once

#include "steadywin_can_simple_primitive.hpp"

namespace ymd::robots::steadywin::can_simple{

using namespace primitive;

struct [[nodiscard]] FrameDeserializer{
    enum class [[nodiscard]] Error:uint8_t{
        FrameIsNotStd,
        PayloadTooShort,
        NotImplemented
    };

private:
    struct [[nodiscard]] BytesReader{
        explicit constexpr BytesReader(std::span<const uint8_t> bytes) : 
            bytes_(bytes) {}

        [[nodiscard]] constexpr Option<int32_t> fetch_i32(){
            if(remaining().size() < 4)
                return None;
            return Some(le_bytes_to_int<int32_t>(fetch_bytes<4>()));
        }
        
        [[nodiscard]] constexpr Option<uint32_t> fetch_u32(){
            if(remaining().size() < 4)
                return None;
            return Some(le_bytes_to_int<uint32_t>(fetch_bytes<4>()));
        }


        [[nodiscard]] constexpr Option<math::fp32> fetch_f32(){
            if(remaining().size() < 4)
                return None;
            return Some(math::fp32::from_bits(le_bytes_to_int<int32_t>(fetch_bytes<4>())));
        }
    private:
        std::span<const uint8_t> bytes_;

        template<size_t N>
        [[nodiscard]] constexpr std::span<const uint8_t, N> fetch_bytes(){
            const auto ret = std::span<const uint8_t, N>(bytes_.data(), N);
            bytes_ = std::span<const uint8_t>(bytes_.data() + N, bytes_.size() - N);
            return ret;
        }

        [[nodiscard]] constexpr std::span<const uint8_t> remaining() const {
            return bytes_;
        }
    };
public:
    static constexpr auto frame_to_event(const hal::BxCanFrame & frame) -> Result<Event, Error> {
        if(not frame.is_standard())
            return Err(Error::FrameIsNotStd);

        const auto frame_id = FrameId::from_stdid(frame.identifier().to_stdid());
        const auto axis_id = frame_id.axis_id;
        const auto command = frame_id.command;
        BytesReader reader(frame.payload_bytes());

        #define DEF_UNWRAP_PAYLOAD(expr) \
        ({\
            const auto may = (expr);\
            if(may.is_none())\
                return Err(Error::PayloadTooShort);\
            may.unwrap();\
        });\

        switch(command.kind()){
            case Command::GetEncoderCount:{
                const auto shadow_count = DEF_UNWRAP_PAYLOAD(reader.fetch_i32());
                const auto cpr_count = DEF_UNWRAP_PAYLOAD(reader.fetch_i32());
                return Ok(Event{
                    .axis_id = axis_id,
                    .signal = Msg::EncoderCount{
                        shadow_count: shadow_count,
                        cpr_count: cpr_count,
                    }
                });
            }
            
            // case Command::GetEncoderError:{
            //     const auto error_bits = DEF_UNWRAP_PAYLOAD(reader.fetch_u32());
            //     // Note: In your C++ code there's no EncoderErrorFlags::from_bits method shown,
            //     // so assuming direct bit interpretation or you'll need to implement that conversion
            //     const EncoderErrorFlags axis_error = EncoderErrorFlags::from_bits(error_bits);
            //     return Ok(Event{
            //         .axis_id = axis_id,
            //         .signal = Msg::EncoderErrorFlags{axis_error}
            //     });
            // }
            
            case Command::GetEncoderEstimates:{
                const auto position = DEF_UNWRAP_PAYLOAD(reader.fetch_f32());
                const auto velocity = DEF_UNWRAP_PAYLOAD(reader.fetch_f32());
                return Ok(Event{
                    .axis_id = axis_id,
                    .signal = Msg::EncoderEstimates{
                        position: position,
                        velocity: velocity,
                    }
                });
            }
            
            case Command::Heartbeat:{
                return Ok(
                    Event{
                        .axis_id = axis_id,
                        .signal = msgs::Heartbeat::form_payload(frame.payload())
                    }
                );
            }
            
            case Command::GetIq:{
                const auto setpoint = DEF_UNWRAP_PAYLOAD(reader.fetch_f32());
                const auto measurement = DEF_UNWRAP_PAYLOAD(reader.fetch_f32());
                return Ok(Event{
                    .axis_id = axis_id,
                    .signal = Msg::MotorCurrent{
                        setpoint: setpoint,
                        measurement: measurement,
                    }
                });
            }
            
            case Command::GetMotorError:{
                const auto error_bits = DEF_UNWRAP_PAYLOAD(reader.fetch_u32());
                // Similar bit mapping for MotorErrorFlags
                const MotorErrorFlags axis_error = MotorErrorFlags::from_bits(error_bits);
                return Ok(Event{
                    .axis_id = axis_id,
                    .signal = Msg::MotorErrorFlags{axis_error}
                });
            }
            
            case Command::GetBusVoltageCurrent:{
                const auto voltage = DEF_UNWRAP_PAYLOAD(reader.fetch_f32());
                return Ok(Event{
                    .axis_id = axis_id,
                    .signal = Msg::VbusVoltage{
                        voltage: voltage,
                    }
                });
            }
            default:
                return Err(Error::NotImplemented);
        }

        #undef DEF_UNWRAP_PAYLOAD
    }

};
}