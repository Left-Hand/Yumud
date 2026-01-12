#include "mt6835.hpp"



using namespace ymd::drivers;
using namespace ymd;

using Self = MT6835;
using Error = Self::Error;

template<typename T = void>
using IResult = Result<T, Error>;


[[nodiscard]] static constexpr 
std::array<uint8_t, 3> make_req(
    const Self::Command cmd, 
    const Self::RegAddr reg_addr, 
    const uint8_t b
){
    const uint16_t high_b = (static_cast<uint16_t>(cmd) << 12) | static_cast<uint16_t>(reg_addr);
    return {
        static_cast<uint8_t>(high_b >> 8),
        static_cast<uint8_t>(high_b & 0xff),
        b
    };
}

IResult<> Self::init() {
    TODO();
    return Ok();
}


IResult<> Self::update() {
    TODO();
    return Ok();
}




IResult<> Self::write_reg(const RegAddr reg_addr, const uint8_t reg_val){

    const auto req = make_req(Command::WriteReg, reg_addr, reg_val);
    std::array<uint8_t, 3> resp;

    if(const auto res = transceive_3b(std::span(resp), std::span(req));
        res.is_err()) return Err(res.unwrap_err());

    return Ok();
}

IResult<> Self::read_reg(const Self::RegAddr reg_addr, uint8_t & reg_val){

    const auto req = make_req(Command::ReadReg, reg_addr, reg_val);
    std::array<uint8_t, 3> resp;

    if(const auto res = transceive_3b(std::span(resp), std::span(req));
        res.is_err()) return Err(res.unwrap_err());

    //最后一个字节返回值
    reg_val = resp.back();

    return Ok();
}

IResult<> Self::burn_eeprom(){
    static constexpr uint8_t CONFIRM_CODE_MAGIC_NUM = 0x55;
    const auto req = std::array<uint8_t, 3>{0b11000000, 0x00, 0x00};
    std::array<uint8_t, 3> resp;
    if(const auto res = transceive_3b(std::span(resp), std::span(req));
        res.is_err()) return Err(res.unwrap_err());

    const auto act_confirm_code = resp.back();
    if(act_confirm_code != CONFIRM_CODE_MAGIC_NUM) 
        return Err(Error::EEpromProgramFailed);

    return Ok();
}

IResult<> Self::transceive_3b(std::span<uint8_t, 3> resp, std::span<const uint8_t, 3> req){
    if(const auto res = spi_drv_.transceive_burst<uint8_t>(std::span(resp), std::span(req));
        res.is_err()) return Err(res.unwrap_err());
    return Ok();
}

