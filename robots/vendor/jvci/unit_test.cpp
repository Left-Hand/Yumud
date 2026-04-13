#include "jvci_primitive.hpp"
#include "jvci_framefactory.hpp"

using namespace ymd::robots::jvci;


namespace {


// 测试强类型转换
[[maybe_unused]] static void test_strong_types() {
    {
        // PositionCode 正值测试
        static constexpr auto p1 = PositionCode::from_degrees(360.5f);
        static_assert(std::abs(p1.to_degrees() - 360.5f) < 0.01f);
    }
    {
        // PositionCode 负值测试
        static constexpr auto p2 = PositionCode::from_degrees(-114.514f);
        static_assert(std::abs(p2.to_degrees() - (-114.514f)) < 0.01f);
    }
    {
        // SpeedCode 正值测试
        static constexpr auto s1 = SpeedCode::from_rpm(500.0f);
        static_assert(std::abs(s1.to_rpm() - 500.0f) < 0.01f);
    }
    {
        // SpeedCode 负值测试
        static constexpr auto s2 = SpeedCode::from_rpm(-500.23f);
        static_assert(std::abs(s2.to_rpm() - (-500.23f)) < 0.01f);
    }
    {
        // BusbarVoltageCode 测试
        static constexpr auto v1 = BusbarVoltageCode::from_volt(12.3f);
        static_assert(std::abs(v1.to_volt() - 12.3f) < 0.01f);
    }
    {
        // TorqueCode 测试
        static constexpr auto t1 = TorqueCode::from_newton_meter(0.2f);
        static_assert(std::abs(t1.to_newton_meter() - 0.2f) < 0.01f);
    }
}

[[maybe_unused]] static void test_example_msgs() {
    static constexpr auto factory = CanFrameRquestFactory{.node_id = NodeId::try_from_u8(0x01).unwrap()};

    // 1. 读取电源电压（寄存器0x0004，倍数×10，范围0-100V）
    // 请求帧ID:0x601，数据:`4B 00 04 00 00 00 00 00`；回复:`4B 00 04 00 00 78 00 00`（12.0V）
    {
        static constexpr auto frame = factory.read_power_voltage();
        static constexpr auto bytes = frame.payload_bytes();

        static_assert(bytes[0] == 0x4b);  // ReadReg16
        static_assert(bytes[1] == 0x00);  // 寄存器地址高字节
        static_assert(bytes[2] == 0x04);  // 寄存器地址低字节 (0x0004)
        static_assert(bytes[3] == 0x00);  // 对齐
        static_assert(bytes[4] == 0x00);  // 无参数
        static_assert(bytes[5] == 0x00);
        static_assert(bytes[6] == 0x00);
        static_assert(bytes[7] == 0x00);
    }

    // 2. 进入闭环（寄存器0x00A2，写入1执行）
    // 请求帧ID:0x601，数据:`2B 00 A2 00 00 01 00 00`；回复:`2A 00 5C DD 00 00 00 00`
    {
        static constexpr auto frame = factory.enter_close_loop();
        static constexpr auto bytes = frame.payload_bytes();

        static_assert(bytes[0] == 0x2b);  // WriteReg16
        static_assert(bytes[1] == 0x00);  // 寄存器地址高字节
        static_assert(bytes[2] == 0xa2);  // 寄存器地址低字节 (0x00A2)
        static_assert(bytes[3] == 0x00);  // 对齐
        static_assert(bytes[4] == 0x00);  // 数据高字节
        static_assert(bytes[5] == 0x01);  // 数据低字节 (1)
        static_assert(bytes[6] == 0x00);
        static_assert(bytes[7] == 0x00);
    }

    // 3. 设置绝对位置 0°（寄存器0x0023，倍数×100）
    // 请求帧ID:0x601，数据:`23 00 23 00 00 00 00 00`；回复:`2A 00 5C DD 00 00 00 00`
    {
        static constexpr auto frame = factory.set_absolute_position(PositionCode::from_degrees(0.0f));
        static constexpr auto bytes = frame.payload_bytes();

        static_assert(bytes[0] == 0x23);  // WriteReg32
        static_assert(bytes[1] == 0x00);  // 寄存器地址高字节
        static_assert(bytes[2] == 0x23);  // 寄存器地址低字节 (0x0023)
        static_assert(bytes[3] == 0x00);  // 对齐
        static_assert(bytes[4] == 0x00);  // 位置高字节
        static_assert(bytes[5] == 0x00);
        static_assert(bytes[6] == 0x00);
        static_assert(bytes[7] == 0x00);  // 位置低字节 (0)
    }

    // 4. 设置绝对位置 360°（寄存器0x0023，编码值0x8CA0=36000）
    // 请求帧ID:0x601，数据:`23 00 23 00 00 00 8C A0`；回复:`2A 00 00 00 00 00 00 00`
    {
        static constexpr auto frame = factory.set_absolute_position(PositionCode::from_degrees(360.0f));
        static constexpr auto bytes = frame.payload_bytes();

        static_assert(bytes[0] == 0x23);  // WriteReg32
        static_assert(bytes[1] == 0x00);
        static_assert(bytes[2] == 0x23);  // 寄存器0x0023
        static_assert(bytes[3] == 0x00);
        static_assert(bytes[4] == 0x00);  // 位置: 0x00008CA0
        static_assert(bytes[5] == 0x00);
        static_assert(bytes[6] == 0x8c);
        static_assert(bytes[7] == 0xa0);
    }

    // 5. PVT指令（-360°/150rpm/60%）
    // 请求帧ID:0x601，数据:`25 FF FF 73 60 00 96 3C`；回复:`2A 00 8C A1 00 00 00 00`
    {
        static constexpr auto frame = factory.pvt_command(
            PositionCode::from_degrees(-360.0f),
            PvSpeedCode::from_rpm(150.0f),
            PvTorqueCode::from_percents(60.0f)
        );
        static constexpr auto bytes = frame.payload_bytes();

        static_assert(bytes[0] == 0x25);  // CmdPVT
        static_assert(bytes[1] == 0xff);  // 位置: 0xFFFF73 60 (-36000)
        static_assert(bytes[2] == 0xff);
        static_assert(bytes[3] == 0x73);
        static_assert(bytes[4] == 0x60);
        static_assert(bytes[5] == 0x00);  // 速度: 0x0096
        static_assert(bytes[6] == 0x96);
        static_assert(bytes[7] == 0x3c);  // 力矩: 0x3C (60%)
    }

    // 6. PVT指令（0°/60rpm/80%）
    // 请求帧ID:0x601，数据:`25 00 00 00 00 00 3C 50`；回复:`2A FF 73 5F 00 00 00 00`
    {
        static constexpr auto frame = factory.pvt_command(
            PositionCode::from_degrees(0.0f),
            PvSpeedCode::from_rpm(60.0f),
            PvTorqueCode::from_percents(80.0f)
        );
        static constexpr auto bytes = frame.payload_bytes();

        static_assert(bytes[0] == 0x25);  // CmdPVT
        static_assert(bytes[1] == 0x00);  // 位置: 0x00000000 (0°)
        static_assert(bytes[2] == 0x00);
        static_assert(bytes[3] == 0x00);
        static_assert(bytes[4] == 0x00);
        static_assert(bytes[5] == 0x00);  // 速度: 0x003C
        static_assert(bytes[6] == 0x3c);
        static_assert(bytes[7] == 0x50);  // 力矩: 0x50 (80%)
    }

    // 7. 切换速度模式（寄存器0x0060，值=1）
    // 请求帧ID:0x601，数据:`2B 00 60 00 00 01 00 00`；回复:`2A 00 00 00 00 00 00 00`
    {
        static constexpr auto frame = factory.set_control_mode(ControlMode::Speed);
        static constexpr auto bytes = frame.payload_bytes();

        static_assert(bytes[0] == 0x2b);  // WriteReg16
        static_assert(bytes[1] == 0x00);
        static_assert(bytes[2] == 0x60);  // 寄存器0x0060 (ControlMode)
        static_assert(bytes[3] == 0x00);
        static_assert(bytes[4] == 0x00);  // 模式值
        static_assert(bytes[5] == 0x01);  // Speed = 1
        static_assert(bytes[6] == 0x00);
        static_assert(bytes[7] == 0x00);
    }

    // 8. 设置速度 500rpm（寄存器0x0022，编码值0xC350=50000）
    // 请求帧ID:0x601，数据:`23 00 21 00 00 00 C3 50`；回复:`2A FF FF FD 00 00 00 00`
    {
        static constexpr auto frame = factory.set_speed(SpeedCode::from_rpm(500.0f));
        static constexpr auto bytes = frame.payload_bytes();

        static_assert(bytes[0] == 0x23);  // WriteReg32
        static_assert(bytes[1] == 0x00);
        static_assert(bytes[2] == 0x21);  // 寄存器0x0021 (SetSpeedH)
        static_assert(bytes[3] == 0x00);
        static_assert(bytes[4] == 0x00);  // 速度: 0x0000C350
        static_assert(bytes[5] == 0x00);
        static_assert(bytes[6] == 0xc3);
        static_assert(bytes[7] == 0x50);
    }

    // 9. 设置速度 0rpm（寄存器0x0022，编码值0x00000000）
    // 请求帧ID:0x601，数据:`23 00 21 00 00 00 00 00`；回复:`2A B9 07 AD 01 F4 00 00`
    {
        static constexpr auto frame = factory.set_speed(SpeedCode::from_rpm(0.0f));
        static constexpr auto bytes = frame.payload_bytes();

        static_assert(bytes[0] == 0x23);  // WriteReg32
        static_assert(bytes[1] == 0x00);
        static_assert(bytes[2] == 0x21);  // 寄存器0x0021
        static_assert(bytes[3] == 0x00);
        static_assert(bytes[4] == 0x00);  // 速度: 0x00000000
        static_assert(bytes[5] == 0x00);
        static_assert(bytes[6] == 0x00);
        static_assert(bytes[7] == 0x00);
    }

    // 10. 切换位置梯形轨迹模式（寄存器0x0060，值=2）
    // 请求帧ID:0x601，数据:`2B 00 60 00 00 02 00 00`；回复:`2A BA F6 7B 00 00 00 00`
    {
        static constexpr auto frame = factory.set_control_mode(ControlMode::TrapezoidPosition);
        static constexpr auto bytes = frame.payload_bytes();

        static_assert(bytes[0] == 0x2b);  // WriteReg16
        static_assert(bytes[1] == 0x00);
        static_assert(bytes[2] == 0x60);  // 寄存器0x0060
        static_assert(bytes[3] == 0x00);
        static_assert(bytes[4] == 0x00);  // 模式值
        static_assert(bytes[5] == 0x02);  // TrapezoidPosition = 2
        static_assert(bytes[6] == 0x00);
        static_assert(bytes[7] == 0x00);
    }

    // 11. 设置相对位置 360°（寄存器0x0025，编码值0x8CA0=36000）
    // 请求帧ID:0x601，数据:`23 00 25 00 00 00 8C A0`；回复:`2A BA F6 7B 00 00 00 00`
    {
        static constexpr auto frame = factory.set_relative_position(PositionCode::from_degrees(360.0f));
        static constexpr auto bytes = frame.payload_bytes();

        static_assert(bytes[0] == 0x23);  // WriteReg32
        static_assert(bytes[1] == 0x00);
        static_assert(bytes[2] == 0x25);  // 寄存器0x0025 (SetRelPositionH)
        static_assert(bytes[3] == 0x00);
        static_assert(bytes[4] == 0x00);  // 位置: 0x00008CA0
        static_assert(bytes[5] == 0x00);
        static_assert(bytes[6] == 0x8c);
        static_assert(bytes[7] == 0xa0);
    }

    // 12. 空闲模式（寄存器0x00A0，写入1执行）
    // 请求帧ID:0x601，数据:`2B 00 A0 00 00 01 00 00`；回复:`2A BB 83 1B 00 00 00 00`
    {
        static constexpr auto frame = factory.idle_state();
        static constexpr auto bytes = frame.payload_bytes();

        static_assert(bytes[0] == 0x2b);  // WriteReg16
        static_assert(bytes[1] == 0x00);
        static_assert(bytes[2] == 0xa0);  // 寄存器0x00A0 (IdleState)
        static_assert(bytes[3] == 0x00);
        static_assert(bytes[4] == 0x00);  // 数据值
        static_assert(bytes[5] == 0x01);  // 1 = 执行
        static_assert(bytes[6] == 0x00);
        static_assert(bytes[7] == 0x00);
    }

    // 13. 重启驱动器（寄存器0x00A5，写入1执行，重启约1.5秒）
    // 请求帧ID:0x601，数据:`2B 00 A5 00 00 01 00 00`；回复:`2A BB 82 DF 00 00 00 00`
    {
        static constexpr auto frame = factory.restart_driver();
        static constexpr auto bytes = frame.payload_bytes();

        static_assert(bytes[0] == 0x2b);  // WriteReg16
        static_assert(bytes[1] == 0x00);
        static_assert(bytes[2] == 0xa5);  // 寄存器0x00A5 (RestartDriver)
        static_assert(bytes[3] == 0x00);
        static_assert(bytes[4] == 0x00);  // 数据值
        static_assert(bytes[5] == 0x01);  // 1 = 执行
        static_assert(bytes[6] == 0x00);
        static_assert(bytes[7] == 0x00);
    }
}


}


