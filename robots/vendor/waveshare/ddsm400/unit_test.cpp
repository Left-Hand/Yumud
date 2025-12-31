#include "ddsm400_transport.hpp"
#include "ddsm400_msgs.hpp"

using namespace ymd::robots::waveshare::ddsm400;
using namespace primitive;
using namespace ymd;


// 开环指令
// 01 64 F8 30 00 00 00 00 00 08 （-2000）
// 01 64 EC 78 00 00 00 00 00 D3 （－5000）
// 01 64 D8 F0 00 00 00 00 00 78 （－10000）
// 01 64 00 00 00 00 00 00 00 50 （0）
// 01 64 07 D0 00 00 00 00 00 27 （2000）
// 01 64 13 88 00 00 00 00 00 A7 （5000）
// 01 64 27 10 00 00 00 00 00 57 （10000）

// 电流环指令
// 给定值范围：－32767～32767 对应量程－4A～4A
// 01 64 F8 30 00 00 00 00 00 08 （－2000）
// 01 64 EC 78 00 00 00 00 00 D3 （－5000）
// 01 64 D8 F0 00 00 00 00 00 78 （－10000）
// 01 64 00 00 00 00 00 00 00 50 （0）
// 01 64 07 D0 00 00 00 00 00 27 （2000）
// 01 64 13 88 00 00 00 00 00 A7 （5000）
// 01 64 27 10 00 00 00 00 00 57 （10000）

// 速度环指令
// 给定值范围：-380~380 rpm
// 01 64 FE 0C 00 00 00 00 00 16 （－50rpm）
// 01 64 FC 18 00 00 00 00 00 E8 （－100rpm）
// 01 64 00 00 00 00 00 00 00 50 （0rpm）
// 01 64 01 F4 00 00 00 00 00 C3 （50rpm）
// 01 64 03 E8 00 00 00 00 00 9F （100rpm）

// 刹车指令
// 速度环模式下有效：
// 01 64 00 00 00 00 00 FF 00 D1 （电机 1）
// 02 64 00 00 00 00 00 FF 00 24 （电机 2）

// 获取电机反馈
// 获取模式反馈
// 01 75 00 00 00 00 00 00 00 47 （电机1）
// 02 75 00 00 00 00 00 00 00 B2 （电机2）

// 获取其它反馈
// 01 74 00 00 00 00 00 00 00 04 （电机 1）
// 02 74 00 00 00 00 00 00 00 F1 （电机 2）



static_assert(sizeof(SpeedCode) == 2);
static_assert(SpeedCode::try_from_rpm(10).unwrap().bits == 100);
static_assert(SpeedCode::try_from_rpm(-10).unwrap().bits == -100);
static_assert(CurrentCode::try_from_amps(-2).unwrap().bits == -0x4000);
static_assert(CurrentCode::try_from_amps(-4).unwrap().bits == -0x8000);
static_assert(CurrentCode::try_from_amps(2).unwrap().bits == 0x4000);
static_assert(LapAngleCode::from_angle(Angular<uq32>::from_turns(0.25_uq32)).bits == 0x4000);
void test1(){
    static constexpr auto motor_id = MotorId::from_u8(1);
    // 电机模式切换
    // 01 A0 00 00 00 00 00 00 00 9E （开环）
    // 01 A0 01 00 00 00 00 00 00 A3 （电流环）
    // 01 A0 02 00 00 00 00 00 00 E4 （速度环）
    // 01 A0 08 00 00 00 00 00 00 E4 （电机使能）
    // 01 A0 09 00 00 00 00 00 00 E4 （电机失能）
    // 若电机 ID 改变，CRC8 值需要重新计算，指令也有变化
    
    {
        static constexpr auto bytes = transport::serialize_request(motor_id, req_msgs::SetLoopMode{
            .loop_mode = LoopMode::OpenLoop
        });
        static_assert(bytes[0] == 0x01);
        static_assert(bytes[1] == 0xA0);
        static_assert(bytes[2] == 0x00);
        static_assert(bytes.back() == 0x9e);
    }
    
    {
        static constexpr auto bytes = transport::serialize_request(motor_id, req_msgs::SetLoopMode{
            .loop_mode = LoopMode::CurrentLoop
        });
        static_assert(bytes[0] == 0x01);
        static_assert(bytes[1] == 0xA0);
        static_assert(bytes[2] == 0x01);
        static_assert(bytes.back() == 0xA3);
    }
    
    {
        static constexpr auto bytes = transport::serialize_request(motor_id, req_msgs::SetLoopMode{
            .loop_mode = LoopMode::SpeedLoop
        });
        static_assert(bytes[0] == 0x01);
        static_assert(bytes[1] == 0xA0);
        static_assert(bytes[2] == 0x02);
        static_assert(bytes.back() == 0xE4);
    }
    
    {
        static constexpr auto bytes = transport::serialize_request(motor_id, req_msgs::SetLoopMode{
            .loop_mode = LoopMode::Activate
        });
        static_assert(bytes[0] == 0x01);
        static_assert(bytes[1] == 0xA0);
        static_assert(bytes[2] == 0x08);
        // !doubt
        // static_assert(bytes.back() == 0xE4);
    }
    
    {
        static constexpr auto bytes = transport::serialize_request(motor_id, req_msgs::SetLoopMode{
            .loop_mode = LoopMode::Deactivate
        });
        static_assert(bytes[0] == 0x01);
        static_assert(bytes[1] == 0xA0);
        static_assert(bytes[2] == 0x09);
        // !doubt
        // static_assert(bytes.back() == 0xE4);
    }
}

