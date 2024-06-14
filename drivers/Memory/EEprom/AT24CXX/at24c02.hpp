#ifndef __AT24C02_HPP__

#define __AT24C02_HPP__

#include "drivers/device_defs.h"
#include "../../memory.hpp"

#define AT24C02_DEBUG

#ifdef AT24C02_DEBUG
#undef AT24C02_DEBUG
#define AT24C02_DEBUG(...) DEBUG_PRINT(SpecToken::Space, ##__VA_ARGS__, "\t|", __PRETTY_FUNCTION__);
#else
#define AT24C02_DEBUG(...)
#endif

class AT24C02:public Storage{
protected:
    I2cDrv bus_drv;
    static constexpr Address chip_size = 256;
    static constexpr Address page_size = 8;
    static constexpr uint32_t min_duration_ms = 7;
    uint32_t last_entry_ms = 0;
    
    void _store(const uint8_t data, const Address loc) override{
        if(loc >= chip_size){
            AT24C02_DEBUG("invalid addr");
            return;
        }

        AT24C02_DEBUG("store", data, "at", loc);
        bus_drv.writeReg((uint8_t)loc, data);
    }
    void _load(uint8_t & data, const Address loc) override{
        if(loc >= chip_size){
            AT24C02_DEBUG("invalid addr");
            return;
        }

        AT24C02_DEBUG("load", data, "at", loc);
        bus_drv.readReg((uint8_t)loc, data);
    }

    void wait_for_done(){
        uint32_t delays;
        if(last_entry_ms == 0){
            delays = min_duration_ms;
            update_entry_ms();
        }else{
            // delays = MAX(last_entry_ms + min_duration_ms - millis(), 0);
            delays = min_duration_ms;
        }

        // AT24C02_DEBUG("wait for", delays, "ms");
        delay(delays);
    }

    void update_entry_ms(){
        last_entry_ms = millis();
    }

    void _store(const void * data, const Address data_size, const Address loc) override {
        auto full_end = loc + data_size; 
        if(full_end > chip_size){
            AT24C02_DEBUG("invalid addr at:", loc);
            return;
        }


        // auto chip_window = window();
        AddressWindow store_window = AddressWindow{loc,loc + data_size};
        AddressWindow op_window = {0,0};
        AT24C02_DEBUG("multi store entry", store_window);
        do{
            op_window = store_window.grid_forward(op_window, page_size);
            if(op_window){
                uint8_t * ptr = ((uint8_t *)data + (op_window.start - store_window.start));
                DEBUGGER << op_window;
                for(uint32_t i = op_window.start; i < op_window.end; i++) DEBUGGER << ',' << ((uint8_t*)data)[i];
                DEBUGGER << "\r\n";
                wait_for_done();
                bus_drv.writePool(op_window.start, ptr, op_window.length());
                update_entry_ms();
            }
        }while(op_window);
    }

    void _load(void * data, const Address data_size, const Address loc) override {
        auto full_end = loc + data_size; 
        if(full_end > chip_size){
            AT24C02_DEBUG("invalid addr at:", loc);
            return;
        }

        bus_drv.readPool(loc, (uint8_t *)data, data_size);

        #ifdef AT24C02_DEBUG
        AddressWindow store_window = AddressWindow{loc,loc + data_size};
        AddressWindow op_window = {0,0};
        AT24C02_DEBUG("check entry", store_window);
        do{
            op_window = store_window.grid_forward(op_window, page_size);
            if(op_window){
                DEBUGGER << op_window;
                for(uint32_t i = op_window.start; i < op_window.end; i++) DEBUGGER << ',' << ((uint8_t*)data)[i];
                DEBUGGER << "\r\n";
            }
        }while(op_window);
        #endif
    }

    void entry_store() override{
        update_entry_ms();
    };

    void exit_store() override{
        // last_entry_ms = millis();
        update_entry_ms();
    };

    void entry_load() override{
        // last_entry_ms = millis();
        wait_for_done();
        update_entry_ms();
    };

    void exit_load() override{
        // last_entry_ms = millis();
        update_entry_ms();
    };
public:
    void init() override{};

    bool busy() override{return last_entry_ms + min_duration_ms - millis() > 0;}

    static constexpr uint8_t default_id = 0b10100000; 

    AT24C02(I2cDrv && _bus_drv):Storage(256), bus_drv(_bus_drv){;}
    AT24C02(I2c & _bus):Storage(256), bus_drv{_bus, default_id}{;}
    AT24C02(I2cDrv & _bus_drv):Storage(256), bus_drv(_bus_drv){;}
};

#endif
