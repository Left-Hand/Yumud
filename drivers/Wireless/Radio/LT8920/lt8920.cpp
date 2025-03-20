#include "lt8920.hpp"
#include "sys/debug/debug.hpp"

using namespace ymd::drivers;
using namespace ymd;


#ifdef LT8920_DEBUG
#undef LT8920_DEBUG
#define LT8920_DEBUG(...) DEBUG_PRINTLN(__VA_ARGS__);
#define LT8920_PANIC(...) PANIC{__VA_ARGS__}
#define LT8920_ASSERT(cond, ...) ASSERT{cond, ##__VA_ARGS__}
#define READ_REG(reg) read_reg(reg.address, reg).loc().expect();
#define WRITE_REG(reg) write_reg(reg.address, reg).loc().expect();
#else
#define LT8920_DEBUG(...)
#define LT8920_PANIC(...)  PANIC_NSRC()
#define LT8920_ASSERT(cond, ...) ASSERT_NSRC(cond)
#define READ_REG(reg) (void)read_reg(reg.address, reg).unwrap();
#define WRITE_REG(reg) (void)write_reg(reg.address, reg).unwrap();
#endif


// #define LT8920_REG_DEBUG(...) LT8920_DEBUG(__VA_ARGS__)
#define LT8920_REG_DEBUG(...)

// #define CHANGE_STATE(x) state = x; LT8920_DEBUG("state = ", (uint8_t)state)
#define CHANGE_STATE(x) state = x;

#define CRCERR_FLAG flag_reg.crcErrorFlag
#define PKT_FLAG flag_reg.pktFlag
#define FIFO_FLAG flag_reg.fifoFlag

bool LT8920::verify(){
    uint16_t reg;
    read_reg(30, reg).unwrap();
    return (reg == 0xf413);
}

uint16_t LT8920::isRfSynthLocked() {
    read_reg(rf_synth_lock_reg.address, (rf_synth_lock_reg));
    return rf_synth_lock_reg.synthLocked;
}

uint8_t LT8920::getRssi() {
    read_reg(raw_rssi_reg.address, (raw_rssi_reg));
    return raw_rssi_reg.rawRssi;
}

void LT8920::setRfChannel(const uint8_t ch) {
    rf_config_reg.rfChannelNo = ch;
    write_reg(rf_config_reg.address, (rf_config_reg));
}

void LT8920::setRfFreqMHz(const uint freq) {
    // Implementation for setRfFreqMHz
    setRfChannel(freq - 2402);
}

void LT8920::setRole(const Role _role) {
    switch(_role){
        case Role::IDLE:
            rf_config_reg.rxEn = false;
            rf_config_reg.txEn = false;
            break;
        case Role::BROADCASTER:
            rf_config_reg.rxEn = false;
            rf_config_reg.txEn = true;
            break;
        case Role::LISTENER:
            rf_config_reg.txEn = false;
            rf_config_reg.rxEn = true;
            break;
    }

    role = _role;
    WRITE_REG(rf_config_reg);
}

void LT8920::setPaCurrent(const uint8_t current) {
    pa_config_reg.paCurrent = current;
    write_reg(pa_config_reg.address, (pa_config_reg));
}

void LT8920::setPaGain(const uint8_t gain) {
    pa_config_reg.paGain = gain;
    write_reg(pa_config_reg.address, (pa_config_reg));
}

void LT8920::enableRssi(const uint16_t open) {
    rssi_pdn_reg.rssiPdn = open;
    write_reg(rssi_pdn_reg.address, (rssi_pdn_reg));
}

void LT8920::enableAutoCali(const uint16_t open) {
    auto_cali_reg.autoCali = open;
    write_reg(auto_cali_reg.address, (auto_cali_reg));
}

void LT8920::setBrclkSel(const BrclkSel brclkSel) {
    config1_reg.brclkSel = (uint16_t)brclkSel;
    write_reg(config1_reg.address, (config1_reg));
}

void LT8920::clearFifoWritePtr() {
    fifo_ptr_reg.clearWritePtr = 1;
    write_reg(fifo_ptr_reg.address, (fifo_ptr_reg));
    fifo_ptr_reg.clearWritePtr = 0;
}

void LT8920::clearFifoReadPtr() {
    fifo_ptr_reg.clearReadPtr = 1;
    write_reg(fifo_ptr_reg.address, (fifo_ptr_reg));
    fifo_ptr_reg.clearReadPtr = 0;
}