// 开环/电流环指令测试
void test_open_and_current_loop(){
    static constexpr auto motor_id = MotorId::from_u8(1);
    
    // -2000
    {
        static constexpr auto bytes = transport::serialize_request(motor_id, req_msgs::SetTarget{
            .setpoint_code = SetPointCode{.bits = -2000},
            .acceleration_time_code = AccelerationTimeCode{.bits = 0},
            .brake_en = DISEN
        });
        static_assert(bytes[0] == 0x01);
        static_assert(bytes[1] == 0x64);
        static_assert(bytes[2] == 0xF8);
        static_assert(bytes[3] == 0x30);
        static_assert(bytes.back() == 0x08);
    }
    
    // -5000
    {
        static constexpr auto bytes = transport::serialize_request(motor_id, req_msgs::SetTarget{
            .setpoint_code = SetPointCode{.bits = -5000},
            .acceleration_time_code = AccelerationTimeCode{.bits = 0},
            .brake_en = DISEN
        });
        static_assert(bytes[0] == 0x01);
        static_assert(bytes[1] == 0x64);
        static_assert(bytes[2] == 0xEC);
        static_assert(bytes[3] == 0x78);
        static_assert(bytes.back() == 0xD3);
    }
    
    // -10000
    {
        static constexpr auto bytes = transport::serialize_request(motor_id, req_msgs::SetTarget{
            .setpoint_code = SetPointCode{.bits = -10000},
            .acceleration_time_code = AccelerationTimeCode{.bits = 0},
            .brake_en = DISEN
        });
        static_assert(bytes[0] == 0x01);
        static_assert(bytes[1] == 0x64);
        static_assert(bytes[2] == 0xD8);
        static_assert(bytes[3] == 0xF0);
        static_assert(bytes.back() == 0x78);
    }
    
    // 0
    {
        static constexpr auto bytes = transport::serialize_request(motor_id, req_msgs::SetTarget{
            .setpoint_code = SetPointCode{.bits = 0},
            .acceleration_time_code = AccelerationTimeCode{.bits = 0},
            .brake_en = DISEN
        });
        static_assert(bytes[0] == 0x01);
        static_assert(bytes[1] == 0x64);
        static_assert(bytes[2] == 0x00);
        static_assert(bytes[3] == 0x00);
        static_assert(bytes.back() == 0x50);
    }
    
    // 2000
    {
        static constexpr auto bytes = transport::serialize_request(motor_id, req_msgs::SetTarget{
            .setpoint_code = SetPointCode{.bits = 2000},
            .acceleration_time_code = AccelerationTimeCode{.bits = 0},
            .brake_en = DISEN
        });
        static_assert(bytes[0] == 0x01);
        static_assert(bytes[1] == 0x64);
        static_assert(bytes[2] == 0x07);
        static_assert(bytes[3] == 0xD0);
        static_assert(bytes.back() == 0x27);
    }
    
    // 5000
    {
        static constexpr auto bytes = transport::serialize_request(motor_id, req_msgs::SetTarget{
            .setpoint_code = SetPointCode{.bits = 5000},
            .acceleration_time_code = AccelerationTimeCode{.bits = 0},
            .brake_en = DISEN
        });
        static_assert(bytes[0] == 0x01);
        static_assert(bytes[1] == 0x64);
        static_assert(bytes[2] == 0x13);
        static_assert(bytes[3] == 0x88);
        static_assert(bytes.back() == 0xA7);
    }
    
    // 10000
    {
        static constexpr auto bytes = transport::serialize_request(motor_id, req_msgs::SetTarget{
            .setpoint_code = SetPointCode{.bits = 10000},
            .acceleration_time_code = AccelerationTimeCode{.bits = 0},
            .brake_en = DISEN
        });
        static_assert(bytes[0] == 0x01);
        static_assert(bytes[1] == 0x64);
        static_assert(bytes[2] == 0x27);
        static_assert(bytes[3] == 0x10);
        static_assert(bytes.back() == 0x57);
    }
}

