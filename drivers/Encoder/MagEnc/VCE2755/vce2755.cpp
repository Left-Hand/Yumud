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
using AnglePacket = RegSet::AnglePacket;

// 对应的 CRC 生成多项式为 X4+X+1，初始值=0000b，数据输入输出不取反。
static constexpr uint8_t calc_crc4(uint32_t bits20) {
    // 确保只使用低20位
    bits20 &= 0x000FFFFF;
    
    // CRC 寄存器，初始值为 0000 (4位)
    uint8_t crc = 0x00;
    
    // 处理20个数据位，从最高位开始
    for (int i = 19; i >= 0; i--) {
        // 获取当前数据位
        bool data_bit = (bits20 >> i) & 0x01;
        
        // 计算反馈位：CRC最高位(bit3)与数据位异或
        bool feedback = ((crc >> 3) & 0x01) ^ data_bit;
        
        // CRC左移1位
        crc = (crc << 1) & 0x0F;
        
        // 如果反馈位为1，则与多项式异或
        if (feedback) {
            // 多项式 X^4 + X + 1 对应的二进制: 10011
            // 去掉最高位后为: 0011 (0x03)
            crc ^= 0x03;
        }
    }
    
    return crc & 0x0F;  // 确保只返回低4位
}
static_assert(calc_crc4(0x12345) == 0x08);
static_assert(calc_crc4(0x00) == 0x00);
static_assert(calc_crc4(0x00001) == 0x03);



IResult<> VCE2755::update(){
    // std::array<uint8_t, 3> bytes;
    if(const auto res = read_bulk(AnglePacket::BASE_ADDR, std::span(regs_.packet_.bytes));
    // if(const auto res = read_bulk(AnglePacket::REG_ADDR, std::span(bytes));
        res.is_err()) return Err(res.unwrap_err());
    lap_turns_ = regs_.packet_.parse().unwrap().to_turns();
    // DEBUG_PRINTLN(bytes);
    return Ok();
}

IResult<> VCE2755::init(const VCE2755::Config & cfg){
    (void)(cfg);
    return Ok();
}


IResult<VCE2755::PackageCode> VCE2755::get_package_code(){
    RegSet::R8_ChipId chip_id_reg;
    if(const auto res = read_reg(chip_id_reg);
        res.is_err()) return Err(res.unwrap_err());
    return Ok(chip_id_reg.code);
}