#include "storage.hpp"

namespace ymd{
void Memory::store(const auto & data, const Address loc){
    storage_.store(loc, &data, (Address)sizeof(data));
}

void Memory::store(auto && data, const Address loc){
    storage_.store(loc, &data, (Address)sizeof(data));
}

void Memory::load(auto & data, const Address loc){
    storage_.load(loc, &data, (Address)sizeof(data));
}

template<typename T>
T Memory::load(const Address loc){
    T data;
    storage_.load(loc, &data, (Address)sizeof(data));
    return data;
}

}