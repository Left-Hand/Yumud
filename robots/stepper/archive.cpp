#include "stepper.hpp"



void Stepper::loadArchive(){
    using Archive = StepperUtils::Archive;
    Archive archive;
    memory.load(archive);

    bool match;

    logger.println("======");
    {
        auto board_info = archive.board_info;

        match = board_info.match();

        logger.println("reading board information...");
        board_info.printout(logger);

        if(!match){
            logger.println("!!!board does not match current build!!!");
            logger.println("we suggest you to save data once to cover useless data by typing\r\n->save");
            if(board_info.broken()){
                logger.println("!!!board eeprom seems to be trash!!!\r\n");
            }else if(board_info.empty()){
                logger.println("!!!board eeprom is empty!!!\r\n");
            }
            logger.println("current build is:");
            board_info.construct();
            board_info.printout(logger);
        }else{
            logger.println("\r\nboard matches current build");
        }
    }

    if(match){
        for(size_t i = 0; i < odo.cali_map.size(); i++){
            odo.cali_map[i] = (odo.cali_map[i]) / 65536;
            elecrad_zerofix = 0;
            // cali_map in archive is q16
        }
    }else{
        logger.println("load aborted because data is corrupted");
    }
    logger.println("======");
}

void Stepper::saveArchive(){
    using Archive = StepperUtils::Archive;
    Archive archive;

    static_assert(sizeof(Archive) <= 256, "archive size overflow");

    logger.println("======");
    logger.println("generating archive...");
    archive.board_info.construct();
    logger.println("current board info:");
    archive.board_info.printout(logger);


    archive.switches = switches;
    uint32_t hashcode = archive.hash();

    for(size_t i = 0; i < odo.cali_map.size(); i++){
        archive.cali_map[i] = int((odo.cali_map[i] + elecrad_zerofix) * 65536);
    }

    logger.println("generate done");
    logger << "hash of archive is " << toString(hashcode, 16) << "\r\n"; 

    logger.println("saving archive...");
    logger.println("please keep power supporting");

    memory.store(archive);

    logger.println("save done, veritfing");

    logger.println("there is no verification currently");

    logger.println("verification done");
    logger.println("======");
}

