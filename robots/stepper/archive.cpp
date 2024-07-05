#include "stepper.hpp"



void Stepper::loadArchive(){
    using Archive = StepperUtils::Archive;
    Archive archive;
    memory.load(archive);

    bool match = true;
    bool corrupt = false;

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
                corrupt = true;
                logger.println("!!!board eeprom seems to be trash!!!\r\n");
            }else if(board_info.empty()){
                corrupt = true;
                logger.println("!!!board eeprom is empty!!!\r\n");
            }

            logger.println("current build is:");
            board_info.construct();
            board_info.printout(logger);
        }else{
            logger.println("\r\nboard matches current build");
        }
    }

    if(!corrupt){
        for(size_t i = 0; i < odo.map().size(); i++){
            odo.map()[i] = real_t(archive.cali_map[i]) / 16384;
            elecrad_zerofix = 0;
            // cali_map in archive is q16
        }
        logger.println("load successfully!");
    }else{
        logger.println("load aborted because data is corrupted");
    }
    logger.println("======");
}

bool Stepper::autoload(){
    using Archive = StepperUtils::Archive;
    Archive archive;
    memory.load(archive);

    bool en = true;

    // en &= (archive.hash() == archive.hashcode);
    if(!en) return false;

    // auto m_switches = archive.switches;
    // en &= (m_switches.cali_done); // if cali done, continue
    // en &= !(m_switches.cali_every_pwon);// if cali is forced when power on, break;
    // en &= !(m_switches.cali_when_update && !archive.board_info.match());// if update cali enabled and version out, break; 
    if(!en) return false;

    for(size_t i = 0; i < odo.map().size(); i++){
        odo.map()[i] = real_t(archive.cali_map[i]) / 16384;
        elecrad_zerofix = 0;
    }

    return true;
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
    archive.hashcode = hashcode;

    for(size_t i = 0; i < odo.map().size(); i++){
        archive.cali_map[i] = uint16_t((odo.map()[i] + (elecrad_zerofix / real_t(poles * TAU))) * 16384);
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


void Stepper::removeArchive(){
    using Archive = StepperUtils::Archive;
    Archive archive;
    archive.clear();
    logger.println("======");
    logger.println("removing archive...");

    memory.store(archive);

    logger.println("done");
    logger.println("======");
}
