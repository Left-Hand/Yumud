#pragma once

#define USE_MOCK_SERVO


auto & SERVO_PWMGEN_TIMER = hal::timer3;
auto & DBG_UART = hal::uart2;
static constexpr auto CTRL_FREQ = 50;
