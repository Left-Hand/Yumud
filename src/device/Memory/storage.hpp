#ifndef __STORAGE_HPP__

#define __STORAGE_HPP__

#include "src/system.hpp"
#include "types/range/range_t.hpp"

class Memory;

class StorageProxy{

};

class Storage{
protected:
    using Address = uint32_t;
    using AddressWindow = Range_t<Address>;
    const Address size;

public:
    Storage(const Address & _size):size(_size){;}

    friend class Memory;

    virtual void entry() = 0;
    virtual void exit() = 0;


    virtual void _store(const uint8_t & data, const Address & loc) = 0;
    virtual void _load(uint8_t & data, const Address & loc) = 0;

    virtual void _store(const void * data, const Address & data_size, const Address & loc){
        for(Address addr = loc; addr < loc + data_size; addr++){
            _store(*((const uint8_t *)data + addr - loc), addr);
        }
    }
    virtual void _load(void * data, const Address & data_size, const Address & loc) {
        for(Address addr = loc; addr < loc + data_size; addr++){
            _load(*((uint8_t *)data + addr - loc), addr);
        }
    }
public:
    virtual void init() = 0;
    auto getSize() const {return size;}

    void store(const void * data, const Address & data_size, const Address & loc){
        entry();
        _store(data, data_size, loc);
        exit();
    }

    void store(const uint8_t & data, const Address & loc){
        entry();
        _store(data, loc);
        exit();
    }

    void load(uint8_t & data, const Address & loc){
        entry();
        _load(data, loc);
        exit();
    }

    void load(void * data, const Address & data_size, const Address & loc){
        entry();
        _load(data, data_size, loc);
        exit();
    }

    uint8_t load(const Address & loc){
        entry();
        uint8_t data;
        _load(data, loc);
        exit();
        return data;
    }
};




#endif
