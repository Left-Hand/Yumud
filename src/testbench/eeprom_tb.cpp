#include "tb.h"
#include "../drivers/Memory/EEprom/AT24CXX/at24cxx.hpp"
#include "../drivers/Memory/Flash/W25QXX/w25qxx.hpp"
#include "../hal/flash/flash.hpp"

#include "algo/random/random_generator.hpp"
#include "hal/bkp/bkp.hpp"

#define EEPROM_TB_FIRSTBYTE
#define EEPROM_TB_SEVERLBYTES
// #define EEPROM_TB_WHOLECHIP
#define EEPROM_TB_RANDOM

// #define EEPROM_TB_PIECES
#define EEPROM_TB_CONTENT

#ifdef AT24CXX_DEBUG
#undef AT24CXX_DEBUG
#define AT24CXX_DEBUG(...) DEBUG_PRINT(SpecToken::Space, ##__VA_ARGS__, "\t|", __PRETTY_FUNCTION__);
#else
#define AT24CXX_DEBUG(...)
#endif

struct Temp{
    uint8_t data[4] = {0,1,2,3};
    char name[8] = "Rstr1aN";
    real_t value = real_t(0.1);
    uint8_t crc = 0x08;
};

static void mem_tb(OutputStream & logger, Memory & mem){
    [[maybe_unused]] bool passflag = true;
    logger.println("mem size is", mem.size());

    #ifdef EEPROM_TB_FIRSTBYTE
    {
        uint8_t before = 22;
        mem.load(before);
        // if(before == 0xff) before = 0;
        mem.store(uint8_t(before + 1));
        uint8_t after;
        mem.load(after);
        ASSERT_WITH_DOWN(before + 1 == after, "firstbyte tb failed", before, "->", after);
        DEBUG_PRINT("firstbyte tb passed", before, "->", after);
    }
    #endif

    #ifdef EEPROM_TB_SEVERLBYTES
    {
        auto before = Sys::Chip::getChipIdCrc();
        mem.store(before);

        decltype(before) after;
        mem.load(after);

        ASSERT_WITH_DOWN(before == after, "svbytes tb failed: ", toString(before,16), "->", toString(after, 16));
        DEBUG_PRINT("svbytes tb passed");
    }
    #endif

    #ifdef EEPROM_TB_RANDOM
    {
        RandomGenerator rnd;
        rnd.init();
        for(size_t i = 0; i < mem.size(); i++){
            mem.store((uint8_t)rnd.update(), i);
        }

        rnd.init();
        for(size_t i = 0; i < mem.size(); i++){
            passflag &= (mem.load<uint8_t>(i) == (uint8_t)rnd.update());
        }

        if(!passflag){
            DEBUG_PRINT("random tb failed");
        }else{
            DEBUG_PRINT("random tb passed");
        }
    }
    #endif

    #ifdef EEPROM_TB_WHOLECHIP
    {
        constexpr auto begin_addr = 0;


        uint8_t data_before[256];
        for(size_t i = 0; i < sizeof(data_before);i++) data_before[i] = i;
        uint8_t data_after[sizeof(data_before)] = {0};

        passflag &= (0 != memcmp(data_before, data_after, sizeof(data_before)));
        ASSERT_WITH_DOWN(passflag, "wholechip tb failed:", "initial data the same");

        mem.store(data_before, begin_addr);
        mem.load(data_after, begin_addr);

        passflag &= (0 == memcmp(data_before, data_after, sizeof(data_before)));

        if(!passflag){
            DEBUG_PRINT("wholechip tb failed:", "data not the same")

            DEBUG_PRINT("data wrote");
            for(auto & item : data_before){
                DEBUG_PRINT("\t\t", item);
            }

            DEBUG_PRINT("data read");
            for(auto & item : data_after){
                DEBUG_PRINT("\t\t", item);
            }

            ASSERT_WITH_DOWN(false, "wholechip tb terminated");
        };
        DEBUG_PRINT("wholechip tb passed");
    }
    #endif

    #ifdef EEPROM_TB_PIECES
    {
        DEBUG_PRINT("muti store begin");
        constexpr auto begin_addr = 7;
        // constexpr auto end_addr = 15;
        uint8_t data[] = {3, 1, 2, 4};
        uint8_t data2[sizeof(data)];

        Memory mem = {mem};
        mem.store(data, begin_addr);
        mem.load(data2, begin_addr);
        for(const auto & item : data2){
            logger.println("data_read", int(item));
        }
    }
    #endif

    #ifdef EEPROM_TB_CONTENT
    {

        logger.println("struct tb");
        Temp temp;
        mem.store(temp);
        temp.value = 0.2;
        logger.println(temp.value);
        mem.load(temp);
        logger.println(temp.value);
    }
    #endif

    logger.println("at24 tb done");
}

