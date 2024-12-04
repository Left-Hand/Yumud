#pragma once

#include "types/range/range_t.hpp"
#include "storage.hpp"

namespace ymd{


class Memory{

protected:
    using Address = size_t;
    using AddressView = Range_t<Address>;

    Storage & storage_;
    AddressView view_;
public:
    Memory(Storage & storage, const AddressView & view);
    // Memory(Memory & memory, const AddressView & view):storage_(memory.storage_), view_(storage_..intersection(memory_.view())){;}
    ~Memory(){;}
public:
    AddressView view(){return view_;}

    size_t size(){return view_.length();}

void store(const Address loc, const is_stdlayout auto & data);
    void store(const Address loc, is_stdlayout auto && data);
    void store(const Address loc, const uint8_t * begin, const uint8_t * end);

    void load(const Address loc, is_stdlayout auto & data);
    void load(const Address loc, uint8_t * begin, uint8_t * end);

    template<typename T>
    T load(const Address loc);

};



}


#include "memory.tpp"