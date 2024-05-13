#ifndef __AT24C02_HPP__

#define __AT24C02_HPP__

#include "device/device_defs.h"
#include "../../memory.hpp"

#define AT24C02_DEBUG

#ifdef AT24C02_DEBUG
#undef AT24C02_DEBUG
#define AT24C02_DEBUG(...) DEBUG_LOG(SpecToken::Space, ##__VA_ARGS__, "\t|", __PRETTY_FUNCTION__);
#else
#define AT24C02_DEBUG(...)
#endif

class AT24C02:public Storage{
protected:
    I2cDrv & bus_drv;
    static constexpr Address chip_size = 256;
    static constexpr Address page_size = 8;
    static constexpr uint32_t min_duration_ms = 5;
    uint32_t last_entry_ms = 0;
    
    void _store(const uint8_t & data, const Address & loc) override{
        if(loc >= chip_size){
            AT24C02_DEBUG("invalid addr");
            return;
        }

        AT24C02_DEBUG("store", data, "at", loc);
        bus_drv.writeReg((uint8_t)loc, data);
    }
    void _load(uint8_t & data, const Address & loc) override{
        if(loc >= chip_size){
            AT24C02_DEBUG("invalid addr");
            return;
        }

        AT24C02_DEBUG("load", data, "at", loc);
        bus_drv.readReg((uint8_t)loc, data);
    }

    void wait_for_done(){
        delay(MAX(last_entry_ms + min_duration_ms - millis(), 0));
    }

    void _store(const void * data, const Address & data_size, const Address & loc) override {
        auto full_end = loc + data_size; 
        if(full_end >= chip_size){
            AT24C02_DEBUG("invalid addr");
            return;
        }

        AT24C02_DEBUG("multi store entry", data_size, loc);
        auto chip_window = getWindow();
        AddressWindow store_window_last = {};
        auto store_window = store_window_last;
        // Address loc_begin = loc;
        // Rangei page_window{0, page_size};

        // if(loc_begin)
        bool virgin = true;
        for(Address i = 0; i < data_size; i++){
            Address loc_global = loc + i;
            store_window = chip_window.part_in_grid(loc_global, page_size, virgin);
            AT24C02_DEBUG("iter", store_window, loc_global);
            if(!store_window)break;
            else{
                if(store_window != store_window_last){
                    virgin = false;
                    // AT24C02_DEBUG("write page happens", store_window.start, store_window.end);
                }
            }
            store_window_last = store_window;
        }

        //     Address loc_begin = (loc_global / page_size) * page_size;
        //     Address loc_end = MIN(full_end, (loc_global / page_size + 1) * page_size);
        //     AT24C02_DEBUG("loc", loc_global, loc_begin, loc_end);
        //     if(loc_global == loc_end) break;
        //     if(loc_global % page_size == 0 && loc_global != loc_begin){
        //         wait_for_done();
        //         AT24C02_DEBUG("write page happens", loc_begin, loc_end);
        //         // bus_drv.writePool(loc_begin, (uint8_t *)data + (loc_begin - loc), 1, loc_end - loc_begin);
        //     }else if(loc_global == loc_end){
        //         wait_for_done();
        //         AT24C02_DEBUG("write page happens", loc_begin, loc_end);
        //         // bus_drv.writePool(loc_begin, (uint8_t *)data + (loc_begin - loc), 1, loc_end - loc_begin);
        //     }
        // }
    }

    void entry_store() override{

    };

    void exit_store() override{
        last_entry_ms = millis();
    };

    void entry_load() override{
        last_entry_ms = millis();
    };

    void exit_load() override{
        last_entry_ms = millis();
    };
public:
    void init() override{};

    bool busy() override{return last_entry_ms + min_duration_ms - millis() > 0;}

    static constexpr uint8_t default_id = 0b10100000; 
    AT24C02(I2cDrv & _bus_drv):Storage(256), bus_drv(_bus_drv){;}
};

#endif
