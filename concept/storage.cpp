#include "storage.hpp"
#include "memory.hpp"

using namespace ymd;

Memory Storage::slice(const Storage::AddressView & _view){
    return Memory(*this, view().intersection(view()));
}

Memory Storage::slice(const Storage::Address from, const Storage::Address to){
    // return Memory(*this, view().intersection(Storage::AddressView{from, to}));
    // DEBUG_PRINTLN(from, to);
    return Memory(*this, {from, to});
}

void Storage::store(const Address loc, const void * data, const Address len){
    if(view().has(loc)){
        entry_store();
        store_bytes(loc, data, len);
        exit_store();
    }
}

void Storage::load(const Address loc, void * data, const Address len){
    if(view().has(loc)){
        entry_load();
        load_bytes(loc, data, len);
        exit_load();
    }
}


void Storage::erase(const Address loc, const size_t len){
    if(view().has(loc)){
        entry_store();
        erase_bytes(loc, len);
        exit_store();
    }
}

Storage::operator Memory(){
    return Memory(*this, view());
}
