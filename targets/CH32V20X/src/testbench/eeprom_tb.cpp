#include "tb.h"
#include "../drivers/Memory/EEprom/AT24CXX/at24cxx.hpp"
#include "../drivers/Memory/Flash/W25QXX/w25q16.hpp"
#include "../hal/flash/flash.hpp"

#include "../algo/random/random_generator.hpp"
#include "../hal/bkp/bkp.hpp"

#define MEMORY_TB_FIRSTBYTE
#define MEMORY_TB_SEVERLBYTES
// #define MEMORY_TB_WHOLECHIP
// #define MEMORY_TB_RANDOM

// #define MEMORY_TB_PIECES
#define MEMORY_TB_CONTENT

#define MEMORY_TB_ASSERT(x, s, a, b)\
if((x) == false) {logger.prints("!!!FAILED[", s, ']', std::hex, std::showbase, a, "->", b); delay(1); CREATE_FAULT;}\
else {logger.prints("Succeed[", s, ']', std::hex, std::showbase, a, "->", b);}\

struct TestData{
    std::array<uint8_t, 4> data = {0x5a,0xa5,0x54,00};
    char name[8] = "Rstr1aN";
    real_t value = real_t(0.1);
    uint32_t crc = 0x08;
};

static void mem_tb(OutputStream & logger, Memory & mem){
    [[maybe_unused]] bool passflag = true;



    logger.prints("start memory testbench \r\nmem size is", mem.size());

    #ifdef MEMORY_TB_FIRSTBYTE
    {
        scexpr uint8_t before_orignal = 0x37;
        mem.store(before_orignal);
        uint8_t before;
        mem.load(before);
        MEMORY_TB_ASSERT(before_orignal == before, "specified byte", before_orignal, before);
        mem.store(uint8_t(before + 1));
        uint8_t after;
        mem.load(after);
        MEMORY_TB_ASSERT(before + 1 == after, "firts byte tb", before, after);
    }
    #endif

    #ifdef MEMORY_TB_SEVERLBYTES
    {
        auto before = Sys::Chip::getChipIdCrc();
        mem.store(before);

        decltype(before) after;
        mem.load(after);

        MEMORY_TB_ASSERT(before == after, "crc bytes tb", before, after);
    }
    #endif

    #ifdef MEMORY_TB_RANDOM
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
            logger.prints("random tb failed");
        }else{
            logger.prints("random tb passed");
        }
    }
    #endif

    #ifdef MEMORY_TB_WHOLECHIP
    {
        constexpr auto begin_addr = 0;


        uint8_t data_before[256];
        for(size_t i = 0; i < sizeof(data_before);i++) data_before[i] = i;
        uint8_t data_after[sizeof(data_before)] = {0};

        passflag &= (0 != memcmp(data_before, data_after, sizeof(data_before)));
        MEMORY_TB_ASSERT(passflag, "wholechip tb failed:", "initial data the same");

        mem.store(data_before, begin_addr);
        mem.load(data_after, begin_addr);

        passflag &= (0 == memcmp(data_before, data_after, sizeof(data_before)));

        if(!passflag){
            logger.prints("wholechip tb failed:", "data not the same")

            logger.prints("data wrote");
            for(auto & item : data_before){
                logger.prints("\t\t", item);
            }

            logger.prints("data read");
            for(auto & item : data_after){
                logger.prints("\t\t", item);
            }

            MEMORY_TB_ASSERT(false, "wholechip tb terminated");
        };
        logger.prints("wholechip tb passed");
    }
    #endif

    #ifdef MEMORY_TB_PIECES
    {
        logger.prints("muti store begin");
        constexpr auto begin_addr = 7;
        // constexpr auto end_addr = 15;
        uint8_t data[] = {3, 1, 2, 4};
        uint8_t data2[sizeof(data)];

        Memory mem = {mem};
        mem.store(data, begin_addr);
        mem.load(data2, begin_addr);
        for(const auto & item : data2){
            logger.prints("data_read", int(item));
        }
    }
    #endif

    #ifdef MEMORY_TB_CONTENT
    {

        logger.prints("content tb");
        TestData temp;

        logger.prints("write", temp);
        mem.store(temp);
        temp.value = real_t(0.2);
        temp.crc = 0x12;
        // temp.name = "rstr1aN";
        temp.data = {22,44,6,78};
        // temp.data.fill(0x78);
        // memset(&temp, 'a', sizeof(temp));
        logger.prints("modi", temp);
        mem.load(temp);
        logger.prints("read", temp);

    }
    #endif

    logger.prints("at24 tb done");
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
    // Memory mem = at24.slice({256,512});
    Memory mem = at24.slice({0,256});
    mem_tb(logger, mem);
}

