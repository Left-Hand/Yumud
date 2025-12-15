#include "src/testbench/tb.h"

#include "core/debug/debug.hpp"

#include "hal/storage/flash/flash.hpp"
#include "hal/bkp/bkp.hpp"
#include "hal/bus/i2c/i2csw.hpp"
#include "hal/bus/i2c/i2cdrv.hpp"

#include "drivers/Storage/EEprom/AT24CXX/at24cxx.hpp"
#include "drivers/Storage/Flash/W25QXX/w25q16.hpp"

#include "algo/random/random_generator.hpp"

#include "core/math/real.hpp"
#include "hal/bus/uart/uarthw.hpp"
#include "hal/gpio/gpio_port.hpp"

using namespace ymd;

#if 0

using namespace ymd::drivers;

#define MEMORY_TB_FIRSTBYTE
#define MEMORY_TB_SEVERLBYTES
// #define MEMORY_TB_WHOLECHIP
// #define MEMORY_TB_RANDOM

// #define MEMORY_TB_PIECES
#define MEMORY_TB_CONTENT

#define MEMORY_TB_ASSERT(x, s, a, b)\
if((x) == false) {logger.prints("!!!FAILED[", s, ']', std::hex, std::showbase, a, "->", b); clock::delay(1ms); PANIC();}\
else {logger.prints("Succeed[", s, ']', std::hex, std::showbase, a, "->", b);}\

struct TestData{
    std::array<uint8_t, 4> data = {0x5a,0xa5,0x54,00};
    char name[8] = "Rstr1aN";
    real_t value = real_t(0.1);
    uint32_t crc = 0x08;

    friend OutputStream & operator<<(OutputStream & os, const TestData & td){
        os << "data:" << std::hex << td.data;
        os << std::dec << "name:" << td.name << "value:" << td.value << "crc:" << td.crc;
        return os;
    }
};

constexpr auto size = sizeof(TestData);
static void mem_tb(OutputStream & logger, Memory & mem){
    [[maybe_unused]] bool passflag = true;



    logger.prints("start memory testbench \r\nmem size is", mem.size());

    #ifdef MEMORY_TB_FIRSTBYTE
    {
        static constexpr uint8_t before_orignal = 0x37;
        // mem.store(0,before_orignal);
        uint8_t before;
        // mem.load(0,before);
        MEMORY_TB_ASSERT(before_orignal == before, "specified byte", before_orignal, before);
        // mem.store(0,uint8_t(before + 1));
        uint8_t after;
        // mem.load(0,after);
        MEMORY_TB_ASSERT(before + 1 == after, "firts byte tb", before, after);
    }
    #endif

    #ifdef MEMORY_TB_SEVERLBYTES
    {
        auto before = sys::chip::get_chip_id_crc();
        mem.store(0,before);

        decltype(before) after;
        mem.load(0,after);

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
        TestData archive;

        logger.prints("write", archive);
        // mem.store(0,archive);
        archive.value = real_t(0.2);
        archive.crc = 0x12;
        // archive.name = "rstr1aN";
        archive.data = {22,44,6,78};
        // archive.data.fill(0x78);
        // memset(&archive, 'a', sizeof(archive));
        logger.prints("modi", archive);
        // mem.load(0,archive);
        logger.prints("read", archive);

    }
    #endif

    logger.prints("at24 tb done");
}

[[maybe_unused]] static void eeprom02_tb(OutputStream & logger, hal::I2c & i2c){
    AT24C02 at24{i2c};
    at24.init();
    Memory mem = at24;
    mem_tb(logger, mem);
}

