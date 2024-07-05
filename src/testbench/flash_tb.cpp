#include "tb.h"

void flash_tb(IOStream & logger){

    Flash sto(-1);
    sto.init();
    Memory flash = sto;
    Sys::Misc::prework();

    logger.setSpace(" ");
    logger.setRadix(16);

    // logger.println("Flash Size:", Sys::Chip::getFlashSize());
    // logger.println("Sys Clock:", Sys::Clock::getSystemFreq());
    // logger.println("AHB Clock:", Sys::Clock::getAHBFreq());
    // logger.println("APB1 Clock:", Sys::Clock::getAPB1Freq());
    // logger.println("APB2 Clock:", Sys::Clock::getAPB2Freq());
    struct Temp{
        uint8_t data[4] = {0,1,2,3};
        char name[8] = "Rstr1aN";
        real_t value = real_t(0.1);
        uint8_t crc = 0x08;
    };
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
    bkp.init();
    bkp.writeData(1, bkp.readData(1) + 1);
    temp.crc = bkp.readData(1);
    flash.store(temp);
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