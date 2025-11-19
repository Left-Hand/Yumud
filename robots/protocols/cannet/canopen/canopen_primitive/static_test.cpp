#include "canopen_primitive.hpp"
#include "canopen_sdo_primitive.hpp"
#include "canopen_nmt_primitive.hpp"

using namespace ymd::canopen;
using namespace ymd::canopen::primitive;

static_assert(sizeof(CobId) == sizeof(uint16_t));
static_assert(PdoOnlyFunctionCode::from_bits(0).is_none());
static_assert(PdoOnlyFunctionCode::from_bits(4).is_some());

[[maybe_unused]] static void test1(){
    static constexpr auto layout1 = SdoExpeditedMsg::from_exception(
        OdPreIndex::from_bits(0),
        OdSubIndex::from_bits(0)
    );

    static_assert(layout1.header.as_bits() == 0x80'0000'00);
}

