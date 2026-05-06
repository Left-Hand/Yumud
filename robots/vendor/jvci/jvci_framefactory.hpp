#pragma once

#include "jvci_primitive.hpp"


namespace ymd::robots::jvci{
struct [[nodiscard]] CanRequestFrameFactory final{
public:
    NodeId node_id;

/// ========== 读取方法 ==========

    /// 读取电源电压（寄存器0x0004，倍数×10，范围0-100V）
    constexpr hal::ClassicCanFrame read_power_voltage() const noexcept {
        return make_can_frame(make_read_context(Command::ReadReg16, RegAddr::PowerVoltage));
    }

    /// 读取母线电流（寄存器0x0005，倍数×100，范围±20A）
    constexpr hal::ClassicCanFrame read_bus_current() const noexcept {
        return make_can_frame(make_read_context(Command::ReadReg16, RegAddr::BusCurrent));
    }

    /// 读取实时速度（寄存器0x0006，倍数×100，范围±10000rpm）
    constexpr hal::ClassicCanFrame read_real_speed() const noexcept {
        return make_can_frame(make_read_context(Command::ReadReg32, RegAddr::RealSpeedH));
    }

    /// 读取实时位置（寄存器0x0008，倍数×100，范围-11796120°~11796480°）
    constexpr hal::ClassicCanFrame read_real_position() const noexcept {
        return make_can_frame(make_read_context(Command::ReadReg32, RegAddr::RealPositionL));
    }

    /// 读取驱动器温度（寄存器0x000A，倍数×10，范围0-150°C）
    constexpr hal::ClassicCanFrame read_driver_temperature() const noexcept {
        return make_can_frame(make_read_context(Command::ReadReg16, RegAddr::DriverTemperature));
    }

    /// 读取电机温度（寄存器0x000B，倍数×10，范围0-150°C）
    constexpr hal::ClassicCanFrame read_motor_temperature() const noexcept {
        return make_can_frame(make_read_context(Command::ReadReg16, RegAddr::MotorTemperature));
    }

    /// 读取错误信息（寄存器0x000C，32bit）
    constexpr hal::ClassicCanFrame read_error_info() const noexcept {
        return make_can_frame(make_read_context(Command::ReadReg32, RegAddr::ErrorInfoH));
    }

    /// ========== 写入方法 ==========

    /// 设置力矩（寄存器0x0020，倍数×100，范围±100Nm）
    constexpr hal::ClassicCanFrame set_torque(const TorqueCode code) const noexcept {
        return make_can_frame(make_write16_context(RegAddr::SetTorque, code));
    }

    /// 设置速度（寄存器0x0022，倍数×100，范围±10000rpm）
    constexpr hal::ClassicCanFrame set_speed(const SpeedCode code) const noexcept {
        return make_can_frame(make_write32_context(RegAddr::SetSpeedH, code));
    }

    /// 设置绝对位置（寄存器0x0023，倍数×100，范围-11796120°~11796480°）
    constexpr hal::ClassicCanFrame set_absolute_position(const PositionCode code) const noexcept {
        return make_can_frame(make_write32_context(RegAddr::SetAbsPositionH, code));
    }

    /// 设置相对位置（寄存器0x0025，倍数×100，范围-11796120°~11796480°）
    constexpr hal::ClassicCanFrame set_relative_position(const PositionCode code) const noexcept {
        return make_can_frame(make_write32_context(RegAddr::SetRelPositionH, code));
    }

    /// 设置低速（寄存器0x0027，倍数×100，范围±300rpm）
    constexpr hal::ClassicCanFrame set_low_speed(const LowSpeedCode code) const noexcept {
        return make_can_frame(make_write16_context(RegAddr::SetLowSpeed, code));
    }

    /// ========== 控制模式与状态 ==========

    /// 切换控制模式（寄存器0x0060，范围0-5）
    constexpr hal::ClassicCanFrame set_control_mode(const ControlMode mode) const noexcept {
        return make_can_frame(make_write16_context(RegAddr::ControlMode, mode));
    }

