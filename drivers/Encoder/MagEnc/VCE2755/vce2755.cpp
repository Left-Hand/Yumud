#include "vce2755_prelude.hpp"

#include "vce2755.hpp"
#include "core/math/real.hpp"
#include "core/math/realmath.hpp"

using namespace ymd;
using namespace ymd::drivers;


using Error = VCE2755::Error;

template<typename T = void>
using IResult = Result<T, Error>;

using RegSet = VCE2755_Regset;
using Packet = RegSet::Packet;

IResult<> VCE2755::update(){
    // std::array<uint8_t, 3> bytes;
    if(const auto res = read_burst(Packet::ADDRESS, std::span(regs_.packet_.bytes));
    // if(const auto res = read_burst(Packet::ADDRESS, std::span(bytes));
        res.is_err()) return Err(res.unwrap_err());
    lap_turns_ = regs_.packet_.parse().unwrap().to_turns();
    // DEBUG_PRINTLN(bytes);
    return Ok();
}

IResult<> VCE2755::init(const VCE2755::Config & cfg){
    return Ok();
}


IResult<VCE2755::PackageCode> VCE2755::get_package_code(){
    RegSet::R8_ChipId chip_id_reg;
    if(const auto res = read_reg(chip_id_reg);
        res.is_err()) return Err(res.unwrap_err());
    return Ok(chip_id_reg.code);
}