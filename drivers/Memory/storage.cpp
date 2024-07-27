#include "storage.hpp"
#include "memory.hpp"

Memory Storage::slice(const Storage::AddressView & _window){
    return Memory(*this, _window.intersection(view()));
}

Memory Storage::slice(const Storage::Address from, const Storage::Address to){
    return Memory(*this, view().intersection(Storage::AddressView{from, to}));
}

Storage::operator Memory(){
    return Memory(*this, view());
}
