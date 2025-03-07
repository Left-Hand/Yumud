#pragma once

#include "types/range/range_t.hpp"

namespace ymd{

class Memory;
class Storage{
protected:
    using Address = size_t;
    using AddressView = Range_t<Address>;

    const Address capacity_;
    const AddressView view_;

    friend class Memory;
protected:
    Storage(const Address capacity):capacity_(capacity), view_({0, capacity}){;}
    Storage(const Address capacity, const AddressView & _view):capacity_(capacity), view_(_view){;}

    virtual ~Storage() = default;
    friend class Memory;

    virtual void entry_store() = 0;
    virtual void exit_store() = 0;

    virtual void entry_load() = 0;
    virtual void exit_load() = 0;

    virtual void storeByte(const Address loc, const uint8_t data){
        storeBytes(loc, &data, 1);
    }
    virtual void loadByte(const Address loc, uint8_t & data){
        loadBytes(loc, &data, 1);
    }

    virtual void storeBytes(const Address loc, const void * data, const Address len) = 0;

    virtual void loadBytes(const Address loc, void * data, const Address len) = 0;

    virtual void eraseBytes(const Address loc, const Address len){};
public:
    virtual void init() = 0;

    virtual bool busy() = 0;
    Address size() const {return capacity_;}
    AddressView view() const {return {0, capacity_};}

    void store(const Address loc, const void * data, const Address len);

    void load(const Address loc, void * data, const Address len);

    void erase(const Address loc, const size_t len);

    operator Memory();
    Memory slice(const AddressView & _view);
    Memory slice(const size_t from, const size_t to);
};


class StoragePaged:public Storage{
protected:
    const Address m_pagesize;
public:
    StoragePaged(const Address capacity, const Address _pagesize):Storage(capacity, {0, capacity}), m_pagesize(_pagesize){;}
    StoragePaged(const Address capacity, const AddressView  & _view, const Address _pagesize):Storage(capacity, _view), m_pagesize(_pagesize){;}
};

}