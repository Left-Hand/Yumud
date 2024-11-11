#include "X25QXX.hpp"

using namespace yumud::drivers;


void X25QXX::updateDeviceId(){
    writeByte(0x90, CONT);
    skipByte();
    skipByte();
    skipByte();

    readByte(jedec_id[2], CONT);
    readByte(jedec_id[0]);
}

void X25QXX::updateJedecId(){
    writeByte(0x9F, CONT);

    readByte(jedec_id[2], CONT);
    readByte(jedec_id[1], CONT);
    readByte(jedec_id[0]);
}

void X25QXX::writePage(const Address addr, const uint8_t * data, size_t len){
    auto & self = *this;

    // DEBUGGER.println("nr");
    // DEBUGGER.print_arr(data, len);
    // DEBUGGER.println("nr!");

    return;

    if(len > 256){
        X25QXX_DEBUG("page too large", len);
        HALT;
    }

    self.writeByte(Command::PageProgram, CONT);
    self.writeAddr(addr, CONT);
    self.writeBytes(data, len);
}


void X25QXX::entry_store(){
    delay(10);
    this->enableWrite();
}
    
void X25QXX::exit_store(){
    this->enableWrite(false);
    delay(10);
}
    
void X25QXX::entry_load(){
    delay(10);
}
    
void X25QXX::exit_load(){
    delay(10);
}


void X25QXX::enablePowerDown(const bool en){
    writeByte(en ? Command::PowerDown : Command::ReleasePowerDown);
}

void X25QXX::eraseBlock(const Address addr){
    auto & self = *this;

    self.writeByte(Command::SectorErase, CONT);
    self.writeAddr(addr);
}

void X25QXX::eraseSector(const Address addr){
    writeByte(Command::SectorErase, CONT);
    writeAddr(addr);
}

void X25QXX::eraseChip(){
    writeByte(Command::ChipErase);
}

bool X25QXX::isIdle(){
    writeByte(Command::ReadStatusRegister, CONT);
    readByte(statusReg);
    return statusReg.busy;
}

bool X25QXX::isWriteable(){
    writeByte(Command::ReadStatusRegister);
    readByte(statusReg);
    return statusReg.write_enable_latch;
}

void X25QXX::loadBytes(void * data, const Address len, const Address addr){
    auto & self = *this;

    self.writeByte(Command::ReadData, CONT);
    self.writeAddr(addr, CONT);
    self.readBytes(data, len);
}

void X25QXX::storeBytes(const void * _data, const size_t len, const size_t _addr){
    auto & self = *this;

    size_t pages = len / 256;
    size_t addr = _addr;
    const uint8_t * data = reinterpret_cast<const uint8_t *>(_data);

    for(size_t i = 0; i < pages; i++){
        self.writePage(addr, data, 256);
        addr += 256;
        data += 256;
    }

    uint8_t remains = len % 256;
    self.writePage(addr, data, remains);
}