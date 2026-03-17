#include "lt8920.hpp"
#include "core/debug/debug.hpp"

using namespace ymd::drivers;
using namespace ymd;

#define LT8920_DEBUG_EN

#ifdef LT8920_DEBUG_EN

#define LT8920_TODO(...) TODO()
#define LT8920_DEBUG(...) DEBUG_PRINTLN(__VA_ARGS__);
#define LT8920_PANIC(...) PANIC{__VA_ARGS__}
#define LT8920_ASSERT(cond, ...) ASSERT{cond, ##__VA_ARGS__}


#define CHECK_RES(x, ...) ({\
    const auto __res_check_res = (x);\
    ASSERT{__res_check_res.is_ok(), ##__VA_ARGS__};\
    __res_check_res;\
})\


#define CHECK_ERR(x, ...) ({\
    const auto && __err_check_err = (x);\
    ASSERT{false, #x, ##__VA_ARGS__};\
    __err_check_err;\
})\

#else
#define LT8920_DEBUG(...)
#define LT8920_TODO(...) PANIC_NSRC()
#define LT8920_PANIC(...)  PANIC_NSRC()
#define LT8920_ASSERT(cond, ...) ASSERT_NSRC(cond)

#define CHECK_RES(x, ...) (x)
#define CHECK_ERR(x, ...) (x)
#endif


// #define CHANGE_STATE(x) state_ = x; LT8920_DEBUG("state_ = ", (uint8_t)state_)
#define CHANGE_STATE(x) state_ = x;

#define QUERY_CRCERR_FLAG regs_.flag_reg.crc_error_flag
#define QUERY_PKT_FLAG regs_.flag_reg.pkt_flag
#define FIFO_FLAG flag_reg.fifo_flag


using Error = LT8920::Error;

template<typename T = void>
using IResult = Result<T, Error>;

using R16_Flag = LT8920_Regset::R16_Flag;

IResult<> LT8920_SpiTransport::write_reg(FlagReg & flag_reg, const uint8_t reg_addr, const uint16_t reg_val){
    if(const auto res = 
        spi_drv_.transceive_single(
            reinterpret_cast<uint8_t &>(flag_reg), 
            uint8_t(reg_addr), CONT);
        res.is_err()) return Err(res.unwrap_err());
    delayT3();

    if(const auto res = 
        spi_drv_.write_single<uint16_t>(reg_val);
        res.is_err()) return Err(res.unwrap_err());
    return Ok();
}

IResult<> LT8920_SpiTransport::read_reg(FlagReg & flag_reg, const uint8_t reg_addr, uint16_t & reg_val){
    if(const auto res = spi_drv_.transceive_single(
        (flag_reg.as_bytes_mut()[0]), 
        uint8_t(reg_addr | 0x80), CONT); 
    res.is_err()) return Err(res.unwrap_err());
    if(const auto res = spi_drv_.read_single<uint16_t>(reg_val);
        res.is_err()) return Err(res.unwrap_err());
    return Ok();
}


IResult<> LT8920::validate(){
    uint16_t reg;
    if(const auto res = read_reg(30, reg);
        res.is_err()) return res;
    if(reg != 0xf413)
        return Err(Error::ChipIdMismatch);
    return Ok();
}

IResult<bool> LT8920::is_rf_synth_locked() {
    auto & reg = regs_.rf_synth_lock_reg;
    if(const auto res = read_reg((reg));
        res.is_err()) return Err(res.unwrap_err());
    return Ok(bool(reg.synth_locked));
}

IResult<uint8_t> LT8920::get_rssi() {
    auto & reg = regs_.raw_rssi_reg;
    if(const auto res = read_reg((reg));
        res.is_err()) return Err(res.unwrap_err());
    return Ok(uint8_t(reg.raw_rssi));
}

IResult<> LT8920::set_rf_channel(const uint8_t ch) {
    auto reg = RegCopy(regs_.rf_config_reg);
    reg.rf_channel_no = ch;
    return write_reg(reg);
}

IResult<> LT8920::set_rf_freq_mhz(const uint32_t freq) {
    // Implementation for setRfFreqMHz
    return set_rf_channel(freq - 2402);
}

IResult<> LT8920::set_role(const Role role) {
    auto reg = RegCopy(regs_.rf_config_reg);
    switch(role){
        case Role::IDLE:
            reg.rx_en = false;
            reg.tx_en = false;
            break;
        case Role::BROADCASTER:
            reg.rx_en = false;
            reg.tx_en = true;
            break;
        case Role::LISTENER:
            reg.tx_en = false;
            reg.rx_en = true;
            break;
    }

    role_ = role;
    return write_reg(reg);
}

