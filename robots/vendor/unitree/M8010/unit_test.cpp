#include "m8010_prelude.hpp"

using namespace ymd;
using namespace ymd::robots::unitree;


namespace {

static_assert(sizeof(KdCode) == 2);
static_assert(std::is_trivially_copyable_v<KdCode>);
static_assert(KdCode::try_from(iq15(0.1)).unwrap().bits == 128);


static_assert(sizeof(X2Code) == 2);
static_assert(std::is_trivially_copyable_v<X2Code>);
static_assert(X2Code::try_from_speed(Angular<iq16>::from_radians(iq16(M_PI))).unwrap().bits == 128);
static_assert(X2Code::try_from_speed(Angular<iq16>::from_radians(iq16(-M_PI))).unwrap().bits == -128);

static_assert(sizeof(TorqueCode) == 2);
static_assert(std::is_trivially_copyable_v<TorqueCode>);
static_assert(TorqueCode::try_from_nm(iq16(0.75)).unwrap().bits == 192);

static_assert(sizeof(X1Code) == 4);
static_assert(std::is_trivially_copyable_v<X1Code>);
static_assert(X1Code::try_from_turns(iq15(0.25)).unwrap().bits == 8192);

static_assert(sizeof(KpCode) == 2);
static_assert(std::is_trivially_copyable_v<KpCode>);
static_assert(KpCode::try_from(uq16(0.1)).unwrap().bits == 128);
static_assert(KpCode::try_from(uq16(1.0)).unwrap().bits == 1280);



}
