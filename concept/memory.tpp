#include "storage.hpp"

namespace yumud{
void Memory::store(const auto & data, const Address loc){
    storage.store(&data, (Address)sizeof(data), loc);
}

void Memory::store(auto && data, const Address loc){
    storage.store(&data, (Address)sizeof(data), loc);
}

void Memory::load(auto & data, const Address loc){
    storage.load(&data, (Address)sizeof(data), loc);
}

template<typename T>
T Memory::load(const Address loc){
    T data;
    storage.load(&data, (Address)sizeof(data), loc);
    return data;
}

}