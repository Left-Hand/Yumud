#ifndef __MEMORY_HPP__

#define __MEMORY_HPP__

#include "storage.hpp"

class Memory{

protected:
    using Address = uint32_t;
    using AddressWindow = Range_t<Address>;

    Storage & storage;
    AddressWindow window;
public:
    Memory(Storage & _storage):storage(_storage), window(_storage.getWindow()){;}
    Memory(Storage & _storage, const AddressWindow & _window):storage(_storage), window(_window.intersection(_storage.getWindow())){;}
    Memory(Memory & _memory, const AddressWindow & _window):storage(_memory.storage), window(_window.intersection(_memory.getWindow())){;}
    ~Memory(){;}
public:
    AddressWindow getWindow(){
        return window;
    }

    template<typename T>
    void store(const T & data, const Address & loc){
        storage.store(data, sizeof(data), loc);
    }

    template<typename T>
    void load(T & data, const Address & loc){
        storage.load(data, sizeof(data), loc);
    }
};


class Sector:public Memory{

};


#endif