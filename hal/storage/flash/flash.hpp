#pragma once

#include "core/system.hpp"


#if 0
namespace ymd{
class Flash{
protected:

    using Page = size_t;
    using PageRange = Range2<Page>;
    using Address = size_t;
    using AddressWindow = Range2<Address>;

    static constexpr Page page_size = 256;
    static constexpr Address base_address = 0x08'00'00'00;
    static constexpr uint max_clock = 72000000;

    const Page page_count;
    uint32_t orginal_clock;

    void config_clock();

    void revert_clock();

    void entry_store();
    void exit_store();
    void entry_load();
    void exit_load();

    void lock();

    void unlock();

    void store_bytes(const Address loc, const void * data, const Address len);

    void load_bytes(const Address loc, void * data, const Address len);

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
    void init(){

    }

    bool busy(){
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

#endif