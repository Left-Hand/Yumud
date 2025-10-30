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
        return Err(Error::InvalidChipId);

    if(const auto res = self_test(); 
        res.is_err()) return Err(res.unwrap_err());

    return Ok();
}

IResult<> ADXL345::self_test(){
    // ADXL345具备自测功能，可同时有效测试机械系统和电子
    // 系统。自测功能使能时(通过DATA_FORMAT寄存器(地址
    // 0x31)的SELF_TEST位)，有静电力施加于机械传感器之
    // 上。与加速度同样的方式，静电力驱使力敏传感元件移
    // 动，且有助于器件体验加速度。增加的静电力导致x、y和z
    // 轴上的输出变化。因为静电力与VS^2成正比，所以输出随着VS
    // 而变化。该效应如图43所示。表14所示的比例因子可用
    // 来为不同的电源电压VS调整预期的自测输出限值。
    // ADXL345的自测功能也表现为双模行为。然而，由于双峰
    // 性，表1和表15至表18所示的限值对潜在的自检值都有
    // 效。在低于100 Hz或在1600 Hz的数据速率下，使用自测功
    // 能，可能产生超出这些限值的值。因此，器件必须为正常
    // 功率运行(BW_RATE寄存器的(地址0x2C) LOW_POWER位
    // = 0)，通过800 Hz或3200 Hz，置于100Hz的数据速率，以便
    // 自测功能正常运行。

    TODO();
    return Ok();
}

IResult<Vec3<iq24>> ADXL345::read_acc(){
    std::array<int16_t, 3> buf;
    if(const auto res = phy_.read_burst(std::bit_cast<uint8_t>(RegAddr::DeviceID), buf);
        res.is_err()) return Err(res.unwrap_err());

    
    auto conv = [&](const int16_t data) -> iq24{
        return data * acc_scaler_;    };

    return Ok(Vec3<iq24>(
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