    /// 进入空闲状态（寄存器0x00A0，写入1执行）
    constexpr hal::ClassicCanFrame idle_state() const noexcept {
        return make_can_frame(make_write16_context(RegAddr::IdleState, static_cast<uint16_t>(1)));
    }

    /// ========== 校准与初始化 ==========

    /// 校准电机（寄存器0x00A1，写入1执行）
    constexpr hal::ClassicCanFrame calibrate_motor() const noexcept {
        return make_can_frame(make_write16_context(RegAddr::CalibrateMotor, static_cast<uint16_t>(1)));
    }

    /// 进入闭环（寄存器0x00A2，写入1执行）
    constexpr hal::ClassicCanFrame enter_close_loop() const noexcept {
        return make_can_frame(make_write16_context(RegAddr::EnterCloseLoop, static_cast<uint16_t>(1)));
    }

    /// ========== 参数管理 ==========

    /// 擦除参数（寄存器0x00A3，写入1执行）
    constexpr hal::ClassicCanFrame erase_param() const noexcept {
        return make_can_frame(make_write16_context(RegAddr::EraseParam, static_cast<uint16_t>(1)));
    }

    /// 保存参数（寄存器0x00A4，写入1执行）
    constexpr hal::ClassicCanFrame save_param() const noexcept {
        return make_can_frame(make_write16_context(RegAddr::SaveParam, static_cast<uint16_t>(1)));
    }

    /// 重启驱动器（寄存器0x00A5，写入1执行，重启约1.5秒）
    constexpr hal::ClassicCanFrame restart_driver() const noexcept {
        return make_can_frame(make_write16_context(RegAddr::RestartDriver, static_cast<uint16_t>(1)));
    }

    /// ========== 原点设置 ==========

    /// 设置原点（寄存器0x00A6，写入1执行，保存偏置角并重启）
    constexpr hal::ClassicCanFrame set_origin() const noexcept {
        return make_can_frame(make_write16_context(RegAddr::SetOrigin, static_cast<uint16_t>(1)));
    }

    /// 设置临时原点（寄存器0x00A7，写入1执行，立即执行不保存）
    constexpr hal::ClassicCanFrame set_temp_origin() const noexcept {
        return make_can_frame(make_write16_context(RegAddr::SetTempOrigin, static_cast<uint16_t>(1)));
    }

    /// ========== 特殊指令 ==========

    /// PV指令（位置-速度）：以指定速度转到目标位置
    /// @param p 目标位置
    /// @param s PV指令专用速度码
    constexpr hal::ClassicCanFrame pv_command(const PositionCode p, const PvSpeedCode s) const noexcept {
        return make_can_frame(make_triple_context(
            Command::CmdPV,
            std::bit_cast<uint32_t>(p.bits),
            std::bit_cast<uint16_t>(s.bits),
            static_cast<uint8_t>(0)
        ));
    }

    /// PVT指令（位置-速度-力矩）：位置梯形轨迹模式下执行
    /// @param p 目标位置
    /// @param s PVT指令专用速度码
    /// @param t 力矩百分比码（0-100%）
    constexpr hal::ClassicCanFrame pvt_command(const PositionCode p, const PvSpeedCode s, const PvTorqueCode t) const noexcept {
        return make_can_frame(make_triple_context(
            Command::CmdPVT,
            std::bit_cast<uint32_t>(p.bits),
            std::bit_cast<uint16_t>(s.bits),
            std::bit_cast<uint8_t>(t.bits)
        ));
    }
private:
    /// 将缓冲区打包为 CAN 标准帧 直接传值
    constexpr hal::ClassicCanFrame make_can_frame(const std::array<uint8_t, 8> buf) const noexcept {
        return hal::ClassicCanFrame::from_parts(
            make_request_canid(node_id),
            hal::ClassicCanPayload::from_u8x8(buf)
        );
    }

