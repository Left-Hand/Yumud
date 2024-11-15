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

    void store(const auto & data, const Address loc = 0);
    void store(auto && data, const Address loc = 0);
    void load(auto & data, const Address loc = 0);

    template<typename T>
    T load(const Address loc = 0);

};

}
#include "memory.tpp"