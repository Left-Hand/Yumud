#include "storage.hpp"
#include "memory.hpp"

Memory Storage::slice(const AddressWindow & _window){
    return Memory(*this, _window.intersection(window()));
}

Storage::operator Memory(){
    return Memory(*this, window());
}
