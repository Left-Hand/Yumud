#include <cstdint>

#include "mavlink_packed_code.hpp"
#include "core/string/view/string_view.hpp"
#include "core/string/view/mut_string_view.hpp"
#include "core/utils/Result.hpp"

#include "mavlink_reflect.hpp"
#include <memory>

using namespace ymd;
using namespace ymd::mavlink;




namespace{


using math::fp32;

MAVLINK_CODEGEN_DEFMSG(
    Heartbeat, 0,
    MAVLINK_CODEGEN_PROXY(type,              MavType),
    MAVLINK_CODEGEN_PROXY(autopilot,         MavAutopilot),
    MAVLINK_CODEGEN_PROXY(base_mode,         MavModeFlag),
    MAVLINK_CODEGEN_PROXY(custom_mode,       uint32_t),
    MAVLINK_CODEGEN_PROXY(system_status,     MavState )
)


// SYS_STATUS (1)
// 系统核心状态（传感器、电池、负载、通信）。
MAVLINK_CODEGEN_DEFMSG(
    SysStatus, 1,

    // 传感器在位掩码（0=不存在，1=存在）
    MAVLINK_CODEGEN_PROXY(onboard_control_sensors_present, MavSysStatusSensor),
    
    // 传感器使能掩码（0=关闭，1=开启）
    MAVLINK_CODEGEN_PROXY(onboard_control_sensors_enabled, MavSysStatusSensor),

    // 传感器健康掩码（0=故障，1=正常）
    MAVLINK_CODEGEN_PROXY(onboard_control_sensors_health, MavSysStatusSensor),

    // 主循环最大占用率（0–1000，建议<1000）
    MAVLINK_CODEGEN_PROXY(load, LoadPercents),

    // 电池电压
    MAVLINK_CODEGEN_PROXY(voltage_battery, VoltageCode),

    // 电池电流
    MAVLINK_CODEGEN_PROXY(current_battery, CurrentCode),

    // 剩余电量百分比
    MAVLINK_CODEGEN_PROXY(battery_remaining, int8_t),

    // 通信丢包率（UART/I2C/SPI/CAN）
    MAVLINK_CODEGEN_PROXY(drop_rate_comm, uint16_t),

    // 通信错误数
    MAVLINK_CODEGEN_PROXY(errors_comm, uint16_t),

    // 飞控自定义错误码
    MAVLINK_CODEGEN_PROXY(errors_count, uint16_t[4]),

    // 扩展传感器在位掩码
    MAVLINK_CODEGEN_PROXY(onboard_control_sensors_present_extended, uint32_t),

    // 扩展传感器使能掩码
    MAVLINK_CODEGEN_PROXY(onboard_control_sensors_enabled_extended, uint32_t),

    // 扩展传感器健康掩码
    MAVLINK_CODEGEN_PROXY(onboard_control_sensors_health_extended, uint32_t)

)


[[maybe_unused]] static void test_pg(){
    {
        static_assert(std::tuple_size_v<std::array<uint8_t, 39>> == 39);
        static_assert(SysStatus_Facade<std::array<uint8_t, 39>>::BYTES_SIZE == 39);
        static constexpr auto it = []{
            SysStatus_Facade<std::array<uint8_t, 39>> temp{};
            temp.onboard_control_sensors_present().set([]{
                auto s = MavSysStatusSensor{0};
                s.sensor_3d_gyro().set(1);
                s.sensor_gps().set(1);
                return s;
            }());


            temp.load().set(LoadPercents{42});

            temp.errors_count()[0].set(0x12);
            temp.errors_count()[1].set(0x34);
            temp.errors_count()[2].set(0x56);
            temp.errors_count()[3].set(0x78);
            return temp;
        }();

        constexpr auto onboard_control_sensors_present = it.onboard_control_sensors_present().get();


        static_assert(onboard_control_sensors_present.sensor_3d_gyro().get() == true);
        static_assert(onboard_control_sensors_present.sensor_absolute_pressure().get() == false);
        static_assert(onboard_control_sensors_present.sensor_gps().get() == true);

        static_assert(it.load().get().bits == 42);
        static_assert(it.errors_count()[0].get() == 0x12);
        static_assert(it.errors_count()[1].get() == 0x34);
        static_assert(it.errors_count()[2].get() == 0x56);
        static_assert(it.errors_count()[3].get() == 0x78);
    }

    {
        static constexpr std::array<uint8_t, 8> buf = {
            static_cast<uint8_t>(MavType::Adsb),
            static_cast<uint8_t>(MavAutopilot::Argentum),
            0x11,
            0x12,0x34,0x56,0x78,
            static_cast<uint8_t>(MavState::Active)
        };


        static constexpr auto it = Heartbeat_Facade<std::span<const uint8_t, 8>>
            {std::span<const uint8_t, 8>(buf)};

        static_assert(it.type().get() == MavType::Adsb);
        static_assert(it.autopilot().get() == MavAutopilot::Argentum);
        static_assert(it.base_mode().get().armed().get() == true);
        static_assert(it.base_mode().get().guided_enabled().get() == true);
        static_assert(it.custom_mode().get() == 0x78563412);
        static_assert(it.system_status().get() == MavState::Active);
    }

    {
        static constexpr auto it = []{
            Heartbeat_Facade<std::array<uint8_t, 8>> temp{};
            temp.type().set(MavType::Adsb);
            temp.autopilot().set(MavAutopilot::Argentum);
            temp.custom_mode().set(0x78563412);
            temp.system_status().set(MavState::Active);
            return temp;
        }();

        static_assert(it.type().get() == MavType::Adsb);
        static_assert(it.autopilot().get() == MavAutopilot::Argentum);
        static_assert(it.custom_mode().get() == 0x78563412);
        static_assert(it.system_status().get() == MavState::Active);
    }
}




MAVLINK_CODEGEN_DEFMSG(
    ParamSet, 23,
    MAVLINK_CODEGEN_PROXY(target_system,         uint8_t),
    MAVLINK_CODEGEN_PROXY(target_component,      uint8_t),
    MAVLINK_CODEGEN_PROXY(param_id,              char[16]),
    MAVLINK_CODEGEN_PROXY(param_value,           fp32),
    MAVLINK_CODEGEN_PROXY(param_type,            MavParamType)
)


[[maybe_unused]] static void test_mavmsgs_serde(){


    {
        static_assert(ParamSet_Facade<std::array<uint8_t, 23>>::BYTES_SIZE == 23);
        static constexpr auto it = []{
            ParamSet_Facade<std::array<uint8_t, 23>> temp{};
            temp.target_system().set(0x14);
            temp.target_component().set(0x51);
            temp.param_value().set(1.73782444e+34f);
            temp.param_id()[0] = 'H';
            temp.param_id()[1] = 'e';
            temp.param_id()[2] = 'l';
            temp.param_id()[3] = 'l';
            temp.param_id()[4] = 'o';
            temp.param_id()[5] = '\0';
            return temp;
        }();

        static_assert(it.target_system().get() == 0x14);
        static_assert(it.target_component().get() == 0x51);
        static_assert((float)it.param_value().get() == 1.73782444e+34f);
        static_assert(it.param_id().length() == 5);
        static_assert(it.param_id()[0] == 'H');
        static_assert(it.param_id()[4] == 'o');
    }

}

}