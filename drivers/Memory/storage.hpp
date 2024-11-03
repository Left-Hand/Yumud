#pragma once

#include "types/range/range_t.hpp"
#include "sys/debug/debug_inc.h"

namespace yumud{

class Memory;
class Storage{
protected:
    using Address = size_t;
    using AddressView = Range_t<Address>;

    const Address m_capacity;
    const AddressView m_view;

    friend class Memory;
protected:
    Storage(const Address _capacity):m_capacity(_capacity), m_view({0, _capacity}){;}
    Storage(const Address _capacity, const AddressView & _view):m_capacity(_capacity), m_view(_view){;}
    friend class Memory;

    virtual void entry_store() = 0;
    virtual void exit_store() = 0;

    virtual void entry_load() = 0;
    virtual void exit_load() = 0;

    virtual void storeBytes(const uint8_t data, const Address loc){
        storeBytes(&data, 1, loc);
    }

    virtual void loadBytes(uint8_t & data, const Address loc){
        loadBytes(&data, 1, loc);
    }

    virtual void storeBytes(const void * data, const Address data_size, const Address loc) = 0;

    virtual void loadBytes(void * data, const Address data_size, const Address loc) = 0;

public:
    virtual void init() = 0;

    virtual bool busy() = 0;
    Address size() const {return m_capacity;}
    AddressView view() const {return {0, m_capacity};}

    void store(const void * data, const Address data_size, const Address loc){
        if(view().has(loc)){
            entry_store();
            storeBytes(data, data_size, loc);
            exit_store();
        }
    }

    void store(const uint8_t data, const Address loc){
        entry_store();
        storeBytes(data, loc);
        exit_store();
    }

    void load(uint8_t & data, const Address loc){
        entry_load();
        loadBytes(data, loc);
        exit_load();
    }

    void load(void * data, const Address data_size, const Address loc){
        entry_load();
        loadBytes(data, data_size, loc);
        exit_load();
    }

    uint8_t load(const Address loc){
        uint8_t data;
        load(data, loc);
        return data;
    }

    operator Memory();
    Memory slice(const AddressView & _view);
    Memory slice(const size_t from, const size_t to);
};


class StoragePaged:public Storage{
protected:
    const Address m_pagesize;
public:
    StoragePaged(const Address _capacity, const Address _pagesize):Storage(_capacity, {0, _capacity}), m_pagesize(_pagesize){;}
    StoragePaged(const Address _capacity, const AddressView  & _view, const Address _pagesize):Storage(_capacity, _view), m_pagesize(_pagesize){;}
};

}