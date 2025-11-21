#include "odrive_can_simple_primitive.hpp"

using namespace ymd;
using namespace robots::odrive::can_simple;
using namespace primitive;



[[maybe_unused]] void test1(){
// 验证错误结构体大小
static_assert(sizeof(AxisError) == 4, "AxisError should be 4 bytes");
static_assert(sizeof(EncoderError) == 4, "EncoderError should be 4 bytes");
static_assert(sizeof(MotorError) == 4, "MotorError should be 4 bytes");

// 验证枚举大小
static_assert(sizeof(AxisState) == 1, "AxisState should be 1 byte");
static_assert(sizeof(CommandKind) == 1, "CommandKind should be 1 byte");
static_assert(sizeof(ControlMode) == 1, "ControlMode should be 1 byte");
static_assert(sizeof(InputMode) == 1, "InputMode should be 1 byte");
// 测试位域转换
static_assert(AxisError::from_bits(0).invalid_state == 0, "Invalid state bit should be 0");
static_assert(AxisError::from_bits(1).invalid_state == 1, "Invalid state bit should be 1");
static_assert(EncoderError::from_bits(1).unstable_gain == 1, "Unstable gain bit should be 1");
static_assert(MotorError::from_bits(1).phase_resistance_out_of_range == 1, "Phase resistance bit should be 1");

}
[[maybe_unused]] void test2(){

// 测试 FrameId 编码和解码
static constexpr AxisId test_axis_id{.bits = 0b101010};
static constexpr Command test_command(CommandKind::SetInputCurrent);
static constexpr FrameId test_frame_id{.axis_id = test_axis_id, .command = test_command};

// 测试 to_stdid 和 from_stdid 的一致性
static constexpr auto encoded_stdid = test_frame_id.to_stdid();
static constexpr auto decoded_frame_id = FrameId::from_stdid(encoded_stdid);
static_assert(decoded_frame_id.axis_id.bits == test_axis_id.bits, "Axis ID should match after round-trip");
static_assert(decoded_frame_id.command.kind() == test_command.kind(), "Command should match after round-trip");

// 测试特定值
static constexpr auto test_stdid = hal::CanStdId::from_u11(0b101010'00100); // axis_id=0b101010, command=4
static constexpr auto decoded_test = FrameId::from_stdid(test_stdid);
static_assert(decoded_test.axis_id.bits == 0b101010, "Axis ID decoding test");
static_assert(decoded_test.command.kind() == CommandKind::GetEncoderError, "Command decoding test");

}
[[maybe_unused]] void test3(){
// 测试 FrameId 编码和解码
static constexpr AxisId test_axis_id{.bits = 0b101010};
static constexpr Command test_command(CommandKind::SetInputCurrent);
static constexpr FrameId test_frame_id{.axis_id = test_axis_id, .command = test_command};

// 测试 to_stdid 和 from_stdid 的一致性
static constexpr auto encoded_stdid = test_frame_id.to_stdid();
static constexpr auto decoded_frame_id = FrameId::from_stdid(encoded_stdid);
static_assert(decoded_frame_id.axis_id.bits == test_axis_id.bits, "Axis ID should match after round-trip");
static_assert(decoded_frame_id.command.kind() == test_command.kind(), "Command should match after round-trip");

}
[[maybe_unused]] void test4(){

// 测试特定值
static constexpr auto test_stdid = hal::CanStdId::from_u11(0b101010'00100); // axis_id=0b101010, command=4
static constexpr auto decoded_test = FrameId::from_stdid(test_stdid);
static_assert(decoded_test.axis_id.bits == 0b101010, "Axis ID decoding test");
static_assert(decoded_test.command.kind() == CommandKind::GetEncoderError, "Command decoding test");

// 测试命令枚举值
static_assert(static_cast<uint8_t>(CommandKind::Undefined) == 0, "Undefined command should be 0");
static_assert(static_cast<uint8_t>(CommandKind::OdriveHeartbeat) == 1, "OdriveHeartbeat command should be 1");
static_assert(static_cast<uint8_t>(CommandKind::OdriveEstop) == 2, "OdriveEstop command should be 2");
static_assert(static_cast<uint8_t>(CommandKind::GetMotorError) == 3, "GetMotorError command should be 3");

}
[[maybe_unused]] void test5(){
// 测试 Command 结构体
static constexpr Command test_cmd(CommandKind::SetAxisNodeId);
static_assert(test_cmd.kind() == CommandKind::SetAxisNodeId, "Command kind should match");
static_assert(Command::from_bits(static_cast<uint8_t>(CommandKind::ResetOdrive)).kind() == CommandKind::ResetOdrive, "Command from_bits should work");

// 测试控制模式枚举值
static_assert(static_cast<uint8_t>(ControlMode::VoltageControl) == 0, "VoltageControl should be 0");
static_assert(static_cast<uint8_t>(ControlMode::CurrentControl) == 1, "CurrentControl should be 1");
static_assert(static_cast<uint8_t>(ControlMode::VelocityControl) == 2, "VelocityControl should be 2");
static_assert(static_cast<uint8_t>(ControlMode::PositionControl) == 3, "PositionControl should be 3");

}
[[maybe_unused]] void test6(){

// 测试输入模式枚举值
static_assert(static_cast<uint8_t>(InputMode::Inactive) == 0, "Inactive should be 0");
static_assert(static_cast<uint8_t>(InputMode::PassThrough) == 1, "PassThrough should be 1");

// 测试消息类型的变体类型
static_assert(std::is_same_v<Msg::EncoderCount, msgs::EncoderCount>, "EncoderCount type should match");
static_assert(std::is_same_v<Msg::EncoderEstimates, msgs::EncoderEstimates>, "EncoderEstimates type should match");
static_assert(std::is_same_v<Msg::Heartbeat, msgs::Heartbeat>, "Heartbeat type should match");
static_assert(std::is_same_v<Msg::MotorCurrent, msgs::MotorCurrent>, "MotorCurrent type should match");
static_assert(std::is_same_v<Msg::VbusVoltage, msgs::VbusVoltage>, "VbusVoltage type should match");
}


[[maybe_unused]] void test7(){
// 测试 AxisId 结构体
static constexpr AxisId test_id{.bits = 42};
static_assert(test_id.bits == 42, "AxisId bits should match");
static_assert(AxisId::from_bits(123).bits == 123, "AxisId from_bits should work");

// 测试 AxisState 枚举值
static_assert(static_cast<uint8_t>(AxisState::Undefined) == 0, "Undefined state should be 0");
static_assert(static_cast<uint8_t>(AxisState::Idle) == 1, "Idle state should be 1");
static_assert(static_cast<uint8_t>(AxisState::ClosedLoopControl) == 8, "ClosedLoopControl state should be 8");
}

#if 0
[[maybe_unused]] void test_ser(){
    // 测试空消息序列化
static constexpr AxisId test_axis_id{.bits = 1};
static constexpr FrameId heartbeat_frame_id{test_axis_id, Command(CommandKind::OdriveHeartbeat)};
static constexpr auto heartbeat_result = FrameSerializer::make_msg(heartbeat_frame_id);
static_assert(heartbeat_result.is_ok(), "Heartbeat message creation should succeed");
static_assert(heartbeat_result.unwrap().is_remote(), "Heartbeat should be a remote frame");
static_assert(heartbeat_result.unwrap().identifier().to_stdid().to_u11() == ((1 << 5) | static_cast<uint8_t>(CommandKind::OdriveHeartbeat)), "Heartbeat frame ID should be correct");

// 测试带一个参数的消息序列化
static constexpr FrameId set_state_frame_id{test_axis_id, Command(CommandKind::SetAxisRequestedState)};
static constexpr auto set_state_result = FrameSerializer::make_msg(set_state_frame_id, static_cast<uint32_t>(AxisState::ClosedLoopControl));
static_assert(set_state_result.is_ok(), "Set state message creation should succeed");
static_assert(!set_state_result.unwrap().is_remote(), "Set state should not be a remote frame");
static_assert(set_state_result.unwrap().length() == 4, "Set state payload should be 4 bytes");

// 测试带两个参数的消息序列化
static constexpr FrameId set_controller_frame{test_axis_id, Command(CommandKind::SetControllerModes)};
static constexpr auto set_controller_result = FrameSerializer::make_msg(
    set_controller_frame, 
    static_cast<uint32_t>(ControlMode::PositionControl),
    static_cast<uint32_t>(InputMode::PassThrough)
);
static_assert(set_controller_result.is_ok(), "Set controller modes message creation should succeed");
static_assert(set_controller_result.unwrap().length() == 8, "Set controller modes payload should be 8 bytes");
}

#endif

[[maybe_unused]] void test_de(){
    // 测试编码和解码一致性
static constexpr AxisId axis_id{.bits = 5};
static constexpr Command command(CommandKind::GetEncoderEstimates);
static constexpr FrameId frame_id{axis_id, command};

// 创建一个模拟的编码器估计值响应帧
static constexpr auto stdid = frame_id.to_stdid();
static constexpr std::array<uint8_t, 8> payload_bytes = {
    0x00, 0x00, 0x80, 0x3F, // 1.0f as little-endian bytes
    0x00, 0x00, 0x00, 0x40  // 2.0f as little-endian bytes
};
static constexpr auto test_frame = hal::CanClassicFrame::from_parts(
    stdid,
    hal::CanClassicPayload::from_bytes(payload_bytes)
);

// 测试反序列化
static constexpr auto deserialize_result = FrameDeserializer::map_frame_to_event(test_frame);
static_assert(deserialize_result.is_ok(), "Deserialization should succeed");
static_assert(deserialize_result.unwrap().axis_id.bits == 5, "Axis ID should match");
static_assert(deserialize_result.unwrap().signal.is<msgs::EncoderEstimates>(), "Should be EncoderEstimates message");
}

[[maybe_unused]] void test_cmd(){
    // 测试紧急停止命令
static constexpr auto estop_result = FrameSerializer::emergency_stop(AxisId{.bits = 3});
static_assert(estop_result.is_ok(), "Emergency stop creation should succeed");
static_assert(estop_result.unwrap().identifier().to_stdid().to_u11() == ((3 << 5) | static_cast<uint8_t>(CommandKind::OdriveEstop)), "Estop frame ID should be correct");

// 测试重启命令
static constexpr auto reboot_result = FrameSerializer::reboot(AxisId{.bits = 2});
static_assert(reboot_result.is_ok(), "Reboot creation should succeed");
static_assert(reboot_result.unwrap().identifier().to_stdid().to_u11() == ((2 << 5) | static_cast<uint8_t>(CommandKind::ResetOdrive)), "Reboot frame ID should be correct");

// 测试清除错误命令
static constexpr auto clear_errors_result = FrameSerializer::clear_errors(AxisId{.bits = 4});
static_assert(clear_errors_result.is_ok(), "Clear errors creation should succeed");
static_assert(clear_errors_result.unwrap().identifier().to_stdid().to_u11() == ((4 << 5) | static_cast<uint8_t>(CommandKind::ClearErrors)), "Clear errors frame ID should be correct");

// 测试设置轴ID命令
static constexpr auto set_axis_id_result = FrameSerializer::set_axis_node_id(AxisId{.bits = 1}, AxisId{.bits = 10});
static_assert(set_axis_id_result.is_ok(), "Set axis node ID creation should succeed");
static_assert(set_axis_id_result.unwrap().length() == 4, "Set axis node ID payload should be 4 bytes");
}

[[maybe_unused]] void test_err(){
    // 测试扩展帧被拒绝
static constexpr auto extended_id = hal::CanExtId::from_u29(0x12345678);
static constexpr auto extended_frame = hal::CanClassicFrame::from_remote(extended_id);
static constexpr auto ext_frame_result = FrameDeserializer::map_frame_to_event(extended_frame);
static_assert(ext_frame_result.is_err(), "Extended frame should result in error");
static_assert(ext_frame_result.unwrap_err() == FrameDeserializer::Error::FrameIsNotStd, "Error should be FrameIsNotStd");

#if 0

// 测试负载太短的情况
static constexpr std::array<uint8_t, 3> short_payload = {0x01, 0x02, 0x03};
static constexpr auto short_frame = hal::CanClassicFrame::from_parts(
    FrameId{AxisId{.bits = 1}, Command(CommandKind::GetEncoderCount)}.to_stdid(),
    hal::CanClassicPayload::from_bytes(short_payload)
);


static constexpr auto short_frame_result = FrameDeserializer::map_frame_to_event(short_frame);
static_assert(short_frame_result.is_ok(), "Short frame should result in error");
static_assert(short_frame_result.is_err(), "Short frame should result in error");
static_assert(short_frame_result.unwrap_err() == FrameDeserializer::Error::PayloadTooShort, "Error should be PayloadTooShort");
#endif
}

[[maybe_unused]] void test_numeric(){ 
// 测试浮点数编码一致性
static constexpr float test_float = 3.14159f;
static constexpr auto float_bits = std::bit_cast<uint32_t>(test_float);
static_assert(std::bit_cast<float>(float_bits) == test_float, "Float bit casting should be reversible");

// 测试电流值缩放
static constexpr float current_value = 2.5f;
static constexpr auto scaled_current = static_cast<uint32_t>(static_cast<int32_t>(100.0f * current_value + 0.5f));
static_assert(scaled_current == 250, "Current scaling should work correctly"); // 2.5 * 100 = 250

}

[[maybe_unused]] void test_all(){ 
// 测试完整的序列化->反序列化流程
static constexpr AxisId test_id{.bits = 7};
static constexpr AxisState target_state = AxisState::ClosedLoopControl;

// 序列化
static constexpr auto serialize_result = FrameSerializer::set_axis_requested_state(test_id, target_state);
static_assert(serialize_result.is_ok(), "Serialization should succeed");


#if 0
// 反序列化
static constexpr auto deserialize_result2 = FrameDeserializer::map_frame_to_event(serialize_result.unwrap());
static_assert(deserialize_result2.is_ok(), "Deserialization should succeed");
static_assert(deserialize_result2.unwrap().axis_id.bits == 7, "Axis ID should match");
static_assert(deserialize_result2.unwrap().signal.is<msgs::Heartbeat>() == false, "Should not be heartbeat");
#endif
}
