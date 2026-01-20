#include "mt6826s_prelude.hpp"

using namespace ymd;
using namespace ymd::drivers;

using Self = MT6826S;

using Error = Self::Error;

template<typename T = void>
using IResult = Result<T, Error>;


[[nodiscard]] static constexpr 
uint16_t make_header(const Self::Command cmd, const Self::RegAddr reg_addr){
    return (static_cast<uint16_t>(cmd) << 12) | static_cast<uint16_t>(reg_addr);
}


IResult<> Self::init(){
    TODO();
    return Ok();
}

IResult<Angular<uq32>> Self::get_lap_angle(){
    TODO();
    return Ok(Angular<uq32>::ZERO);
}

IResult<EncoderFaultBitFields> Self::get_fault(){
    EncoderFaultBitFields fault = EncoderFaultBitFields::zero();
    return Ok(fault);
}

