#pragma once

#include "steadywin_can_simple_primitive.hpp"

namespace ymd::robots::steadywin::can_simple{

using namespace primitive;




struct [[nodiscard]] FrameFactory{
    using Error = Infallible;

    template<typename T = void>
    using IResult = Result<T, Error>;

    using SerFrameResult = Result<hal::BxCanFrame, Error>;

    AxisId axis_id;
    constexpr SerFrameResult get_motor_error() const {
        return make_request(axis_id, Command::GetMotorError);
    }

    constexpr SerFrameResult get_encoder_estimates() const {
        return make_request(axis_id, Command::GetEncoderEstimates);
    }

    constexpr SerFrameResult get_encoder_count() const {
        return make_request(axis_id, Command::GetEncoderCount);
    }

    constexpr SerFrameResult get_iq() const {
        return make_request(axis_id, Command::GetIq);
    }

    constexpr SerFrameResult get_sensorless_estimate() const {
        return make_request(axis_id, Command::GetSensorlessEstimates);
    }

    constexpr SerFrameResult get_vbus_voltage() const {
        return make_request(axis_id, Command::GetVbusVoltage);
    }



    constexpr SerFrameResult emergency_stop(AxisId id)  const {
        return make_msg(FrameId{id, Command::Estop});
    }

    constexpr SerFrameResult reboot(AxisId id)  const {
        return make_msg(FrameId{id, Command::ResetOdrive});
    }


    constexpr SerFrameResult clear_errors(AxisId id)  const {
        return make_msg(FrameId{id, Command::ClearErrors});
    }


    constexpr SerFrameResult start_anticogging(AxisId id)  const {
        return make_msg(FrameId{id, Command::StartAnticogging});
    }


    constexpr SerFrameResult set_axis_node_id(AxisId new_id)  const {
        return make_msg(
            FrameId{id, Command::SetAxisNodeId}, 
            static_cast<uint32_t>(new_id.to_bits())
        );
    }


    constexpr SerFrameResult set_axis_requested_state(AxisState axis_state)  const {
        return make_msg(
            FrameId{id, Command::SetAxisRequestedState}, 
            static_cast<uint32_t>(axis_state)
        );
    }

    constexpr SerFrameResult set_input_current(float value)  const {
        const auto scaled_value = static_cast<uint32_t>(static_cast<int32_t>(
            100.0f * value + 0.5f)
        );
        return make_msg(
            FrameId{id, Command::SetInputCurrent}, 
            scaled_value
        );
    }


    constexpr SerFrameResult set_velocity_limit(math::fp32 value)  const {
        const auto value_bits = std::bit_cast<uint32_t>(value);
        return make_msg(
            FrameId{id, Command::SetVelLimit}, 
            value_bits
        );
    }

    constexpr SerFrameResult set_controller_modes(LoopMode loop_mode)  const {
        return make_msg(
            FrameId{axis_id, Command::SetControllerMode}, 
            static_cast<uint32_t>(loop_mode),
            static_cast<uint32_t>(InputMode::PassThrough)
        );
    }

private:
    static constexpr SerFrameResult make_request(const AxisId axis_id, Command cmd){ 
        return Ok(hal::BxCanFrame::from_remote(encode_id(axis_id, cmd)));
    }

    static constexpr hal::CanStdId encode_id(const AxisId axis_id, const Command cmd) {
        return FrameId{axis_id, cmd}.to_stdid();
    }

    static constexpr SerFrameResult make_msg(FrameId frame_id){
        return Ok(hal::BxCanFrame::from_empty(frame_id.to_stdid()));
    }

    static constexpr SerFrameResult make_msg(FrameId frame_id, const uint32_t arg1){
        return Ok(
            hal::BxCanFrame::from_parts(
                frame_id.to_stdid(),
                hal::BxCanPayload::from_bytes(std::bit_cast<std::array<uint8_t, 4>>(arg1))
            )
        );
    }

    static constexpr SerFrameResult make_msg(FrameId frame_id, const uint32_t arg1, const uint32_t arg2){
        std::array<uint8_t, 8> bytes;
        const auto arr1 = std::bit_cast<std::array<uint8_t, 4>>(arg1);
        const auto arr2 = std::bit_cast<std::array<uint8_t, 4>>(arg2);
        std::copy(arr1.begin(), arr1.end(), bytes.begin());
        std::copy(arr2.begin(), arr2.end(), bytes.begin() + 4);
        return Ok(
            hal::BxCanFrame::from_parts(
                frame_id.to_stdid(), 
                hal::BxCanPayload::from_bytes(bytes)
            )
        );
    }

};


}