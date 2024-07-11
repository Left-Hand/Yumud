#include "at24cxx.hpp"

#ifdef AT24CXX_DEBUG
#undef AT24CXX_DEBUG
#define AT24CXX_DEBUG(...) DEBUG_PRINT(SpecToken::Space, ##__VA_ARGS__, "\t|", __PRETTY_FUNCTION__);
#else
#define AT24CXX_DEBUG(...)
#endif


void AT24CXX::_store(const uint8_t data, const Address loc){
    if(loc >= m_size){
        AT24CXX_DEBUG("invalid addr");
        return;
    }

    AT24CXX_DEBUG("store", data, "at", loc);
    bus_drv.writeReg((uint8_t)loc, data);
}
void AT24CXX::_load(uint8_t & data, const Address loc){
    if(loc >= m_size){
        AT24CXX_DEBUG("invalid addr");
        return;
    }

    AT24CXX_DEBUG("load", data, "at", loc);
    bus_drv.readReg((uint8_t)loc, data);
}

void AT24CXX::wait_for_done(){
    uint32_t delays;
    if(last_entry_ms == 0){
        delays = min_duration_ms;
        update_entry_ms();
    }else{
        // delays = MAX(last_entry_ms + min_duration_ms - millis(), 0);
        delays = min_duration_ms;
    }

    // AT24CXX_DEBUG("wait for", delays, "ms");
    delay(delays);
}

void AT24CXX::_store(const void * data, const Address data_size, const Address loc){
    auto full_end = loc + data_size; 
    if(full_end > m_size){
        AT24CXX_DEBUG("invalid addr at:", loc);
        return;
    }


    // auto chip_window = window();
    AddressWindow store_window = AddressWindow{loc,loc + data_size};
    AddressWindow op_window = {0,0};
    AT24CXX_DEBUG("multi store entry", store_window);
    do{
        op_window = store_window.grid_forward(op_window, page_size);
        if(op_window){
            uint8_t * ptr = ((uint8_t *)data + (op_window.start - store_window.start));
            // DEBUGGER << op_window;
            // for(uint32_t i = op_window.start; i < op_window.end; i++) DEBUGGER << ',' << ((uint8_t*)data)[i];
            // DEBUGGER << "\r\n";
            wait_for_done();
            bus_drv.writePool(op_window.start, ptr, op_window.length());
            update_entry_ms();
        }
    }while(op_window);
}


void AT24CXX::_load(void * data, const Address data_size, const Address loc){
    auto full_end = loc + data_size; 
    if(full_end > m_size){
        AT24CXX_DEBUG("invalid addr at:", loc);
        return;
    }

    bus_drv.readPool(loc, (uint8_t *)data, data_size);

    // #ifdef AT24CXX_DEBUG
    // AddressWindow store_window = AddressWindow{loc,loc + data_size};
    // AddressWindow op_window = {0,0};
    // AT24CXX_DEBUG("check entry", store_window);
    // do{
    //     op_window = store_window.grid_forward(op_window, page_size);
    //     if(op_window){
    //         DEBUGGER << op_window;
    //         for(uint32_t i = op_window.start; i < op_window.end; i++) DEBUGGER << ',' << ((uint8_t*)data)[i];
    //         DEBUGGER << "\r\n";
    //     }
    // }while(op_window);
    // #endif
}