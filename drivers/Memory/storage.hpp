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

    virtual void entry_store() = 0;
    virtual void exit_store() = 0;

    virtual void entry_load() = 0;
    virtual void exit_load() = 0;
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

    virtual bool busy() = 0;
    auto getSize() const {return size;}
    Rangei getWindow() const {return {0, getSize()}; }
    void store(const void * data, const Address & data_size, const Address & loc){
        entry_store();
        _store(data, data_size, loc);
        exit_store();
    }

    void store(const uint8_t & data, const Address & loc){
        entry_store();
        _store(data, loc);
        exit_store();
    }

    void load(uint8_t & data, const Address & loc){
        entry_load();
        _load(data, loc);
        exit_load();
    }

    void load(void * data, const Address & data_size, const Address & loc){
        entry_load();
        _load(data, data_size, loc);
        exit_load();
    }

    uint8_t load(const Address & loc){
        uint8_t data;
        load(data, loc);
        return data;
    }
};




#endif
