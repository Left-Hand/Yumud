#include "lt8920.hpp"


#define REG16(x) (*(uint16_t *)(&x))
#define REG8(x) (*(uint8_t *)(&x))

#define WRITE_REG16(reg) writeReg(reg.address, REG16(reg));
#define READ_REG16(reg) readReg(reg.address, REG16(reg));



// #define LT8920_REG_DEBUG(...) LT8920_DEBUG(__VA_ARGS__)
#define LT8920_REG_DEBUG(...)

// #define CHANGE_STATE(x) state = x; LT8920_DEBUG("state = ", (uint8_t)state)
#define CHANGE_STATE(x) state = x;

#define CRCERR_FLAG flag_reg.crcErrorFlag
#define PKT_FLAG flag_reg.pktFlag
#define FIFO_FLAG flag_reg.fifoFlag

#pragma GCC diagnostic ignored "-Wstrict-aliasing"

bool LT8920::verify(){
    uint16_t reg;
    readReg(30, reg);
    return (reg == 0xf413);
}

uint16_t LT8920::isRfSynthLocked() {
    readReg(rf_synth_lock_reg.address, REG16(rf_synth_lock_reg));
    return rf_synth_lock_reg.synthLocked;
}

uint8_t LT8920::getRssi() {
    readReg(raw_rssi_reg.address, REG16(raw_rssi_reg));
    return raw_rssi_reg.rawRssi;
}

void LT8920::setRfChannel(const uint8_t ch) {
    rf_config_reg.rfChannelNo = ch;
    writeReg(rf_config_reg.address, REG16(rf_config_reg));
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
    WRITE_REG16(rf_config_reg);
}

void LT8920::setPaCurrent(const uint8_t current) {
    pa_config_reg.paCurrent = current;
    writeReg(pa_config_reg.address, REG16(pa_config_reg));
}

void LT8920::setPaGain(const uint8_t gain) {
    pa_config_reg.paGain = gain;
    writeReg(pa_config_reg.address, REG16(pa_config_reg));
}

void LT8920::enableRssi(const uint16_t open) {
    rssi_pdn_reg.rssiPdn = open;
    writeReg(rssi_pdn_reg.address, REG16(rssi_pdn_reg));
}

void LT8920::enableAutoCali(const uint16_t open) {
    auto_cali_reg.autoCali = open;
    writeReg(auto_cali_reg.address, REG16(auto_cali_reg));
}

void LT8920::setBrclkSel(const BrclkSel brclkSel) {
    config1_reg.brclkSel = (uint16_t)brclkSel;
    writeReg(config1_reg.address, REG16(config1_reg));
}

void LT8920::clearFifoWritePtr() {
    fifo_ptr_reg.clearWritePtr = 1;
    writeReg(fifo_ptr_reg.address, REG16(fifo_ptr_reg));
    fifo_ptr_reg.clearWritePtr = 0;
}

void LT8920::clearFifoReadPtr() {
    fifo_ptr_reg.clearReadPtr = 1;
    writeReg(fifo_ptr_reg.address, REG16(fifo_ptr_reg));
    fifo_ptr_reg.clearReadPtr = 0;
}

void LT8920::clearFifoPtr() {
    fifo_ptr_reg.clearReadPtr = 1;
    fifo_ptr_reg.clearWritePtr = 1;
    WRITE_REG16(fifo_ptr_reg);
    fifo_ptr_reg.clearReadPtr = 0;
    fifo_ptr_reg.clearWritePtr = 0;
}

void LT8920::setSyncWordBitsgth(const SyncWordBits len) {
    config1_reg.syncWordLen = (uint16_t)len;
    writeReg(config1_reg.address, REG16(config1_reg));
}

void LT8920::setRetransTime(const uint8_t times) {
    config2_reg.retransTimes = times - 1;
    writeReg(config2_reg.address, REG16(config2_reg));
}

void LT8920::enableAutoAck(const bool en) {
    config3_reg.autoAck = en;
    writeReg(config3_reg.address, REG16(config3_reg));
}

void LT8920::enableCrc(const bool en){
    config3_reg.crcEn = en;
    writeReg(config3_reg.address, REG16(config3_reg));
}

void LT8920::setErrBitsTolerance(uint8_t errbits){
    errbits = MIN(errbits, 6);
    threshold_reg.errbits = errbits + 1;
    WRITE_REG16(threshold_reg);
}

bool LT8920::receivedAck(){
    if(auto_ack_en){
        READ_REG16(fifo_ptr_reg);
        return fifo_ptr_reg.fifoReadPtr == 0;
    }else{
        return false;
    }
}

void LT8920::setDataRate(const DataRate dr){
    data_rate_reg.dataRate = (uint16_t)dr;
    WRITE_REG16(data_rate_reg);
}

