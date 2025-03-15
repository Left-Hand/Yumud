#pragma once

#include <bit>
#include <array>
#include <functional>
#include <memory>
#include <type_traits>
#include <optional>

#include "sys/io/regs.hpp"
#include "sys/math/fraction.hpp"


#include "hal/bus/i2c/i2cdrv.hpp"
#include "hal/bus/spi/spidrv.hpp"
#include "hal/bus/uart/uart.hpp"

#include "hal/timer/pwm/pwm_channel.hpp"
#include "hal/adc/analog_channel.hpp"


#define DEF_R16(T, name)\
static_assert(sizeof(T) == 2, "x must be 16bit register");\
static_assert(std::has_unique_object_representations_v<T>, "x must has unique bitfield");\
T name = {};\

#define DEF_R8(T, name)\
static_assert(sizeof(T) == 1, "x must be 16bit register");\
static_assert(std::has_unique_object_representations_v<T>, "x must has unique bitfield");\
T name = {};\

#define REG16I_QUICK_DEF(addr, type, name)\
struct type :public Reg16i<>{scexpr uint8_t address = addr; int16_t data;}; DEF_R16(type, name)

#define REG16_QUICK_DEF(addr, type, name)\
struct type :public Reg16<>{scexpr RegAddress address = addr; uint16_t data;}; DEF_R16(type, name)

#define REG8_QUICK_DEF(addr, type, name)\
struct type :public Reg16i<>{scexpr RegAddress address = addr; uint8_t data;}; DEF_R8(type, name)