    /// 生成三参数指令帧（PV/PVT指令）
    /// @param command 命令字（CmdPV=0x24 或 CmdPVT=0x25）
    /// @param arg1 位置参数（32bit）
    /// @param arg2 速度参数（16bit）
    /// @param arg3 力矩/填充参数（8bit）
    static constexpr std::array<uint8_t, 8> make_triple_context(
        const Command command,
        const uint32_t arg1,
        const uint16_t arg2,
        const uint8_t arg3
    ){
        std::array<uint8_t, 8> buf = {
            static_cast<uint8_t>(command),
            static_cast<uint8_t>((arg1) >> 24),
            static_cast<uint8_t>((arg1) >> 16),
            static_cast<uint8_t>((arg1) >> 8),
            static_cast<uint8_t>((arg1)),

            static_cast<uint8_t>((arg2) >> 8),
            static_cast<uint8_t>((arg2)),

            static_cast<uint8_t>((arg3)),
        };

        return buf;
    }

    /// 生成无参数指令帧的基础模板
    /// @param command 命令字
    /// @param reg_addr 寄存器地址
    /// @return 填充好头部的帧缓冲区
    __attribute__((always_inline))
    static constexpr std::array<uint8_t, 8> make_noarg_context(const Command command, const RegAddr reg_addr){
        std::array<uint8_t, 8> buf = {
            static_cast<uint8_t>(command),
            static_cast<uint8_t>(static_cast<uint16_t>(reg_addr) >> 8),
            static_cast<uint8_t>(static_cast<uint16_t>(reg_addr)),
            0x00,  // 对齐填充
            0x00, 0x00, 0x00, 0x00
        };

        return buf;
    }

    /// 生成读取指令帧
    /// @param command 读取命令（ReadReg16=0x4B 或 ReadReg32=0x43）
    /// @param reg_addr 寄存器地址
    static constexpr std::array<uint8_t, 8> make_read_context(const Command command, const RegAddr reg_addr){
        return make_noarg_context(command, reg_addr);
    }

    /// 生成写入指令帧
    /// @param command 写入命令（WriteReg16=0x2B 或 WriteReg32=0x23）
    /// @param reg_addr 寄存器地址
    /// @param arg 参数值（会根据命令类型自动分解为高低字节）
    static constexpr std::array<uint8_t, 8> make_write_context(
        const Command command,
        const RegAddr reg_addr,
        const uint32_t arg
    ){
        auto buf = make_noarg_context(command, reg_addr);

        switch(command){
            case Command::WriteReg16:
                buf[4] = static_cast<uint8_t>(arg >> 8);
                buf[5] = static_cast<uint8_t>(arg);
                break;
            case Command::WriteReg32:
                buf[4] = static_cast<uint8_t>(arg >> 24);
                buf[5] = static_cast<uint8_t>(arg >> 16);
                buf[6] = static_cast<uint8_t>(arg >> 8);
                buf[7] = static_cast<uint8_t>(arg);
                break;
            default:
                __builtin_unreachable();
        }
        return buf;
    }

    /// 生成16位写入指令帧
    /// 自动处理1字节和2字节类型的转换
    template<typename T>
    requires (sizeof(T) <= 2)
    static constexpr std::array<uint8_t, 8> make_write16_context(
        const RegAddr reg_addr, const T code
    ){
        if constexpr(sizeof(T) == 1){
            return make_write_context(Command::WriteReg16, reg_addr, static_cast<uint16_t>(code));
        }else{
            return make_write_context(Command::WriteReg16, reg_addr, std::bit_cast<uint16_t>(code));
        }
    }

    /// 生成32位写入指令帧
    /// 自动处理4字节强类型码的位转换
    template<typename T>
    requires (sizeof(T) == 4)
    static constexpr std::array<uint8_t, 8> make_write32_context(
        const RegAddr reg_addr, const T code
    ){
        return make_write_context(Command::WriteReg32, reg_addr, std::bit_cast<uint32_t>(code));
    }
};
}