void LT8920::clearFifoPtr() {
    fifo_ptr_reg.clearReadPtr = 1;
    fifo_ptr_reg.clearWritePtr = 1;
    WRITE_REG(fifo_ptr_reg);
    fifo_ptr_reg.clearReadPtr = 0;
    fifo_ptr_reg.clearWritePtr = 0;
}

void LT8920::setSyncWordBitsgth(const SyncWordBits len) {
    config1_reg.syncWordLen = (uint16_t)len;
    write_reg(config1_reg.address, (config1_reg));
}

void LT8920::setRetransTime(const uint8_t times) {
    config2_reg.retransTimes = times - 1;
    write_reg(config2_reg.address, (config2_reg));
}

void LT8920::enableAutoAck(const bool en) {
    config3_reg.autoAck = en;
    write_reg(config3_reg.address, (config3_reg));
}

void LT8920::enableCrc(const bool en){
    config3_reg.crcEn = en;
    write_reg(config3_reg.address, (config3_reg));
}

void LT8920::setErrBitsTolerance(uint8_t errbits){
    errbits = MIN(errbits, 6);
    threshold_reg.errbits = errbits + 1;
    WRITE_REG(threshold_reg);
}

bool LT8920::receivedAck(){
    if(auto_ack_en){
        READ_REG(fifo_ptr_reg);
        return fifo_ptr_reg.fifoReadPtr == 0;
    }else{
        return false;
    }
}

void LT8920::setDataRate(const DataRate dr){
    data_rate_reg.dataRate = (uint16_t)dr;
    WRITE_REG(data_rate_reg);
}

void LT8920::setDataRate(const uint32_t dr){
    switch(dr){
        default:
            LT8920_PANIC("unknown data rate, default to 62.5kbps");
            [[fallthrough]];
        case 62500:
            setDataRate(DataRate::Kbps62_5);
            break;
        case 125000:
            setDataRate(DataRate::Kbps125);
            break;
        case 250000:
            setDataRate(DataRate::Kbps250);
            break;
        case 1000000:
            setDataRate(DataRate::Mbps1);
            break;
    }
}

void LT8920::writeBlock(const uint8_t *data, uint8_t len){
    if(state != State::IDLE) return;
    if(role == Role::LISTENER) return;
    if(len == 0) return;

    len = MIN(len, 32);

    CHANGE_STATE(State::TX_PKT);
    
    setRole(Role::IDLE);
    clearFifoPtr();

    {
        if(first_as_len_en){
            //如果使能则第一个字节为数据长度
            writeFifo(&len, 1);
        }

        writeFifo(data, len);
    }

    setRole(Role::BROADCASTER);
    CHANGE_STATE(State::TX_WAIT_ACK);
}