[[maybe_unused]] static void eeprom02_tb(OutputStream & logger, I2c & i2c){
    AT24C02 at24{i2c};
    at24.init();
    Memory mem = at24;
    mem_tb(logger, mem);
}

[[maybe_unused]] static void eeprom64_tb(OutputStream & logger, I2c & i2c){
    AT24C64 at24{i2c};
    at24.init();
    Memory mem = at24.slice({256,512});
    mem_tb(logger, mem);
}

// static void flash_tb(OutputStream & logger){
//     // AT24C02 at24{i2c};
//     Flash flash{0};
//     flash.init();
//     Memory mem = flash;
//     mem_tb(logger, mem);
// }


[[maybe_unused]] static void flash_tb(IOStream & logger){

    Flash sto(-1);
    sto.init();
    Memory flash = sto;
    Sys::Misc::prework();

    logger.setSpace(" ");
    logger.setRadix(16);

    logger.println("Flash Size:", Sys::Chip::getFlashSize());
    logger.println("../sys Clock:", Sys::Clock::getSystemFreq());
    logger.println("AHB Clock:", Sys::Clock::getAHBFreq());
    logger.println("APB1 Clock:", Sys::Clock::getAPB1Freq());
    logger.println("APB2 Clock:", Sys::Clock::getAPB2Freq());

    Temp temp;

    flash.load(temp);
    if(temp.data[0] == 0x39){
        logger.println("need to store new");
        logger.println("new data is");
        Temp new_temp = Temp();
        logger.println(new_temp.data[0], new_temp.data[1], new_temp.data[2], new_temp.data[3]);
        flash.store(new_temp);
        flash.load(temp);
        logger.println("new store done");
    }

    logger.println("data loaded");
    logger.println(temp.data[0], temp.data[1], temp.data[2], temp.data[3]);
    logger.println(temp.name);
    logger.println(temp.value);
    logger.println(temp.crc);

    logger.println("init bkp");
    bkp.init();
    bkp.writeData(1, bkp.readData(1) + 1);
    temp.crc = bkp.readData(1);
    flash.store(temp);
    logger.println("flash tb done");

    // flash.load(temp);
    // logger.println();
    // if(temp.data[0] == 0xE3 || temp.data[0] == 0x39 || (bkp.readData(1) & 0b11) == 0){
    //     logger.println(temp.data[0]);

    //     temp.data[3] = bkp.readData(1);
    //     temp.name[2] = 'k';
    //     // temp.value = real_t(0.2);
    //     temp.crc = 10;

    //     logger.println(temp.data[0], temp.data[1], temp.data[2], temp.data[3]);
    //     logger.println(temp.name);
    //     logger.println(temp.value);
    //     logger.println(temp.crc);
    //     flash.store(temp);
    //     flash.load(temp);
    // }else{
    //     logger.println("suss");
    //         logger.println(temp.data[0], temp.data[1], temp.data[2], temp.data[3]);
    // logger.println(temp.name);
    // logger.println(temp.value);
    // logger.println(temp.crc);
    // }


    // }
    // logger.println(temp.data[0], temp.data[1], temp.data[2], temp.data[3]);
    // logger.println(temp.name);
    // logger.println(temp.value);
    // logger.println(temp.crc);

    // 
}

void eeprom_main(){
    uart1.init(576000, CommMethod::Blocking);
    auto & logger = uart1;
    logger.setEps(4);
    logger.setRadix(10);
    logger.setSpace(",");

    I2cSw i2csw = I2cSw(portD[1], portD[0]);
    i2csw.init(400000);

    eeprom64_tb(logger, i2csw);
    // flash_tb(logger);
}