#include "storage.hpp"

namespace ymd{
void Memory::store(const Address loc, const is_stdlayout auto & data){
    storage_.store(loc + view_.from, &data, (Address)sizeof(data));
}

void Memory::store(const Address loc, is_stdlayout auto && data){
    storage_.store(loc + view_.from, &data, (Address)sizeof(data));
}

void Memory::load(const Address loc, is_stdlayout auto & data){
    storage_.load(loc + view_.from, &data, (Address)sizeof(data));
}

template<typename T>
T Memory::load(const Address loc){
    T data;
    storage_.load(loc + view_.from, &data, (Address)sizeof(data));
    return data;
}

}