#pragma once

#include "odrive_can_simple_primitive.hpp"

namespace ymd::robots::steadywin::can_simple{

using namespace primitive;




struct [[nodiscard]] FrameSerializer{
    using Error = Infallible;

    template<typename T = void>
    using IResult = Result<T, Error>;

    using CanResult = Result<hal::BxCanFrame, Error>;

    static constexpr CanResult get_motor_error(AxisId axis_id){
        return request(axis_id, Command::GetMotorError);
    }

    static constexpr CanResult get_encoder_estimates(AxisId axis_id){
        return request(axis_id, Command::GetEncoderEstimates);
    }

    static constexpr CanResult get_encoder_count(AxisId axis_id){
        return request(axis_id, Command::GetEncoderCount);
    }

    static constexpr CanResult get_iq(AxisId axis_id){
        return request(axis_id, Command::GetIq);
    }

    static constexpr CanResult get_sensorless_estimate(AxisId axis_id){
        return request(axis_id, Command::GetSensorlessEstimates);
    }

    static constexpr CanResult get_vbus_voltage(AxisId axis_id){
        return request(axis_id, Command::GetVbusVoltage);
    }

    static constexpr CanResult request(AxisId axis_id, Command cmd){ 
        return Ok(hal::BxCanFrame::from_remote(encode_id(axis_id, cmd)));
    }


    static constexpr CanResult emergency_stop(AxisId id) {
        return make_msg(FrameId{id, Command::Estop});
    }

    static constexpr CanResult reboot(AxisId id) {
        return make_msg(FrameId{id, Command::ResetOdrive});
    }


    static constexpr CanResult clear_errors(AxisId id) {
        return make_msg(FrameId{id, Command::ClearErrors});
    }


    static constexpr CanResult start_anticogging(AxisId id) {
        return make_msg(FrameId{id, Command::StartAnticogging});
    }


    static constexpr CanResult set_axis_node_id(AxisId id, AxisId new_id) {
        return make_msg(
            FrameId{id, Command::SetAxisNodeId}, 
            static_cast<uint32_t>(new_id.to_bits())
        );
    }


    static constexpr CanResult set_axis_requested_state(AxisId id, AxisState axis_state) {
        return make_msg(
            FrameId{id, Command::SetAxisRequestedState}, 
            static_cast<uint32_t>(axis_state)
        );
    }

    static constexpr CanResult set_input_current(AxisId id, float value) {
        const auto scaled_value = static_cast<uint32_t>(static_cast<int32_t>(
            100.0f * value + 0.5f)
        );
        return make_msg(
            FrameId{id, Command::SetInputCurrent}, 
            scaled_value
        );
    }


    static constexpr CanResult set_velocity_limit(AxisId id, fp32 value) {
        const auto value_bits = std::bit_cast<uint32_t>(value);
        return make_msg(
            FrameId{id, Command::SetVelLimit}, 
            value_bits
        );
    }

    static constexpr CanResult set_controller_modes(AxisId id, ControlMode control_mode) {
        return make_msg(
            FrameId{id, Command::SetControllerModes}, 
            static_cast<uint32_t>(control_mode),
            static_cast<uint32_t>(InputMode::PassThrough)
        );
    }

private:
    static constexpr hal::CanStdId encode_id(const AxisId axis_id, const Command cmd){
        return FrameId{axis_id, cmd}.to_stdid();
    }

    static constexpr CanResult make_msg(FrameId frame_id){
        return Ok(hal::BxCanFrame::from_empty(frame_id.to_stdid()));
    }

    static constexpr CanResult make_msg(FrameId frame_id, const uint32_t arg1){
        return Ok(
            hal::BxCanFrame::from_parts(
                frame_id.to_stdid(),
                hal::BxCanPayload::from_bytes(std::bit_cast<std::array<uint8_t, 4>>(arg1))
            )
        );
    }

    static constexpr CanResult make_msg(FrameId frame_id, const uint32_t arg1, const uint32_t arg2){
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