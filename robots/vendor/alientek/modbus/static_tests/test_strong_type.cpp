
#include "../pdstepper_modbus_api_facade.hpp"
#include "../pdstepper_modbus_backend.hpp"

using namespace ymd;
using namespace ymd::robots::pdstepper;

namespace{



static constexpr float mwb = std::bit_cast<float>(uint32_t(0x40970A3D)) ;
static_assert(mwb == 4.72);

}