IResult<> LT8920::set_pa_current(const uint8_t current) {
    auto reg = RegCopy(regs_.pa_config_reg);
    reg.pa_current = current;
    return write_reg((reg));
}

IResult<> LT8920::set_pa_gain(const uint8_t gain) {
    auto reg = RegCopy(regs_.pa_config_reg);
    reg.pa_gain = gain;
    return write_reg((reg));
}

IResult<> LT8920::enable_rssi(const uint16_t open) {
    auto reg = RegCopy(regs_.rssi_pdn_reg);
    reg.rssi_pdn = open;
    return write_reg((reg));
}

IResult<> LT8920::enable_auto_cali(const uint16_t open) {
    auto reg = RegCopy(regs_.auto_cali_reg);
    reg.auto_cali = open;
    return write_reg((reg));
}

IResult<> LT8920::set_brclk_sel(const BrclkSel brclkSel) {
    auto reg = RegCopy(regs_.config1_reg);
    reg.brclk_sel = (uint16_t)brclkSel;
    return write_reg((reg));
}

IResult<> LT8920::clear_fifo_write_ptr() {
    auto reg = RegCopy(regs_.fifo_ptr_reg);
    reg.clear_write_ptr = 1;
    return write_reg(reg);
}

IResult<> LT8920::clear_fifo_read_ptr() {
    auto reg = RegCopy(regs_.fifo_ptr_reg);
    reg.clear_read_ptr = 1;
    return write_reg((reg));
}

IResult<> LT8920::clear_fifo_ptr() {
    auto reg = RegCopy(regs_.fifo_ptr_reg);
    reg.clear_read_ptr = 1;
    reg.clear_write_ptr = 1;
    return write_reg(reg);
}

IResult<> LT8920::set_sync_word_bitsgth(const SyncWordBits len) {
    auto reg = RegCopy(regs_.config1_reg);
    reg.sync_word_len = std::bit_cast<uint8_t>(len);
    return write_reg((reg));
}

IResult<> LT8920::set_retrans_time(const uint8_t times) {
    auto reg = RegCopy(regs_.config2_reg);
    reg.retrans_times = times - 1;
    return write_reg((reg));
}

IResult<> LT8920::enable_auto_ack(const Enable en) {
    auto reg = RegCopy(regs_.config3_reg);
    reg.auto_ack = (en == EN);
    return write_reg((reg));
}

IResult<> LT8920::enable_crc(const Enable en){
    auto reg = RegCopy(regs_.config3_reg);
    reg.crc_en = (en == EN);
    return write_reg((reg));
}

IResult<> LT8920::set_err_bits_tolerance(uint8_t errbits){
    errbits = std::min<uint8_t>(errbits, 6);
    auto reg = RegCopy(regs_.threshold_reg);
    reg.errbits = errbits + 1;
    return write_reg(reg);
}

IResult<bool> LT8920::received_ack(){
    if(auto_ack_en_){
        auto & reg = regs_.fifo_ptr_reg;
        if(const auto res = read_reg(reg);
            res.is_err()) return Err(res.unwrap_err());
        return Ok(reg.fifo_read_ptr == 0);
    }else{
        return Ok(false);
    }
}

IResult<> LT8920::set_datarate(const DataRate dr){
    auto reg =  RegCopy(regs_.datarate_reg);
    reg.dataRate = (uint16_t)dr;
    return write_reg(reg);
}

IResult<> LT8920::write_block(const std::span<const uint8_t> bytes){

    if(state_ != State::IDLE) return Ok();
    if(role_ == Role::LISTENER) return Ok();

    const uint8_t len = static_cast<uint8_t>(std::min(bytes.size(), 32u));
    if(len == 0) return Ok();


    CHANGE_STATE(State::TX_PKT);
    
    if(const auto res = set_role(Role::IDLE);
        res.is_err()) return res;
    if(const auto res = clear_fifo_ptr();
        res.is_err()) return res;

    {
        if(first_as_len_en_){
            //如果使能则第一个字节为数据长度
            if(const auto res = write_fifo(std::span(&len, 1));
                res.is_err()) return res;
        }

        if(const auto res = write_fifo(bytes);
            res.is_err()) return res;
    }

    if(const auto res = set_role(Role::BROADCASTER);
        res.is_err()) return res;
    CHANGE_STATE(State::TX_WAIT_ACK);

    return Ok();
}

