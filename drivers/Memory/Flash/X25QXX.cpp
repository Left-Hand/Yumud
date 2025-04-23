#include "X25QXX.hpp"
#include "core/debug/debug.hpp"

using namespace ymd::drivers;

#define self (*this)
#define X25QXX_DEBUG

#ifdef X25QXX_DEBUG
#undef X25QXX_DEBUG
#define X25QXX_DEBUG(...) DEBUG_PRINTLN(__VA_ARGS__)
#else
#define X25QXX_DEBUG(...)
#endif

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

void X25QXX::update_device_id(){
    write_byte(0x90, CONT);
    skip_byte();
    skip_byte();
    skip_byte();

    read_byte(jedec_id.manufacturer_id, CONT);
    read_byte(jedec_id.capacity);
}

void X25QXX::update_jedec_id(){
    write_byte(0x9F, CONT);

    read_byte(jedec_id.manufacturer_id, CONT);
    read_byte(jedec_id.memory_type, CONT);
    read_byte(jedec_id.capacity);
}

bool X25QXX::wait_for_free(size_t timeout){
    write_byte(Command::ReadStatusRegister, CONT);

    size_t begin = millis();

    while(millis() - begin < timeout){
        read_byte(statusReg, CONT);
        if(statusReg.busy == false){
            break;
        }
    }
    
    spi_drv_.end();

    return statusReg.busy;
}

void X25QXX::update_status(){
    write_byte(Command::ReadStatusRegister, CONT);
    read_byte(statusReg);
};

bool X25QXX::busy(){
    update_status();
    return statusReg.busy;
}

void X25QXX::write_page(const Address addr, const uint8_t * data, size_t len){
    if(len > 256){
        X25QXX_DEBUG("page too large", len);
        HALT;
    }

    self.write_byte(Command::PageProgram, CONT);
    self.write_addr(addr, CONT);
    self.write_bytes(data, len);
}


void X25QXX::entry_store(){
    self.wait_for_free(UINT32_MAX);
    // self.enable_write();
}
    
void X25QXX::exit_store(){
    // this->enable_write(false);
}
    
void X25QXX::entry_load(){
    delay(10);
}
    
void X25QXX::exit_load(){
    delay(10);
}


void X25QXX::enable_power_down(const bool en){
    write_byte(en ? Command::PowerDown : Command::ReleasePowerDown);
}


//4kb
void X25QXX::erase_sector(const Address addr){


    self.write_byte(Command::SectorErase, CONT);
    self.write_addr(addr);
}

//64kb
void X25QXX::erase_block(const Address addr){


    self.write_byte(Command::BlockErase, CONT);
    self.write_addr(addr);
}


void X25QXX::erase_whole_chip(){

    
    self.write_byte(Command::ChipErase);
}


bool X25QXX::is_writeable(){
    write_byte(Command::ReadStatusRegister);
    read_byte(statusReg);
    return statusReg.write_enable_latch;
}

void X25QXX::load_bytes(const Address loc , void * data, const Address len){


    self.write_byte(Command::ReadData, CONT);
    self.write_addr(loc, CONT);
    self.read_bytes(data, len);
}

void X25QXX::store_bytes(const Address loc, const void * data, const Address len){


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
            self.wait_for_free(UINT32_MAX);
            auto * ptr = (reinterpret_cast<const uint8_t *>(data) + (op_window.from - store_window.from));
            self.write_page(op_window.from, ptr, op_window.length());
        }
    }while(op_window);
}

void X25QXX::write_addr(const Address addr, const Continuous cont){
    if(is_large_chip()){
        PANIC("large chip is under develop");
        write_byte(addr >> 24, CONT);
    }

    write_byte(addr >> 16, CONT);
    write_byte(addr >> 8, CONT);
    write_byte(addr, cont);
}
void X25QXX::erase_bytes(const Address loc, const size_t len){
    //FIXME
    erase_sector(loc);
}
