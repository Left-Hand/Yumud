// #ifndef __AT24C64_HPP__

// #define __AT24C64_HPP__

// #include "drivers/device_defs.h"

// #ifdef AT24C64_DEBUG
// #define AT24C64_DEBUG(...) DEBUG_LOG(__VA_ARGS__)
// #else
// #define AT24C64_DEBUG(...)
// #endif

// class AT24C64:public Storage{
// protected:
//     I2cDrv & bus_drv;
//     const uint32_t chip_size = 256;
//     const uint32_t max_page = 8;
// public:
//     AT24C64(I2cDrv & _bus_drv): bus_drv(_bus_drv){;}

//     void writeData(const uint8_t addr, const uint8_t * data, uint8_t len){
//         if(addr + len > chip_size || len > max_page){
//             AT24C64_DEBUG("AT24C64: writeData: invalid addr or len");
//             len = MIN(chip_size, addr + len) - addr;
//         }
//         bus_drv.write({(uint8_t)(addr >> 8), (uint8_t)(addr)}, false);
//         bus_drv.write(data, len);
//     }

//     void writeData(const uint8_t addr, const uint8_t & data){
//         AT24C64_DEBUG("AT24C64 write: ", addr, data);
//         writeData(addr, &data, 1);
//     }

//     void readData(const uint8_t addr, uint8_t * data, const size_t len){
//         if(addr + len > chip_size || len > max_page){
//             AT24C64_DEBUG("AT24C64: readData: invalid addr or len");
//             len = MIN(chip_size, addr + len) - addr;
//         }
//         bus_drv.write({(uint8_t)(addr >> 8), (uint8_t)(addr)}, false);
//         bus_drv.write(data, len);
//     }

//     void readData(const uint8_t  addr, uint8_t * data){
//         AT24C64_DEBUG("AT24C64 read: ", addr, *data);
//         readData(addr, data, 1);
//     }


//     void wait_for_done(){
//         uint32_t delays;
//         if(last_entry_ms == 0){
//             delays = min_duration_ms;
//             update_entry_ms();
//         }else{
//             // delays = MAX(last_entry_ms + min_duration_ms - millis(), 0);
//             delays = min_duration_ms;
//         }

//         // AT24C02_DEBUG("wait for", delays, "ms");
//         delay(delays);
//     }

//     void update_entry_ms(){
//         last_entry_ms = millis();
//     }

//     void _store(const void * data, const Address data_size, const Address loc) override {
//         auto full_end = loc + data_size; 
//         if(full_end > chip_size){
//             AT24C02_DEBUG("invalid addr at:", loc);
//             return;
//         }


//         // auto chip_window = window();
//         AddressWindow store_window = AddressWindow{loc,loc + data_size};
//         AddressWindow op_window = {0,0};
//         AT24C02_DEBUG("multi store entry", store_window);
//         do{
//             op_window = store_window.grid_forward(op_window, page_size);
//             if(op_window){
//                 uint8_t * ptr = ((uint8_t *)data + (op_window.start - store_window.start));
//                 // DEBUGGER << op_window;
//                 // for(uint32_t i = op_window.start; i < op_window.end; i++) DEBUGGER << ',' << ((uint8_t*)data)[i];
//                 // DEBUGGER << "\r\n";
//                 wait_for_done();
//                 bus_drv.writePool(op_window.start, ptr, op_window.length());
//                 update_entry_ms();
//             }
//         }while(op_window);
//     }

//     void _load(void * data, const Address data_size, const Address loc) override {
//         auto full_end = loc + data_size; 
//         if(full_end > chip_size){
//             AT24C02_DEBUG("invalid addr at:", loc);
//             return;
//         }

//         bus_drv.readPool(loc, (uint8_t *)data, data_size);

//         // #ifdef AT24C02_DEBUG
//         // AddressWindow store_window = AddressWindow{loc,loc + data_size};
//         // AddressWindow op_window = {0,0};
//         // AT24C02_DEBUG("check entry", store_window);
//         // do{
//         //     op_window = store_window.grid_forward(op_window, page_size);
//         //     if(op_window){
//         //         DEBUGGER << op_window;
//         //         for(uint32_t i = op_window.start; i < op_window.end; i++) DEBUGGER << ',' << ((uint8_t*)data)[i];
//         //         DEBUGGER << "\r\n";
//         //     }
//         // }while(op_window);
//         // #endif
//     }

//     void entry_store() override{
//         update_entry_ms();
//     };

//     void exit_store() override{
//         // last_entry_ms = millis();
//         update_entry_ms();
//     };

//     void entry_load() override{
//         // last_entry_ms = millis();
//         wait_for_done();
//         update_entry_ms();
//     };

//     void exit_load() override{
//         // last_entry_ms = millis();
//         update_entry_ms();
//     };
// };

// #endif
