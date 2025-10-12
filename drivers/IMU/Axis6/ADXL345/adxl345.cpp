#include "adxl345.hpp"
// #include "core/debug/debug.hpp"

using namespace ymd::drivers;

using namespace ymd;
using namespace ymd::drivers;


using Error = ADXL345::Error;
template<typename T = void>
using IResult = Result<T, Error>;


IResult<> ADXL345::validate(){
    auto & reg = regs_.deviceid_reg;
    if(const auto res = read_reg(reg); 
        res.is_err()) return Err(res.unwrap_err());

    if(reg.id != VALID_DEVICE_ID)
        return Err(Error::WrongWhoAmI);

    return Ok();
}

IResult<Vec3<q24>> ADXL345::read_acc(){
    std::array<int16_t, 3> buf;
    if(const auto res = phy_.read_burst(std::bit_cast<uint8_t>(RegAddr::DeviceID), buf);
        res.is_err()) return Err(res.unwrap_err());

    
    auto conv = [&](const int16_t data) -> q24{
        return data * acc_scaler_;    };

    return Ok(Vec3<q24>(
        conv(buf[0]),
        conv(buf[1]),
        conv(buf[2])
    ));
}

IResult<> ADXL345::write_reg(const RegAddr reg_address, const uint8_t reg_data){
    return phy_.write_reg(std::bit_cast<uint8_t>(reg_address), reg_data);
}

IResult<> ADXL345::read_reg(const RegAddr reg_address, uint8_t & reg_data){
    return phy_.read_reg(std::bit_cast<uint8_t>(reg_address), reg_data);
}