// 速度环指令测试
void test_speed_loop(){
    static constexpr auto motor_id = MotorId::from_u8(1);
    
    // -50rpm
    {
        static constexpr auto speed_code = SpeedCode::try_from_rpm(-50).unwrap();
        static constexpr auto bytes = transport::serialize_request(motor_id, req_msgs::SetTarget{
            .setpoint_code = SetPointCode{.bits = speed_code.bits},
            .acceleration_time_code = AccelerationTimeCode{.bits = 0},
            .brake_en = DISEN
        });
        static_assert(bytes[0] == 0x01);
        static_assert(bytes[1] == 0x64);
        static_assert(bytes[2] == 0xFE);
        static_assert(bytes[3] == 0x0C);
        static_assert(bytes.back() == 0x16);
    }
    
    // -100rpm
    {
        static constexpr auto speed_code = SpeedCode::try_from_rpm(-100).unwrap();
        static constexpr auto bytes = transport::serialize_request(motor_id, req_msgs::SetTarget{
            .setpoint_code = SetPointCode{.bits = speed_code.bits},
            .acceleration_time_code = AccelerationTimeCode{.bits = 0},
            .brake_en = DISEN
        });
        static_assert(bytes[0] == 0x01);
        static_assert(bytes[1] == 0x64);
        static_assert(bytes[2] == 0xFC);
        static_assert(bytes[3] == 0x18);

        //! doubt
        // static_assert(bytes.back() == 0xE8);
    }
    
    // 0rpm
    {
        static constexpr auto speed_code = SpeedCode::try_from_rpm(0).unwrap();
        static constexpr auto bytes = transport::serialize_request(motor_id, req_msgs::SetTarget{
            .setpoint_code = SetPointCode{.bits = speed_code.bits},
            .acceleration_time_code = AccelerationTimeCode{.bits = 0},
            .brake_en = DISEN
        });
        static_assert(bytes[0] == 0x01);
        static_assert(bytes[1] == 0x64);
        static_assert(bytes[2] == 0x00);
        static_assert(bytes[3] == 0x00);
        static_assert(bytes.back() == 0x50);
    }
    
    // 50rpm
    {
        static constexpr auto speed_code = SpeedCode::try_from_rpm(50).unwrap();
        static constexpr auto bytes = transport::serialize_request(motor_id, req_msgs::SetTarget{
            .setpoint_code = SetPointCode{.bits = speed_code.bits},
            .acceleration_time_code = AccelerationTimeCode{.bits = 0},
            .brake_en = DISEN
        });
        static_assert(bytes[0] == 0x01);
        static_assert(bytes[1] == 0x64);
        static_assert(bytes[2] == 0x01);
        static_assert(bytes[3] == 0xF4);
        static_assert(bytes.back() == 0xC3);
    }
    
    // 100rpm
    {
        static constexpr auto speed_code = SpeedCode::try_from_rpm(100).unwrap();
        static constexpr auto bytes = transport::serialize_request(motor_id, req_msgs::SetTarget{
            .setpoint_code = SetPointCode{.bits = speed_code.bits},
            .acceleration_time_code = AccelerationTimeCode{.bits = 0},
            .brake_en = DISEN
        });
        static_assert(bytes[0] == 0x01);
        static_assert(bytes[1] == 0x64);
        static_assert(bytes[2] == 0x03);
        static_assert(bytes[3] == 0xE8);
        static_assert(bytes.back() == 0x9F);
    }
}

// 刹车指令测试
void test_brake(){
    static constexpr auto motor_id = MotorId::from_u8(1);
    
    {
        static constexpr auto bytes = transport::serialize_request(motor_id, req_msgs::SetTarget{
            .setpoint_code = SetPointCode{.bits = 0},
            .acceleration_time_code = AccelerationTimeCode{.bits = 0},
            .brake_en = EN
        });
        static_assert(bytes[0] == 0x01);
        static_assert(bytes[1] == 0x64);
        static_assert(bytes[2] == 0x00);
        static_assert(bytes[3] == 0x00);
        static_assert(bytes[7] == 0xFF);
        static_assert(bytes.back() == 0xD1);
    }
}

// 获取电机反馈测试
void test_get_feedback(){
    static constexpr auto motor_id = MotorId::from_u8(1);
    
    // 获取模式反馈
    {
        static constexpr auto bytes = transport::serialize_request(motor_id, req_msgs::GetLoopMode{});
        static_assert(bytes[0] == 0x01);
        static_assert(bytes[1] == 0x75);
        static_assert(bytes.back() == 0x47);
    }
    
    // 获取其它反馈
    {
        static constexpr auto bytes = transport::serialize_request(motor_id, req_msgs::GetJourney{});
        static_assert(bytes[0] == 0x01);
        static_assert(bytes[1] == 0x74);
        static_assert(bytes.back() == 0x04);
    }
}