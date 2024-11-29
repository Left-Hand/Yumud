

#include "algo/random/random_generator.hpp"
#include "hal/bkp/bkp.hpp"

#include "hal/bus/i2c/i2csw.hpp"
#include "hal/bus/i2c/i2cdrv.hpp"

#include "hal/bus/uart/uarthw.hpp"

#include "data.hpp"


#include "drivers/Memory/EEprom/AT24CXX/at24cxx.hpp"
#include "drivers/Memory/Flash/W25QXX/w25q16.hpp"
#include "hal/flash/flash.hpp"

using namespace ymd;
using namespace ymd::drivers;

void programmer_main(){
    DEBUGGER_INST.init(DEBUG_UART_BAUD, CommMethod::Blocking);
    auto & logger = DEBUGGER;
    logger.setEps(2);
    logger.setRadix(10);
    logger.setSplitter("\t\t");

    
    I2cSw       i2c{portD[2], portC[12]};
    i2c.init(400_KHz);


    // const auto & data = eeprom_data;
    AT24C64 eeprom{i2c};

    eeprom.init();
    auto mem = Memory(eeprom);


    // std::array<uint8_t, 4> arr = {1,2,3,4};
    const auto & arr = eeprom_data;
    DEBUG_PRINTLN(std::hex);
    scexpr size_t size = arr.size();

    static_assert(size == sizeof(arr));
    std::array<uint8_t, size> buf;


    mem.store(0, arr.begin(), arr.begin() + size);
        // mem.store(0,static_cast<const uint8_t *>(arr.begin()), static_cast<const uint8_t *>(arr.begin()) + size);
    // for(size_t i = 0; i < size; ++i){
        // mem.store(0,static_cast<const uint8_t *>(arr.begin()), static_cast<const uint8_t *>(arr.begin()) + 32);
        // delay(100);
        // mem.store(32,static_cast<const uint8_t *>(arr.begin() + 32), static_cast<const uint8_t *>(arr.begin()) + 64);
    // }
    // delay(20);

    mem.load(0,buf.begin(), buf.begin() + size);
    // for(size_t i = 0; i < size; ++i){
    //     mem.load(buf[i],i);
    // }

    std::vector<size_t> err_index;

    for(size_t i = 0; i < size; ++i){
        if(arr[i] != buf[i]){
            err_index.emplace_back(i);
        }
    }

    // DEBUGGER.print_arr(buf.begin(), 4);
    // while(true);
    // DEBUG_PRINTLN(arr);
    // DEBUG_PRINTLN(buf);
    // delay(100);


    // auto [it1, it2] = std::mismatch(arr.begin(), arr.end(), buf.begin());
    // if(it1 == arr.end()){
    //     DEBUG_PRINTLN("success");
    // }else{
        // DEBUG_PRINTLN("fail", std::distance(arr.begin(), it1));
        // DEBUGGER.print_arr(arr.begin(), size);
        // DEBUG_PRINTLN("\r\n");
        // DEBUGGER.print_arr(buf.begin(), size);
        // DEBUG_PRINTLN("\r\n");
        // delay(100);
        // DEBUG_PRINTLN("\r\n\r\n");
    if(err_index.size()){
        for(auto i : err_index){
            DEBUG_PRINTLN("error");
            DEBUG_PRINTLN(i, arr[i], buf[i]);
        }
    }else{
        DEBUG_PRINTLN("success");
    }
    DEBUG_PRINTLN(err_index);
    DEBUG_PRINTLN("over");
    // }
    while(true){
    }

    // std::array<int, 5> arr1 = {1, 2, 3, 4, 5};
    // std::array<int, 5> arr2 = {1, 2, 3, 4, 6};

    // auto [it1, it2] = std::mismatch(arr1.begin(), arr1.end(), arr2.begin());

    // if (it1 == arr1.end()) {
    //     std::cout << "Arrays are equal" << std::endl;
    // } else {
    //     std::cout << "First mismatch at index: " << std::distance(arr1.begin(), it1) << std::endl;
    //     std::cout << "arr1[" << std::distance(arr1.begin(), it1) << "] = " << *it1 << std::endl;
    //     std::cout << "arr2[" << std::distance(arr2.begin(), it2) << "] = " << *it2 << std::endl;
    // }
}
