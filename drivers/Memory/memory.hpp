#ifndef __MEMORY_HPP__

#define __MEMORY_HPP__

#include "../types/range/range_t.hpp"
#include "storage.hpp"

// class Storage;

class Memory{

protected:
    using Address = size_t;
    using AddressWindow = Range_t<Address>;

    Storage & storage;
    AddressWindow m_window;
public:
    Memory(Storage & _storage, const AddressWindow & _window);
    Memory(Memory & _memory, const AddressWindow & _window):storage(_memory.storage), m_window(_window.intersection(_memory.window())){;}
    ~Memory(){;}
public:
    AddressWindow window(){return m_window;}

    size_t size(){return m_window.length();}

    template<typename T>
    void store(const T & data, const Address loc = 0);

    template<typename T>
    void load(T & data, const Address loc = 0);

    template<typename T>
    T load(const Address loc = 0);

};

#include "memory.tpp"


#endif