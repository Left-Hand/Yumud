#ifndef __STORAGE_HPP__

#define __STORAGE_HPP__

#include "src/system.hpp"
#include "types/range/range_t.hpp"
// #include "memory.hpp"

class Memory;
class Storage{
protected:
    using Address = uint32_t;
    using AddressWindow = Range_t<Address>;

    const Address m_size;
    const AddressWindow m_window;

    friend class Memory;
protected:
    Storage(const Address _size):m_size(_size), m_window({0, _size}){;}
    Storage(const Address _size, const AddressWindow  & _window):m_size(_size), m_window(_window){;}
    friend class Memory;

    virtual void entry_store() = 0;
    virtual void exit_store() = 0;

    virtual void entry_load() = 0;
    virtual void exit_load() = 0;
    virtual void _store(const uint8_t data, const Address loc){
        _store(&data, sizeof(data), loc);
    }
    virtual void _load(uint8_t & data, const Address loc){
        _load(&data, sizeof(data), loc);
    }

    virtual void _store(const void * data, const Address data_size, const Address loc){
        for(Address addr = loc; addr < loc + data_size; addr++){
            _store(*((const uint8_t *)data + addr - loc), addr);
        }
    }
    virtual void _load(void * data, const Address data_size, const Address loc) {
        for(Address addr = loc; addr < loc + data_size; addr++){
            _load(*((uint8_t *)data + addr - loc), addr);
        }
    }

public:
    virtual void init() = 0;

    virtual bool busy() = 0;
    Address size() const {return m_size;}
    AddressWindow window() const {return {0, m_size};}
    void store(const void * data, const Address & data_size, const Address & loc){
        if(window().has(loc)){
            entry_store();
            _store(data, data_size, loc);
            exit_store();
        }
    }

    void store(const uint8_t & data, const Address loc){
        entry_store();
        _store(data, loc);
        exit_store();
    }

    void load(uint8_t & data, const Address loc){
        entry_load();
        _load(data, loc);
        exit_load();
    }

    void load(void * data, const Address & data_size, const Address loc){
        entry_load();
        _load(data, data_size, loc);
        exit_load();
    }

    uint8_t load(const Address loc){
        uint8_t data;
        load(data, loc);
        return data;
    }

    operator Memory();
    Memory slice(const AddressWindow & _window);
};




#endif