void LT8920::setDataRate(const uint32_t dr){
    switch(dr){
        default:
            LT8920_DEBUG("unknown data rate, default to 62.5kbps")
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
        nrst_gpio->outpp(0);
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

    REG16(fifo_ptr_reg) = 0;

    // delay(5);
    // setBrclkSel(BrclkSel::Mhz12);
    // delay(5);
    // enableRssi();
    // enableAutoAck();
    // setSyncWordBitsgth(SyncWordBits::_32);


    writeReg(0, 0x6fe0);    //masked
    writeReg(1, 0x5681);    //masked
    writeReg(2, 0x6617);    //masked

    writeReg(4, 0x9cc9);    //masked
    writeReg(5, 0x6637);    //masked

    writeReg(8, 0x6c90);    //masked
    writeReg(9, 0x4840);    //power (default 71af) UNDOCUMENTED

    writeReg(10, 0x7ffd);   //bit 0: XTAL OSC enable
    writeReg(11, 0x0008);   //bit 8: Power down RSSI (0=  RSSI operates normal)
    writeReg(12, 0x0000);
    writeReg(13, 0x48bd);   //(default 4855)

    writeReg(22, 0x00ff);
    writeReg(23, 0x8005);  //bit 2: Calibrate VCO before each Rx/Tx enable
    writeReg(24, 0x0067);
    writeReg(25, 0x1659);
    writeReg(26, 0x19e0);
    writeReg(27, 0x1300);  //bits 5:0, Crystal Frequency adjust
    writeReg(28, 0x1800);

    writeReg(32, 0x4800);
    //0x5000 = 0101 0000 0000 0000 = preamble 010 (3 bytes), B 10 (48 bits)
    writeReg(33, 0x3fc7);
    writeReg(34, 0x2000);  //
    writeReg(35, 0x0300);  //POWER mode,  bit 8/9 on = retransmit = 3x (default)

    setSyncWord(0x0380'5a5a'0380'0380);
    setErrBitsTolerance(1);

    // enableAutoAck();
    // enableCrc();

    writeReg(41, 0xb800);
    writeReg(42, 0xfdb0);
    writeReg(43, 0x000f);
    writeReg(44, 0x0400);
    writeReg(45, 0x0552);
    writeReg(52, 0x8080);
    writeReg(50, 0x0000);

    CHANGE_STATE(State::IDLE);
}

void LT8920::setSyncWord(const uint64_t syncword){
    uint16_t words[4] = {0};
    memcpy(words, &syncword, 8);
    for(uint8_t i = 0; i < 4; i++){
        sync_word_regs[i].data = words[i];
        writeReg(sync_word_regs[i].head_address + i, REG16(sync_word_regs[i]));
    }
}


void LT8920::writeReg(const RegAddress address, const uint16_t reg){
    LT8920_REG_DEBUG("W", std::hex, reg, "at", std::dec, (uint8_t)address);
    if(spi_drv){
        spi_drv->transfer(REG8(flag_reg), (uint8_t)address, false);
        delayT3();

        spi_drv->write(REG16(reg));
    }else if(i2c_drv){
        i2c_drv->writeReg((uint8_t)address, reg);
    }
}

void LT8920::readReg(const RegAddress address, uint16_t & reg){
    LT8920_REG_DEBUG("R", std::hex, reg, "at", std::dec, (uint8_t)address);
    if(spi_drv){
        spi_drv->transfer(REG8(flag_reg), uint8_t(address | 0x80), false);
        spi_drv->read(reg);
    }else if(i2c_drv){
        i2c_drv->readReg((uint8_t)address, reg);
    }
}


void LT8920::writeFifo(const uint8_t * data, const size_t len){
    LT8920_REG_DEBUG("Wfifo", std::dec, len);
    if(spi_drv){
        spi_drv->write(uint8_t(50), false);
        spi_drv->write(data, len);
    }else if(i2c_drv){
        i2c_drv->writePool(uint8_t(50) , data, len, LSB);
    }
}

void LT8920::readFifo(uint8_t * data, const size_t len){
    LT8920_REG_DEBUG("Rfifo", std::dec, len);
    if(spi_drv){
        spi_drv->write(uint8_t(50 | 0x80), false);
        spi_drv->read(data, len);
    }else if(i2c_drv){
        i2c_drv->readPool(uint8_t(50), data, len, LSB);
    }
}

void LT8920::updateFifoStatus(){
    if(spi_drv){
        // spi_drv->transfer(REG8(flag_reg), flag_reg.address);
        READ_REG16(flag_reg);
    } else if(i2c_drv){
        i2c_drv->readReg(flag_reg.address, REG8(flag_reg));
    }
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



#pragma GCC diagnostic pop