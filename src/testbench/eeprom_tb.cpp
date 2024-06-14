#include "tb.h"

// #define EEPROM_TB_FIRSTBYTE
// #define EEPROM_TB_SEVERLBYTES
#define EEPROM_TB_WHOLECHIP
// #define EEPROM_TB_PIECES

void eeprom_tb(IOStream & logger){

    logger.setEps(4);
    logger.setRadix(10);
    logger.setSpace("");

    I2cSw i2csw = I2cSw(portD[1], portD[0]);
    i2csw.init(400000);

    AT24C02 at24{I2cDrv(i2csw, AT24C02::default_id)};
    at24.init();

    Memory mem = at24;

    #ifdef EEPROM_TB_FIRSTBYTE
    {
        uint8_t before;
        mem.load(before, 0);
        if(before == 0xff) before = 0;
        mem.store(uint8_t(before + 1), 0);
        uint8_t after;
        mem.load(after, 0);
        ASSERT_WITH_DOWN(before + 1 == after, "firstbyte tb failed", before, "->", after);
        DEBUG_PRINT("firstbyte tb passed", before, "->", after);
    }
    #endif

    #ifdef EEPROM_TB_SEVERLBYTES
    {
        auto before = Sys::Chip::getChipIdCrc();
        mem.store(before, 0);

        decltype(before) after;
        mem.load(after, 0);

        ASSERT_WITH_DOWN(before == after, "svbytes tb failed: ", toString(before,16), "->", toString(after, 16));
        DEBUG_PRINT("svbytes tb passed");
    }
    #endif

    #ifdef EEPROM_TB_WHOLECHIP
    {
        constexpr auto begin_addr = 0;
        bool passflag = true;

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
}