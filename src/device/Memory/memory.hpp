#ifndef __MEMORY_HPP__

#define __MEMORY_HPP__

#include "storage.hpp"

class Memory{
public:

    Memory(Storage & _storage, const Rangei & _window):storage(_storage), window(_window){;}
    ~Memory(){;}
protected:
    using Address = uint32_t;
    using AddressWindow = Range_t<Address>;

    Storage & storage;
    AddressWindow window;
public:
    auto getWindow(){
        return window;
    }

    template<typename T>
    void store(const T & data){

    }

    template<typename T>
    void load(T & data){
        
    }
};


class Sector:public Memory{

};


#endif