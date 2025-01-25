#include "memory.hpp"

using namespace ymd;
Memory::Memory(Storage & _storage, const AddressView & _window):
        // storage_(_storage),view_(_window.intersection(_storage.view())){;}
        storage_(_storage),view_(_window){
            // DEBUG_PRINTLN(_window);
        }

void Memory::store(const Address loc, const uint8_t * begin,const uint8_t * end){
    storage_.store(loc, begin, end - begin);
}

void Memory::load(const Address loc, uint8_t * begin,uint8_t * end){
    storage_.load(loc, begin, end - begin);
}

