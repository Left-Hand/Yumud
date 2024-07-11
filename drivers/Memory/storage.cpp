#include "storage.hpp"
#include "memory.hpp"

Memory Storage::slice(const AddressWindow & _window){
    return Memory(*this, _window.intersection(window()));
}

Memory Storage::slice(const size_t from, const size_t to){
    return Memory(*this, window().intersection(AddressWindow{from, to}));
}

Storage::operator Memory(){
    return Memory(*this, window());
}
