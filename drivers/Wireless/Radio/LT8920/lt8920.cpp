#include "lt8920.hpp"


#define REG16(x) (*(uint16_t *)(&x))
#define REG8(x) (*(uint8_t *)(&x))

#define WRITE_REG16(reg) writeReg(reg.address, REG16(reg));
#define READ_REG16(reg) readReg(reg.address, REG16(reg));

#pragma GCC diagnostic ignored "-Wstrict-aliasing"

void LT8920::verify(){
    READ_REG16(device_id_reg);
    LT8920_DEBUG("digi ver", uint8_t(device_id_reg.digiVersion));
    LT8920_DEBUG("RF ver", uint8_t(device_id_reg.rfVersion));
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
}

void LT8920::setRadioMode(const uint16_t isRx) {
    if (isRx) {
        rf_config_reg.txEn = false;
        rf_config_reg.rxEn = true;
    } else {
        rf_config_reg.rxEn = false;
        rf_config_reg.txEn = true;
    }
    writeReg(rf_config_reg.address, REG16(rf_config_reg));
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

uint8_t LT8920::getDigiVersion() {
    readReg(device_id_reg.address, REG16(device_id_reg));
    return device_id_reg.digiVersion;
}

uint8_t LT8920::getRfVersion() {
    readReg(device_id_reg.address, REG16(device_id_reg));
    return device_id_reg.rfVersion;
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

void LT8920::enableTx(bool en){
    rf_config_reg.txEn = en;
    WRITE_REG16(rf_config_reg);
}

void LT8920::enableRx(bool en){
    rf_config_reg.rxEn = en;
    WRITE_REG16(rf_config_reg);
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
// void LT8920::read(uint8_t *buffer, size_t maxBuffer){
//     uint16_t value = readRegister(R_STATUS);
//     if (bitRead(value, STATUS_CRC_BIT) == 0){
//         //CRC ok

//         uint16_t data = readRegister(R_FIFO);
//         uint8_t packetSize = data >> 8;
//         if(maxBuffer < packetSize+1)
//         {
//             //BUFFER TOO SMALL
//             return -2;
//         }

//         uint8_t pos;
//         buffer[pos++] = (data & 0xFF);
//         while (pos < packetSize)
//         {
//         data = readRegister(R_FIFO);
//         buffer[pos++] = data >> 8;
//         buffer[pos++] = data & 0xFF;
//         }

//         return packetSize;
//     }else{
//             //CRC error
//             return -1;
//     }
// }

// void LT8920::startListening()
// {
//   writeReg(R_CHANNEL, _channel & CHANNEL_MASK);   //turn off rx/tx
//   delay(3);
//   writeReg(R_FIFO_CONTROL, 0x0080);  //flush rx
//   writeReg(R_CHANNEL,  (_channel & CHANNEL_MASK) | _BV(CHANNEL_RX_BIT));   //enable RX
//   delay(5);
// }

void LT8920::setDataRate(const DataRate dr){
    data_rate_reg.dataRate = (uint16_t)dr;
    WRITE_REG16(data_rate_reg);
}

void LT8920::writeBlock(const uint8_t *data, uint8_t len){
    if(len == 0) return;
    len = MIN(len, 32);

    enableTx(false);
    clearFifoPtr();

    {
        if(first_as_len_en){
            //如果使能则第一个字节为数据长度
            writeFifo(&len, 1);
        }

        writeFifo(data, len);
    }

    enableTx(true);
}

void LT8920::readBlock(uint8_t * data, uint8_t len){
    if(len == 0) return;
    len = MIN(len, 32);
}

void LT8920::init(){

    if(nrst_gpio){
        nrst_gpio->clr();
        delay(20);
        nrst_gpio->set();
        delay(20);
    }

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

    setSyncWord(0x03805a5a03800380);
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
}

void LT8920::setSyncWord(const uint64_t syncword){
    const uint16_t * p = reinterpret_cast<const uint16_t *>(&syncword);
    for(uint8_t i = 0; i < 4; i++){
        sync_word_regs[i].data = p[i];
        writeReg(sync_word_regs[i].head_address + i, REG16(sync_word_regs[i]));
    }
}


void LT8920::writeReg(const RegAddress address, const uint16_t reg){
    if(spi_drv){
        {
            spi_drv->write((uint8_t)((uint8_t)address), false);
            delayT3();
        }

        spi_drv->write(REG16(reg));
    }else if(i2c_drv){
        i2c_drv->writeReg((uint8_t)address, reg);
    }
    LT8920_DEBUG("W",*(uint16_t *)&reg, "at", (uint8_t)address);
}

void LT8920::readReg(const RegAddress address, uint16_t & reg){
    if(spi_drv){
        // spi_drv->transfer(REG8(flag_reg), (uint8_t)((uint8_t)address | 0x80), false);
        // delayT3();

        spi_drv->read(REG16(reg));

    }else if(i2c_drv){
        i2c_drv->readReg((uint8_t)address, reg);
    }

    LT8920_DEBUG("R",*(uint16_t *)&reg, "at", (uint8_t)address);
}


void LT8920::writeFifo(const uint8_t * data, const size_t len){
    if(spi_drv){
        for(size_t i=0; i<len; i++){
            spi_drv->write(uint16_t(uint16_t(0x80 | 50) << 8 | data[i]));
        }
    }else if(i2c_drv){
        i2c_drv->writePool(uint8_t(0x80 | 50) , data, len, LSB);
    }
    LT8920_DEBUG("W", len);
}

void LT8920::readFifo(uint8_t * data, const size_t len){
    if(spi_drv){
        for(size_t i=0; i<len; i++){
            uint16_t temp = 0;
            spi_drv->transfer(temp, uint16_t(uint8_t(0x80 | 50) << 8), true);
            data[i] = temp & 0xff;
        }
    }else if(i2c_drv){
        i2c_drv->readPool(uint8_t(50), data, len, LSB);
    }
    LT8920_DEBUG("R", len);
}

bool LT8920::getFifoStatus(){
    if(fifo_status_gpio){
        return fifo_status_gpio->read();
    }else{
        READ_REG16(flag_reg);
        return flag_reg.fifoFlag;
    }
}

bool LT8920::getPktStatus(){
    if(pkt_status_gpio){
        return pkt_status_gpio->read();
    }else{
        READ_REG16(flag_reg);
        return flag_reg.pktFlag;
    }
}

#pragma GCC diagnostic pop