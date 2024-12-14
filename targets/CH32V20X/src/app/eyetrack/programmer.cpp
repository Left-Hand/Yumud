

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


    AT24C64 eeprom{i2c};

    eeprom.init();
    auto mem = Memory(eeprom);

    const auto & arr = eeprom_data;
    DEBUG_PRINTLN(std::hex);
    scexpr size_t size = arr.size();

    static_assert(size == sizeof(arr));
    std::array<uint8_t, size> buf;


    mem.store(0, arr.begin(), arr.begin() + size);

    mem.load(0,buf.begin(), buf.begin() + size);

    std::vector<size_t> err_index;

    for(size_t i = 0; i < size; ++i){
        if(arr[i] != buf[i]){
            err_index.emplace_back(i);
        }
    }

    if(err_index.size()){
        for(auto i : err_index){
            DEBUG_PRINTLN("error");
            DEBUG_PRINTLN(i, arr[i], buf[i]);
        }
    }else{
        DEBUG_PRINTLN("success");
    }
    DEBUG_PRINTLN("over");
    while(true){
    }
}
