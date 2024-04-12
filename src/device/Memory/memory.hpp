#ifndef __MEMORY_HPP__

#define __MEMORY_HPP__

#include "storage.hpp"

template<int size>
class Memory{
protected:
    Storage & storage;
public:
    Memory(Storage & _storage):storage(_storage){;}
    ~Memory(){;}
};


template<int size>
class Sector:public Memory<size>{

};


#endif