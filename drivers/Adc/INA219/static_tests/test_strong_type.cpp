#include "ina219.hpp"

using namespace ymd;
using namespace ymd::drivers;



using Self = INA219;
using Error = INA219::Error;

template<typename T = void>
using IResult = Result<T, Error>;



static_assert(sizeof(Self::BusVoltageCode) == sizeof(uint16_t));
static_assert(std::abs((float)Self::BusVoltageCode(0x0fa0).to_volts() - 16.0f) < 1E-4);


using ShuntVoltCode = Self::ShuntVoltCode;

static_assert(sizeof(ShuntVoltCode) == sizeof(int16_t));

static_assert(ShuntVoltCode::from_mv(320).bits == 0x7d00);
static_assert(ShuntVoltCode::from_mv(160).bits == 0x3e80);
static_assert(ShuntVoltCode::from_mv(80).bits == 0x1f40);
static_assert(ShuntVoltCode::from_mv(40).bits == 0x0fa0);

static_assert((uint16_t)ShuntVoltCode::from_mv(-320).bits == 0x8300);
static_assert((uint16_t)ShuntVoltCode::from_mv(-160).bits == 0xc180);
static_assert((uint16_t)ShuntVoltCode::from_mv(-80).bits == 0xe0c0);
static_assert((uint16_t)ShuntVoltCode::from_mv(-40).bits == 0xf060);