IResult<> LT8920::tick(){
    if(const auto res = update_fifo_status();
        res.is_err()) return res;
    switch(role_){
        case Role::BROADCASTER:
            switch(state_){
                case State::TX_WAIT_ACK:
                    if(QUERY_PKT_FLAG == true){
                        CHANGE_STATE(State::IDLE);
                    }
                    break;
                case State::IDLE:
                    break;
                    // CHANGE_STATE(State::tx)
                default:
                    break;
            }
            break;

        case Role::LISTENER:
            switch(state_){
                case State::IDLE:
                    CHANGE_STATE(State::RX_WAIT_ACK)
                    break;
                
                case State::RX_WAIT_ACK:
                    if(QUERY_PKT_FLAG == true){
                            //pass
                    }
                    break;
                default:
                    break;
            }
            break;
        default:
            break;
    }

    return Ok();
}

IResult<> LT8920::read_block(std::span<uint8_t> bytes){

    if(role_ != Role::LISTENER) return Ok();
    
    if(QUERY_PKT_FLAG == false) return Ok();
    if(QUERY_CRCERR_FLAG == true) return Ok();

    uint8_t len = static_cast<uint8_t>(std::min(bytes.size(), 32u));
    if(len == 0) return Ok();
    if(first_as_len_en_){
        if(const auto res = read_fifo(std::span(&len, 1));
            res.is_err()) return res;
    }

    if(const auto res = read_fifo(std::span(bytes.begin(), len));
        res.is_err()) return res;
    if(const auto res = clear_fifo_ptr();
        res.is_err()) return res;

    if(const auto res = set_role(Role::LISTENER);
        res.is_err()) return res;
    CHANGE_STATE(State::RX_WAIT_ACK);

    return Ok();
}

IResult<> LT8920::reset(){
    if(nrst_gpio.is_some()){
        auto & gpio = nrst_gpio.unwrap();
        gpio.outpp(LOW);
        clock::delay(20ms);
        gpio.set_high();
        clock::delay(20ms);
    }
    return Ok();
}

