#pragma once

#include "types/regions/range2/range2.hpp"
#include "storage.hpp"

namespace ymd{


class Memory{

protected:

    StorageIntf & storage_;
    const AddressRange view_;
protected:

public:
    constexpr auto address(){return view_.from;}
    constexpr auto capacity(){return view_.capacity();}

    Memory(StorageIntf & storage, const AddressRange & view):
        storage_(storage),
        view_(view){;} 

    Memory(StorageIntf & storage):
        storage_(storage),
        view_(AddressRange{Address{0}, storage.capacity()}){;} 
    // auto store(const Address loc, is_stdlayout auto && obj){
    //     return store_bytes(loc, {reinterpret_cast<const uint8_t *>(&obj), 
    //         sizeof(std::decay_t<decltype(obj)>)});
    // }

    // auto store_bytes(const Address loc, const std::span<const uint8_t> pdata){
    //     return storage_.store_bytes(loc, pdata);
    // }

    // auto load(const Address loc, is_stdlayout auto & obj){
    //     return load_bytes(loc, {reinterpret_cast<uint8_t *>(&obj), 
    //         sizeof(std::decay_t<decltype(obj)>)});
    // }

    // auto load_bytes(const Address loc, const std::span<uint8_t> pdata){
    //     return storage_.load_bytes(loc, pdata);
    // }

    // auto erase_bytes(const Address loc, const AddressDiff capacity){
    //     return storage_.erase_bytes(loc, capacity);
    // }

    Memory slice(const Address loc, const AddressDiff capacity){
        return Memory{storage_, AddressRange{loc, capacity}};
    }
};

}