[[maybe_unused]] static void eeprom64_tb(OutputStream & logger, hal::I2c & i2c){
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


[[maybe_unused]] static void flash_tb(OutputStream & logger){
    logger.set_splitter(" ");
    logger.set_radix(10);

    // logger.prints("Flash Size:", Sys::Chip::getFlashSize());
    // logger.prints("SYS Clock:", Sys::Clock::getSystemFreq());
    // logger.prints("SYS Clock:", 14000000u);
    // logger.prints("AHB Clock:", Sys::Clock::getAHBFreq());
    // logger.prints("APB1 Clock:", Sys::Clock::getAPB1Freq());
    // logger.prints("APB2 Clock:", Sys::Clock::getAPB2Freq());

    // logger.set_radix(16);
    // while(true);
    // logger.prints(Flash::getMaxPages());
    // while(true);
    Flash sto(0);
    sto.init();
    Memory flash = sto.slice_pages(-1);
    sys::preinit();

    // logger.prints("ok");
    clock::delay(10ms);

    if(true){
        using Cnt = size_t;
        Cnt cnt = 0;
        flash.load(0,cnt);
        if(cnt > 100){
            DEBUG_PRINTLN("unknown cnt:", cnt);
            cnt = 0;
        }
        flash.store(0, cnt+1);
        DEBUG_PRINTLN(cnt);
        DEBUG_PRINTLN("done");
        while(true);
    }

    TestData archive;
    flash.load(0,archive);
    // if(archive.data[0] != 0x39){
    if(true){
        logger.prints("need to store new");
        TestData new_archive = TestData();
        logger.prints("new archive is", new_archive);
        logger.prints("start to store");
        flash.store(0, new_archive);
        flash.load(0, archive);
        logger.prints("new store done", archive);
        while(true);
    }else{
        logger.prints("data is:", archive.data[0]);
        logger.prints("new data is:", TestData());

    }
    while(true);
    logger.prints("data loaded");
    logger.prints(archive.data[0], archive.data[1], archive.data[2], archive.data[3]);
    logger.prints(archive.name);
    logger.prints(archive.value);
    logger.prints(archive.crc);

    logger.prints("init bkp");
    bkp.init();
    bkp[1] = bkp[1] + 1;
    archive.crc = bkp[1];
    flash.store(0,archive);
    logger.prints("flash tb done");

    // flash.load(archive);
    // logger.prints();
    // if(archive.data[0] == 0xE3 || archive.data[0] == 0x39 || (bkp.readData(1) & 0b11) == 0){
    //     logger.prints(archive.data[0]);

    //     archive.data[3] = bkp.readData(1);
    //     archive.name[2] = 'k';
    //     // archive.value = real_t(0.2);
    //     archive.crc = 10;

    //     logger.prints(archive.data[0], archive.data[1], archive.data[2], archive.data[3]);
    //     logger.prints(archive.name);
    //     logger.prints(archive.value);
    //     logger.prints(archive.crc);
    //     flash.store(archive);
    //     flash.load(archive);
    // }else{
    //     logger.prints("suss");
    //         logger.prints(archive.data[0], archive.data[1], archive.data[2], archive.data[3]);
    // logger.prints(archive.name);
    // logger.prints(archive.value);
    // logger.prints(archive.crc);
    // }


    // }
    // logger.prints(archive.data[0], archive.data[1], archive.data[2], archive.data[3]);
    // logger.prints(archive.name);
    // logger.prints(archive.value);
    // logger.prints(archive.crc);

    // 
}
#endif


void eeprom_main(){
    hal::uart2.init({
        .remap = hal::UART2_REMAP_PA2_PA3,
        .baudrate = 576000
    });
    DEBUGGER.retarget(&hal::uart2);
    DEBUGGER.set_eps(2);
    DEBUGGER.set_radix(10);
    DEBUGGER.set_splitter("\t");

    auto scl_pin = hal::PB<13>();
    auto sda_pin = hal::PB<12>();

    hal::I2cSw i2csw = hal::I2cSw{&scl_pin, &sda_pin};
    i2csw.init({400000});

    
    using namespace drivers;
    auto at24 = AT24CXX(
        AT24CXX::Config::AT24C02{}, 
        hal::I2cDrv{&i2csw, AT24CXX::DEFAULT_I2C_ADDR}
    );


    at24.init().examine();
    const auto pbuf = std::span("Hello Woenejroktrvnknkebifdjknbuijekdnh3nuicedjkrld!");
    auto pbuf2 = std::array<uint8_t, 30>{};
    
    // const auto pbuf = std::span("Hello");
    DEBUG_PRINTLN("pw");
    while(true){
        // at24.store_bytes(Address(8), 
        //     {reinterpret_cast<const uint8_t*>(pbuf.data()), pbuf.size()}).examine();

        at24.load_bytes(Address(8), 
            {(pbuf2.data()), pbuf2.size()}).examine();

        // for(size_t i = 0; i < 30; ++i){
        while(true){
            at24.poll().examine();
            clock::delay(1ms);
            if(at24.is_idle())
                break;
        }

        clock::delay(1000ms);
    }
    // eeprom02_tb(DEBUGGER, i2csw);
    // eeprom64_tb(DEBUGGER, i2csw);

    // flash_tb(logger);
    while(true);
}

// void flash_main(){
//     // DEBUGGER_INST.init(DEBUG_UART_BAUD, CommStrategy::Blocking);

//     DEBUGGER_INST.init({576000});
//     DEBUGGER.retarget(&DEBUGGER_INST);

//     // auto & led = hal::PA<8>();
//     // led.outpp();
//     // while(true){
//     //     DEBUG_PRINTLN(millis());
//     //     clock::delay(100ms);
//     //     led.toggle();
//     // }
//     flash_tb(DEBUGGER);
// }

// OutputStream & operator << (OutputStream & os, const TestData & td){
//     return os << td.data << ',' << td.name << ',' << td.value << ',' << td.crc;
// }

