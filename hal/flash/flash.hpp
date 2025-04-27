#pragma once

#include "core/system.hpp"
#include "concept/storage.hpp"
#include "concept/memory.hpp"

namespace ymd{
class Flash:public Storage{
protected:

    using Page = size_t;
    using PageRange = Range_t<Page>;
    using Address = size_t;
    using AddressWindow = Range_t<Address>;

    scexpr Page page_size = 256;
    scexpr Address base_address = 0x08'00'00'00;
    scexpr uint max_clock = 72000000;

    const Page page_count;
    uint32_t orginal_clock;

    void config_clock();

    void revert_clock();

    void entry_store() override;
    void exit_store() override;
    void entry_load() override;
    void exit_load() override;

    void lock();

    void unlock();

    void store_bytes(const Address loc, const void * data, const Address len) override;

    void load_bytes(const Address loc, void * data, const Address len) override;

    void erase_page(const Address vaddr);
    void program_page(const Address vaddr, const void * buf);
public:
    Flash(Address _page_begin, Address _page_end):
        Storage((_page_end - _page_begin) * page_size),
        page_count(_page_end - _page_begin){;}
            
    Flash(int _page_begin = 0):
        Flash(_page_begin < 0 ? (get_max_pages() + _page_begin) : _page_begin, get_max_pages()){;}

    ~Flash(){}

    static size_t get_max_pages(){
        return sys::chip::get_flash_size() / page_size;
    }
    void init() override{

    }

    bool busy() override{
        return false;
    }

    Memory slice_pages(const int page_from){
        return slice_pages(page_from < 0 ? get_max_pages() + page_from : page_from, get_max_pages()); 
    }

    Memory slice_pages(const size_t page_from, const size_t page_to){
        // DEBUG_PRINTLN(page_from, page_to);
        return slice(page_from * page_size, page_to * page_size); 
    }
};
};