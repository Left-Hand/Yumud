#include "KTH7823.hpp"
#include "core/math/real.hpp"
#include "core/math/realmath.hpp"



#define KTH7823_DEBUG_EN

#ifdef KTH7823_DEBUG_EN
#define KTH7823_DEBUG(...) DEBUG_PRINTLN(__VA_ARGS__);
#define KTH7823_PANIC(...) PANIC(__VA_ARGS__)
#define KTH7823_ASSERT(cond, ...) ASSERT(cond, __VA_ARGS__)
#else
#define KTH7823_DEBUG(...)
#define KTH7823_PANIC(...)  PANIC_NSRC()
#define KTH7823_ASSERT(cond, ...) ASSERT_NSRC(cond)
#endif

using namespace ymd;
using namespace ymd::drivers::kth7823;

IResult<> Transport::direct_read(uint16_t & val){
    uint16_t rx_bits;
    if(const auto res = transceive_u16(rx_bits, 0);
        res.is_err()) return Err(res.unwrap_err());
    val = rx_bits;
    return Ok();
}

IResult<> Transport::read_reg(const uint8_t reg_addr, uint8_t & reg_val){
    const uint16_t tx_bits = ((reg_addr & 0b00'111111) | 0b01'000000) << 8;
    uint16_t rx_bits;
    const auto res = transceive_u16(rx_bits, tx_bits);
    if(res.is_err()) return Err(res.unwrap_err());
    reg_val = rx_bits >> 8;
    return Ok();
}

IResult<> Transport::burn_reg(const uint8_t reg_addr, const uint8_t reg_val){
    // SPI 烧写寄存器操作由两个 16 位的帧组成。第一个帧是写请求
    // 帧，其中包含一个 2 位的写命令（10），后跟一个 6 位的寄存器
    // 地址和一个 8 位的数值。写命令指示芯片执行写操作，寄存器地
    // 址用于指定要写入的寄存器，而数值则是要写入的数据。数据传
    // 输从最高位（MSB）开始。
    // 第二个帧是返回的确认帧，其中包含了新写入寄存器的值。帧格
    // 式为 XXXX − XXXX − 0000 − 0000。这个确认帧是芯片返回的
    // 响应，用于确认数据已成功写入寄存器。
    // 在进行 SPI 烧写寄存器操作时，需要特别注意在第一个帧和第二
    // 个帧之间等待至少 20 毫秒的时间。这段等待时间用于确保写入
    // 的数据能正确存储到芯片的非易失性存储器中。如果在写入请求
    // 后没有等待足够的时间，可能会读取到寄存器先前的值。因此，
    // 执行写入寄存器操作时务必遵守这个等待时间。
    
    {
        const uint16_t tx_bits = static_cast<uint16_t>(static_cast<uint16_t>((reg_addr & 0b00'111111) | 0b10'000000) << 8) | reg_val;
        if(const auto res = spi_drv_.write_single<uint16_t>(tx_bits, CONT);
            res.is_err()) return Err(res.unwrap_err());
    }

    //TODO use async
    clock::delay(20ms);


    uint16_t rx_bits;
    if(const auto res = spi_drv_.read_single<uint16_t>(rx_bits);
        res.is_err()) return Err(res.unwrap_err());
    
    if((rx_bits >> 8) != reg_val) 
        return Err(Error::RegProgramUncovered);
    
    if((rx_bits & 0xff) != 0) 
        return Err(Error::RegProgramResponseFormatInvalid);
    return Ok();
}

IResult<> Transport::disable_reg_oper(){
    uint16_t dummy;
    return transceive_u16(dummy, 0b1110'1000'0000'0010);
}

IResult<> Transport::enable_reg_oper(){
    uint16_t dummy;
    return transceive_u16(dummy, 0b1110'1000'0000'0000);
}

IResult<> Transport::transceive_u16(uint16_t & rx, const uint16_t tx){
    if(const auto res = spi_drv_.write_single<uint16_t>(tx, CONT);
        res.is_err()) return Err(res.unwrap_err());

    if(const auto res = spi_drv_.transceive_single<uint16_t>(rx, tx);
        res.is_err()) return Err(res.unwrap_err());

    return Ok();
}

IResult<> KTH7823::update(){
    uint16_t bits;
        
    if(const auto res = transport_.direct_read(bits); 
        res.is_err()) return Err(res.unwrap_err());


    lap_turns_ = lap_turns_.from_bits(static_cast<uint32_t>(bits) << 16);

    return Ok();
}

IResult<> KTH7823::validate(){
    return Ok();
}

IResult<> KTH7823::burn_zero_angle(const Angular<uq32> angle){
    const uint16_t b16 = static_cast<uint16_t>(angle.to_turns().to_bits() >> 16);

    auto reg_low = RegCopy(regset_.zero_low_reg);
    reg_low.bits = static_cast<uint8_t>(b16 & 0xff);

    auto reg_high = RegCopy(regset_.zero_high_reg);
    reg_high.bits = static_cast<uint8_t>(b16 >> 8);

    // return Ok();
    if(const auto res = burn_reg(reg_low); 
        res.is_err()) return Err(res.unwrap_err());
    if(const auto res = burn_reg(reg_high); 
        res.is_err()) return Err(res.unwrap_err());
    return Ok();
}


static constexpr uint8_t quantize_trim(const uq16 trim){
    const auto temp = math::round_cast<uint32_t>(trim * 256);
    // const auto temp = static_cast<uint32_t>(trim * uq16(258.0 / 256));
    if(temp > 255) return 255;
    return static_cast<uint8_t>(temp);
}

//keep mu >= 1
static constexpr uint8_t ratio_to_trim_quantized(const uq16 mu){
    const uq16 trim = 1 - 1/mu;
    return quantize_trim(trim);
}


//these tests are fitting from datasheet
static_assert(ratio_to_trim_quantized(1) == 0);
static_assert(ratio_to_trim_quantized(1.5_uq16) == 85);
static_assert(ratio_to_trim_quantized(2) == 128);
static_assert(ratio_to_trim_quantized(2.5_uq16) == 154);
static_assert(ratio_to_trim_quantized(3) == 171);
static_assert(ratio_to_trim_quantized(3.5_uq16) == 183);
static_assert(ratio_to_trim_quantized(4) == 192);
static_assert(ratio_to_trim_quantized(4.5_uq16) == 199);
static_assert(ratio_to_trim_quantized(5) == 205);


//mu effect to mag-encoder
// paste in desmos
// \ \arctan\left(\frac{c\sin\left(x\right)}{\cos\left(x\right)}\right)-\left(\operatorname{mod}\left(x+\frac{\pi}{2},\pi\right)-\frac{\pi}{2}\right)

IResult<> KTH7823::set_trim(const uq16 x, const uq16 y){
    uint8_t trim_bits = 0;

    //apply trim to weaker
    bool x_need_trim = false;
    bool y_need_trim = false;

    if(x > y){
        const uq16 trim = 1 - y/x;
        trim_bits = quantize_trim(trim);
        y_need_trim = false;
    }else{
        const uq16 trim = 1 - x/y;
        trim_bits = quantize_trim(trim);
        x_need_trim = true;
    }

    {
        auto reg = RegCopy(regset_.gain_trim_reg);
        reg.gain_trim = trim_bits;
        if(const auto res = burn_reg(reg);
            res.is_err()) return Err(res.unwrap_err());
    }

    {
        auto reg = RegCopy(regset_.xy_trim_reg);
        reg.x_trim = x_need_trim;
        reg.y_trim = y_need_trim;
        if(const auto res = burn_reg(reg);
            res.is_err()) return Err(res.unwrap_err());
    }

    return Ok();
}

IResult<> KTH7823::set_mag_threshold(const MagThreshold low, const MagThreshold high){
    auto reg = RegCopy(regset_.mag_alert_reg);
    reg.mag_low = low;
    reg.mag_high = high;

    return burn_reg(reg);
}

IResult<> KTH7823::set_direction(const RotateDirection direction){
    auto reg = RegCopy(regset_.rd_reg);
    reg.rd = direction == RotateDirection::CW;

    return burn_reg(reg);
}

IResult<> KTH7823::set_zero_parameters(const ZeroPulseWidth width, const ZeroPulsePhase phase){
    auto reg = RegCopy(regset_.z_config_reg);
    reg.zl = width;
    reg.zd = phase;

    return burn_reg(reg);
}

IResult<> KTH7823::set_abz_freq_limit(const AbzFreqLimit freq_lim){
    auto reg = RegCopy(regset_.abz_limit_reg);
    reg.abz_limit = freq_lim;
    return burn_reg(reg);
}


IResult<> KTH7823::set_pulse_per_turn(const uint16_t num_ppt){
    KTH7823_ASSERT(num_ppt < 1024, "num_ppt must be less than 1024");
    KTH7823_ASSERT(num_ppt != 0, "num_ppt must not be zero");
    
    const uint8_t ppt_low = num_ppt & 0x03;
    const uint8_t ppt_high = num_ppt >> 2;
    {
        auto reg = RegCopy(regset_.ppt_high_reg);
        reg.ppt_high = ppt_high;
        return burn_reg(reg);
    }

    {
        auto reg = RegCopy(regset_.z_config_reg);
        reg.ppt_low = ppt_low;
        return burn_reg(reg);
    }
    return Ok();
}