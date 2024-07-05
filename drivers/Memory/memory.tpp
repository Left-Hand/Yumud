#include "storage.hpp"

template<typename T>
void Memory::store(const T & data, const Address loc){
    // DEBUG_PRINT("memory::store", (u32)&data, sizeof(data), loc);
    storage.store(&data, (Address)sizeof(data), loc);
}

template<typename T>
void Memory::load(T & data, const Address loc){
    storage.load(&data, (Address)sizeof(data), loc);
}