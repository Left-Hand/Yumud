#include "src/testbench/tb.h"
#include "dsp/controller/l1/l1_adaptive.hpp"

#include "middlewares/repl/repl_server.hpp"
#include "robots/mock/mock_burshed_motor.hpp"

#include "hal/gpio/gpio_port.hpp"
#include "hal/bus/uart/hw_singleton.hpp"
#include "hal/timer/hw_singleton.hpp"

#include "core/clock/time.hpp"


using namespace ymd;
using namespace ymd::dsp;
using namespace ymd::dsp::l1;

static constexpr auto UART_BAUD = 576000;

class SingleLinkArm {
private:
    float length;
    float angle;
    float angular_velocity;
    float dt;

public:
    SingleLinkArm(float len, float timestep) 
        : length(len), angle(0.0), angular_velocity(0.0), dt(timestep) {}

    void update(float torque) {
        // 这里假设机械臂的转动惯量和阻尼系数为1
        float inertia = 10.0;
        float damping = 18.2;
        angular_velocity += (torque - damping * angular_velocity) / inertia * dt;
        angle += angular_velocity * dt;
    }

    float get_angle() const {
        return angle;
    }

    std::pair<float, float> get_position() const {
        float x = length * std::cos(angle);
        float y = length * std::sin(angle);
        return std::make_pair(x, y);
    }
};



void l1_adaptive_main(){
    auto init_debugger = []{
        auto & DBG_UART = hal::usart2;

        DBG_UART.init({
            .remap = hal::USART2_REMAP_PA2_PA3,
            .baudrate = hal::NearestFreq(UART_BAUD),
            .tx_strategy = CommStrategy::Blocking
        });

        DEBUGGER.retarget(&DBG_UART);
        DEBUGGER.set_eps(5);
        DEBUGGER.set_splitter(",");
        DEBUGGER.no_brackets(EN);
    };

    init_debugger();
    // 参数设置
    float dt = 0.001;  // 时间步长
    
    // 初始化控制器配置
    ymd::dsp::l1::L1AdaptiveController::Config config{
        .fs = static_cast<uint32_t>(1.0 / dt),  // 采样频率
        .param_am = 19.0,
        .param_b = 1.1,
        .param_tau = 0.001
    };
    
    // 初始化控制器
    ymd::dsp::l1::L1AdaptiveController controller(config);

    // 模拟参数
    float length = 1.0;  // 机械臂长度

    // 正弦轨迹参数
    float amplitude = 0.55;  // 正弦轨迹振幅
    float frequency = 0.2;   // 正弦轨迹频率

    SingleLinkArm arm(length, dt);

    float t = 0;

    while(true) {
        t += dt;
        
        // 计算期望角度
        float desired_angle = amplitude * std::sin(2.0 * M_PI * frequency * t);
        
        // 获取实际角度
        float actual_angle = arm.get_angle();
        
        float torque = controller.iterate(desired_angle, actual_angle);
        
        // 对扭矩进行限制，防止初始冲击
        const float max_torque = 100.0f;
        if(torque > max_torque) torque = max_torque;
        if(torque < -max_torque) torque = -max_torque;
        
        // 更新机械臂状态
        arm.update(torque);
        
        DEBUG_PRINTLN(desired_angle, actual_angle, torque, desired_angle - actual_angle);
        
    }

}