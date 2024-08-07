#include "storage.hpp"
#include "memory.hpp"

Memory Storage::slice(const Storage::AddressView & _view){
    return Memory(*this, _view.intersection(view()));
}

Memory Storage::slice(const Storage::Address from, const Storage::Address to){
    return Memory(*this, view().intersection(Storage::AddressView{from, to}));
}

Storage::operator Memory(){
    return Memory(*this, view());
}
