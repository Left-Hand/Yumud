#include "ina3221.hpp"

using namespace ymd;
using namespace ymd::drivers;


using Self = INA3221;
template<typename T = void>
using IResult = Self::IResult<T>;

using Error = Self::Error;
using Regs = INA3221_Regs;



static_assert(Self::ShuntVoltCode::from_mv(-80).bits == 0xc180);
static_assert(Self::ShuntVoltCode(0xc180).to_mv() == -80);
static_assert(std::abs((float)Self::ShuntVoltCode(0xc180).to_volts() - (-0.08)) < 1E-4);


// static_assert(Self::mv_to_svsum_code(-80) == 0xc180);
// static_assert(Self::svsum_code_to_mv(0xc180) == -80);

static_assert(Self::BusVoltCode::from_mv(32760).bits == 0x7ff8);
static_assert(Self::BusVoltCode(0x7ff8).to_mv() == 32760);

// static constexpr auto f = (float)Self::BusVoltCode(0x7ff8).to_volts();
static_assert(std::abs((float)Self::BusVoltCode(0x7ff8).to_volts() - (32.76)) < 1E-4);
// static_assert( == 32760);