IResult<> LT8920::init(){
    if(const auto res = validate();
        res.is_err()) return res;

    if(const auto res = reset();
        res.is_err()) return res;

    regs_.rf_config_reg.__resv__ = 0;
    if(const auto res = set_role(Role::IDLE);
        res.is_err()) return res;
    if(const auto res = set_rf_channel(0);
        res.is_err()) return res;

    regs_.fifo_ptr_reg.as_bits_mut() = 0;

    // clock::delay(5ms);
    // setBrclkSel(BrclkSel::Mhz12);
    // clock::delay(5ms);
    // enableRssi();
    // enableAutoAck();
    // setSyncWordBitsgth(SyncWordBits::_32);


    if(const auto res = write_reg(0, 0x6fe0);    //masked
        res.is_err()) return Err(res.unwrap_err());
    if(const auto res = write_reg(1, 0x5681);    //masked
        res.is_err()) return Err(res.unwrap_err());
    if(const auto res = write_reg(2, 0x6617);    //masked
        res.is_err()) return Err(res.unwrap_err());

    if(const auto res = write_reg(4, 0x9cc9);    //masked
        res.is_err()) return Err(res.unwrap_err());
    if(const auto res = write_reg(5, 0x6637);    //masked
        res.is_err()) return Err(res.unwrap_err());

    if(const auto res = write_reg(8, 0x6c90);    //masked
        res.is_err()) return Err(res.unwrap_err());
    if(const auto res = write_reg(9, 0x4840);    //power (default 71af) UNDOCUMENTED
        res.is_err()) return Err(res.unwrap_err());

    if(const auto res = write_reg(10, 0x7ffd);   //bit 0: XTAL OSC enable
        res.is_err()) return Err(res.unwrap_err());
    if(const auto res = write_reg(11, 0x0008);   //bit 8: Power down RSSI (0=  RSSI operates normal)
        res.is_err()) return Err(res.unwrap_err());
    if(const auto res = write_reg(12, 0x0000);
        res.is_err()) return Err(res.unwrap_err());
    if(const auto res = write_reg(13, 0x48bd);   //(default 4855)
        res.is_err()) return Err(res.unwrap_err());

    if(const auto res = write_reg(22, 0x00ff);
        res.is_err()) return Err(res.unwrap_err());
    if(const auto res = write_reg(23, 0x8005);  //bit 2: Calibrate VCO before each Rx/Tx enable
        res.is_err()) return Err(res.unwrap_err());
    if(const auto res = write_reg(24, 0x0067);
        res.is_err()) return Err(res.unwrap_err());
    if(const auto res = write_reg(25, 0x1659);
        res.is_err()) return Err(res.unwrap_err());
    if(const auto res = write_reg(26, 0x19e0);
        res.is_err()) return Err(res.unwrap_err());
    if(const auto res = write_reg(27, 0x1300);  //bits 5:0, Crystal Frequency adjust
        res.is_err()) return Err(res.unwrap_err());
    if(const auto res = write_reg(28, 0x1800);
        res.is_err()) return Err(res.unwrap_err());

    if(const auto res = write_reg(32, 0x4800);
        res.is_err()) return Err(res.unwrap_err());
    //0x5000 = 0101 0000 0000 0000 = preamble 010 (3 bytes), B 10 (48 bits)
    if(const auto res = write_reg(33, 0x3fc7);
        res.is_err()) return Err(res.unwrap_err());
    if(const auto res = write_reg(34, 0x2000);  //
        res.is_err()) return Err(res.unwrap_err());
    if(const auto res = write_reg(35, 0x0300);  //POWER mode,  bit 8/9 on = retransmit = 3x (default)
        res.is_err()) return Err(res.unwrap_err());

    if(const auto res = set_sync_word(0x0380'5a5a'0380'0380);
        res.is_err()) return Err(res.unwrap_err());
    if(const auto res = set_err_bits_tolerance(1);
        res.is_err()) return Err(res.unwrap_err());

    // enable_auto_ack();
    // enable_crc();

    if(const auto res = write_reg(41, 0xb800);
        res.is_err()) return Err(res.unwrap_err());
    if(const auto res = write_reg(42, 0xfdb0);
        res.is_err()) return Err(res.unwrap_err());
    if(const auto res = write_reg(43, 0x000f);
        res.is_err()) return Err(res.unwrap_err());
    if(const auto res = write_reg(44, 0x0400);
        res.is_err()) return Err(res.unwrap_err());
    if(const auto res = write_reg(45, 0x0552);
        res.is_err()) return Err(res.unwrap_err());
    if(const auto res = write_reg(52, 0x8080);
        res.is_err()) return Err(res.unwrap_err());
    if(const auto res = write_reg(50, 0x0000);
        res.is_err()) return Err(res.unwrap_err());

    CHANGE_STATE(State::IDLE);

    return Ok();
}

IResult<> LT8920::set_sync_word(const uint64_t syncword){
    static constexpr size_t NUM_WORDS = 4u;
    const auto words = std::bit_cast<std::array<uint16_t, NUM_WORDS>>(syncword);
    auto & sync_word_regs = regs_.sync_word_regs;
    for(size_t i = 0; i < NUM_WORDS; i++){
        auto reg = RegCopy(sync_word_regs[i]);
        reg.bits = words[i];
        if(const auto res = write_reg(
            R16_SYNCWORD_BASE_ADDR + i, 
            reg.bits
        ); res.is_err()) return res;
        reg.apply();
    }

    return Ok();
}



IResult<> LT8920::write_fifo(std::span<const uint8_t> bytes){
    return transport_.write_fifo(bytes);

}

IResult<> LT8920::read_fifo(std::span<uint8_t> bytes){
    return transport_.read_fifo(bytes);

}

IResult<> LT8920::update_fifo_status(){
    return transport_.update_fifo_status(regs_.flag_reg);
}

IResult<bool> LT8920::get_fifo_status(){
    if(fifo_status_gpio.is_some()){
        return Ok(fifo_status_gpio.unwrap().read().to_bool());
    }else{
        if(const auto res = update_fifo_status();
            res.is_err()) return Err(res.unwrap_err());
        return Ok(bool(regs_.flag_reg.fifo_flag));
    }
}

IResult<bool> LT8920::get_pkt_status(){
    if(pkt_status_gpio.is_some()){
        return Ok(pkt_status_gpio.unwrap().read().to_bool());
    }else{
        if(const auto res = update_fifo_status();
            res.is_err()) return Err(res.unwrap_err());
        return Ok(bool(regs_.flag_reg.pkt_flag));
    }
}
