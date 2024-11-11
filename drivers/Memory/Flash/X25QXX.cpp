#include "X25QXX.hpp"

using namespace yumud::drivers;


class RAIIfunctor{
protected:
    using Callback = std::function<void(void)>; 
    Callback end_f_;
public:
    RAIIfunctor(Callback && begin_f, Callback && end_f):
        end_f_(std::move(end_f)){
            begin_f();
        }

    ~RAIIfunctor(){
        end_f_();
    }
};

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

bool X25QXX::waitForFree(size_t timeout){
    writeByte(Command::ReadStatusRegister, CONT);

    size_t begin = millis();

    while(millis() - begin < timeout){
        readByte(statusReg, CONT);
        if(statusReg.busy == false){
            break;
        }
    }
    
    spi_drv_.end();

    return statusReg.busy;
}

void X25QXX::updateStatus(){
    writeByte(Command::ReadStatusRegister, CONT);
    readByte(statusReg);
};

bool X25QXX::busy(){
    updateStatus();
    return statusReg.busy;
}

void X25QXX::writePage(const Address addr, const uint8_t * data, size_t len){
    auto & self = *this;

    if(len > 256){
        X25QXX_DEBUG("page too large", len);
        HALT;
    }

    self.writeByte(Command::PageProgram, CONT);
    self.writeAddr(addr, CONT);
    self.writeBytes(data, len);
}


void X25QXX::entry_store(){
    auto & self = *this;

    self.waitForFree(UINT_MAX);
    self.enableWrite();
}
    
void X25QXX::exit_store(){
    this->enableWrite(false);
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


//4kb
void X25QXX::eraseSector(const Address addr){
    auto & self = *this;

    self.writeByte(Command::SectorErase, CONT);
    self.writeAddr(addr);
}

//64kb
void X25QXX::eraseBlock(const Address addr){
    auto & self = *this;

    self.writeByte(Command::BlockErase, CONT);
    self.writeAddr(addr);
}


void X25QXX::eraseWholeChip(){
    auto & self = *this;
    
    self.writeByte(Command::ChipErase);
}


bool X25QXX::isWriteable(){
    writeByte(Command::ReadStatusRegister);
    readByte(statusReg);
    return statusReg.write_enable_latch;
}

void X25QXX::loadBytes(const Address loc , void * data, const Address len){
    auto & self = *this;

    self.writeByte(Command::ReadData, CONT);
    self.writeAddr(loc, CONT);
    self.readBytes(data, len);
}

void X25QXX::storeBytes(const Address loc, const void * data, const Address len){
    auto & self = *this;

    // size_t addr = _addr;
    // const uint8_t * data = reinterpret_cast<const uint8_t *>(_data);

    // for(size_t i = 0; i < pages; i++){
    //     self.writePage(addr, data, 256);
    //     addr += 256;
    //     data += 256;
    // }

    // size_t remains = len % 256;
    // self.writePage(addr, data, remains);

    AddressView store_window = AddressView{loc,loc + len};
    AddressView op_window = {0,0};
    
    do{
        op_window = store_window.grid_forward(op_window, m_pagesize);
        if(op_window){
            self.waitForFree(UINT_MAX);
            auto * ptr = (reinterpret_cast<const uint8_t *>(data) + (op_window.from - store_window.from));
            self.writePage(op_window.from, ptr, op_window.length());
        }
    }while(op_window);
}

void X25QXX::writeAddr(const Address addr, const Continuous cont){
    if(isLargeChip()){
        PANIC("large chip is under develop");
        writeByte(addr >> 24, CONT);
    }

    writeByte(addr >> 16, CONT);
    writeByte(addr >> 8, CONT);
    writeByte(addr, cont);
}
void X25QXX::eraseBytes(const Address loc, const size_t len){
    //FIXME
    eraseSector(loc);
}
