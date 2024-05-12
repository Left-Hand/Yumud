#ifndef __STORAGE_HPP__

#define __STORAGE_HPP__

#include "src/system.hpp"
#include "types/range/range_t.hpp"

class Memory;

class StorageProxy{

};

class Storage{
public:
    Storage(const Address & _size):size(_size){;}

    friend class Memory;
protected:
    using Address = uint32_t;
    using AddressWindow = Range_t<Address>;
    const Address size;
public:
    void getSize() const {return size;}

    virtual uint8_t store(const void * data, const Address & data_size, const Address & loc){

    }
    virtual void store(const uint8_t & data, const Address & loc) = 0;

    virtual uint8_t load()= 0;
    virtual void load(uint8_t & data, const Address & loc) = 0;

    virtual void entry();
    virtual void exit();
};




#endif
