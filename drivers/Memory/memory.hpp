#ifndef __MEMORY_HPP__

#define __MEMORY_HPP__

#include "../types/range/range_t.hpp"
#include "storage.hpp"

// class Storage;

class Memory{

protected:
    using Address = size_t;
    using AddressView = Range_t<Address>;

    Storage & storage;
    AddressView m_view;
public:
    Memory(Storage & _storage, const AddressView & _window);
    Memory(Memory & _memory, const AddressView & _window):storage(_memory.storage), m_view(_window.intersection(_memory.view())){;}
    ~Memory(){;}
public:
    AddressView view(){return m_view;}

    size_t size(){return m_view.length();}

    template<typename T>
    void store(const T & data, const Address loc = 0);

    template<typename T>
    void load(T & data, const Address loc = 0);

    template<typename T>
    T load(const Address loc = 0);

};

#include "memory.tpp"


#endif