// static void flash_tb(OutputStream & logger){
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

    logger.setSplitter(" ");
    logger.setRadix(16);

    logger.prints("Flash Size:", Sys::Chip::getFlashSize());
    logger.prints("../sys Clock:", Sys::Clock::getSystemFreq());
    logger.prints("AHB Clock:", Sys::Clock::getAHBFreq());
    logger.prints("APB1 Clock:", Sys::Clock::getAPB1Freq());
    logger.prints("APB2 Clock:", Sys::Clock::getAPB2Freq());

    TestData temp;

    flash.load(temp);
    if(temp.data[0] == 0x39){
        logger.prints("need to store new");
        logger.prints("new data is");
        TestData new_temp = TestData();
        logger.prints(new_temp.data[0], new_temp.data[1], new_temp.data[2], new_temp.data[3]);
        flash.store(new_temp);
        flash.load(temp);
        logger.prints("new store done");
    }

    logger.prints("data loaded");
    logger.prints(temp.data[0], temp.data[1], temp.data[2], temp.data[3]);
    logger.prints(temp.name);
    logger.prints(temp.value);
    logger.prints(temp.crc);

    logger.prints("init bkp");
    bkp.init();
    bkp[1] = bkp[1] + 1;
    temp.crc = bkp[1];
    flash.store(temp);
    logger.prints("flash tb done");

    // flash.load(temp);
    // logger.prints();
    // if(temp.data[0] == 0xE3 || temp.data[0] == 0x39 || (bkp.readData(1) & 0b11) == 0){
    //     logger.prints(temp.data[0]);

    //     temp.data[3] = bkp.readData(1);
    //     temp.name[2] = 'k';
    //     // temp.value = real_t(0.2);
    //     temp.crc = 10;

    //     logger.prints(temp.data[0], temp.data[1], temp.data[2], temp.data[3]);
    //     logger.prints(temp.name);
    //     logger.prints(temp.value);
    //     logger.prints(temp.crc);
    //     flash.store(temp);
    //     flash.load(temp);
    // }else{
    //     logger.prints("suss");
    //         logger.prints(temp.data[0], temp.data[1], temp.data[2], temp.data[3]);
    // logger.prints(temp.name);
    // logger.prints(temp.value);
    // logger.prints(temp.crc);
    // }


    // }
    // logger.prints(temp.data[0], temp.data[1], temp.data[2], temp.data[3]);
    // logger.prints(temp.name);
    // logger.prints(temp.value);
    // logger.prints(temp.crc);

    // 
}

void eeprom_main(){
    DEBUGGER.init(DEBUG_UART_BAUD, CommMethod::Blocking);
    auto logger = DEBUGGER;
    logger.setEps(2);
    logger.setRadix(10);
    logger.setSplitter("\t\t");

    I2cSw i2csw = I2cSw(portD[1], portD[0]);
    i2csw.init(400000);

    eeprom02_tb(logger, i2csw);
    // flash_tb(logger);
}

OutputStream & operator << (OutputStream & os, const TestData & td){
    return os << td.data << ',' << td.name << ',' << td.value << ',' << td.crc;
}