void LT8920::tick(){
    updateFifoStatus();
    switch(role){
        case Role::BROADCASTER:
            switch(state){
                case State::TX_WAIT_ACK:
                    if(PKT_FLAG == true){
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
            switch(state){
                case State::IDLE:
                    CHANGE_STATE(State::RX_WAIT_ACK)
                    break;
                
                case State::RX_WAIT_ACK:
                    if(PKT_FLAG == true){
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
}

void LT8920::readBlock(uint8_t * data, uint8_t len){
    if(len == 0) return;
    if(role != Role::LISTENER) return;
    
    if(PKT_FLAG == false) return;
    if(CRCERR_FLAG == true) return;

    len = MIN(len, 32);

    if(first_as_len_en){
        readFifo(&len, 1);
    }

    readFifo(data, len);
    clearFifoPtr();

    setRole(Role::LISTENER);
    CHANGE_STATE(State::RX_WAIT_ACK);
}

void LT8920::reset(){
    if(nrst_gpio){
        nrst_gpio->outpp(LOW);
        delay(20);
        nrst_gpio->set();
        delay(20);
    }
}

void LT8920::init(){
    reset();

    rf_config_reg.__resv__ = 0;
    setRole(Role::IDLE);
    setRfChannel(0);

    fifo_ptr_reg = 0;

    // delay(5);
    // setBrclkSel(BrclkSel::Mhz12);
    // delay(5);
    // enableRssi();
    // enableAutoAck();
    // setSyncWordBitsgth(SyncWordBits::_32);


    write_reg(0, 0x6fe0);    //masked
    write_reg(1, 0x5681);    //masked
    write_reg(2, 0x6617);    //masked

    write_reg(4, 0x9cc9);    //masked
    write_reg(5, 0x6637);    //masked

    write_reg(8, 0x6c90);    //masked
    write_reg(9, 0x4840);    //power (default 71af) UNDOCUMENTED

    write_reg(10, 0x7ffd);   //bit 0: XTAL OSC enable
    write_reg(11, 0x0008);   //bit 8: Power down RSSI (0=  RSSI operates normal)
    write_reg(12, 0x0000);
    write_reg(13, 0x48bd);   //(default 4855)

    write_reg(22, 0x00ff);
    write_reg(23, 0x8005);  //bit 2: Calibrate VCO before each Rx/Tx enable
    write_reg(24, 0x0067);
    write_reg(25, 0x1659);
    write_reg(26, 0x19e0);
    write_reg(27, 0x1300);  //bits 5:0, Crystal Frequency adjust
    write_reg(28, 0x1800);

    write_reg(32, 0x4800);
    //0x5000 = 0101 0000 0000 0000 = preamble 010 (3 bytes), B 10 (48 bits)
    write_reg(33, 0x3fc7);
    write_reg(34, 0x2000);  //
    write_reg(35, 0x0300);  //POWER mode,  bit 8/9 on = retransmit = 3x (default)

    setSyncWord(0x0380'5a5a'0380'0380);
    setErrBitsTolerance(1);

    // enableAutoAck();
    // enableCrc();

    write_reg(41, 0xb800);
    write_reg(42, 0xfdb0);
    write_reg(43, 0x000f);
    write_reg(44, 0x0400);
    write_reg(45, 0x0552);
    write_reg(52, 0x8080);
    write_reg(50, 0x0000);

    CHANGE_STATE(State::IDLE);
}

void LT8920::setSyncWord(const uint64_t syncword){
    uint16_t words[4] = {0};
    memcpy(words, &syncword, 8);
    for(uint8_t i = 0; i < 4; i++){
        sync_word_regs[i].data = words[i];
        write_reg(sync_word_regs[i].head_address + i, (sync_word_regs[i]));
    }
}


BusError LT8920::write_reg(const RegAddress address, const uint16_t reg){
    LT8920_REG_DEBUG("W", std::hex, reg, "at", std::dec, uint8_t(address));
    if(spi_drv_){
        spi_drv_->transferSingle(reinterpret_cast<uint8_t &>(flag_reg), uint8_t(address), CONT).unwrap();
        delayT3();

        return spi_drv_->writeSingle((reg));
    }else if(i2c_drv_){
        return i2c_drv_->write_reg(uint8_t(address), reg, MSB);
    }

    PANIC();
}

BusError LT8920::read_reg(const RegAddress address, uint16_t & reg){
    LT8920_REG_DEBUG("R", std::hex, reg, "at", std::dec, uint8_t(address));
    if(spi_drv_){
        spi_drv_->transferSingle(reinterpret_cast<uint8_t &>(flag_reg), uint8_t(address | 0x80), CONT).unwrap();
        return spi_drv_->readSingle(reg);
    }else if(i2c_drv_){
        return i2c_drv_->read_reg(uint8_t(address), reg, MSB);
    }

    PANIC();
}


BusError LT8920::writeFifo(const uint8_t * data, const size_t len){
    LT8920_REG_DEBUG("Wfifo", std::dec, len);
    if(spi_drv_){
        spi_drv_->writeSingle(uint8_t(50), CONT).unwrap();
        return spi_drv_->write_burst<uint8_t>(data, len);
    }else if(i2c_drv_){
        return i2c_drv_->write_burst(uint8_t(50) , std::span(data, len));
    }

    PANIC();
}

BusError LT8920::readFifo(uint8_t * data, const size_t len){
    LT8920_REG_DEBUG("Rfifo", std::dec, len);
    if(spi_drv_){
        spi_drv_->writeSingle(uint8_t(50 | 0x80), CONT).unwrap();
        return spi_drv_->read_burst(data, len);
    }else if(i2c_drv_){
        return i2c_drv_->read_burst(uint8_t(50), std::span(data, len));
    }

    PANIC();
}

BusError LT8920::updateFifoStatus(){
    if(spi_drv_){
        // return spi_drv_->transferSingle(flag_reg.as_ref(), flag_reg.address);
    } else if(i2c_drv_){
        return i2c_drv_->read_reg(flag_reg.address, reinterpret_cast<uint8_t &>(flag_reg));
    }

    PANIC();
}

bool LT8920::getFifoStatus(){
    if(fifo_status_gpio){
        return fifo_status_gpio->read();
    }else{
        updateFifoStatus();
        return flag_reg.fifoFlag;
    }
}

bool LT8920::getPktStatus(){
    if(pkt_status_gpio){
        return pkt_status_gpio->read();
    }else{
        updateFifoStatus();
        return flag_reg.pktFlag;
    }
}
