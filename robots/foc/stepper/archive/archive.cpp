#include "../stepper.hpp"


bool FOCStepper::loadArchive(const bool outen){
    using BoardInfo = StepperUtils::BoardInfo;
    Archive archive;
    memory.load(archive);

    bool match = true;
    bool abort = false;

    ARCHIVE_PRINTS("======");
    {
        const auto & board_info = archive.board_info;

        match &= board_info.is_latest();

        ARCHIVE_PRINTS("reading board information...");
        ARCHIVE_PRINTS(board_info);

        if(!match){
            ARCHIVE_PRINTS("!!!board does not match current build!!!");
            ARCHIVE_PRINTS("we suggest you to save data once to cover useless data by typing\r\n->save");

            if(board_info.is_invalid()){
                abort = true;
                ARCHIVE_PRINTS("!!!board eeprom seems to be trash!!!\r\n");
            }else if(board_info.is_empty()){
                abort = true;
                ARCHIVE_PRINTS("!!!board eeprom is empty!!!\r\n");
            }

            ARCHIVE_PRINTS("current build is:");
            
            BoardInfo temp_board_info;
            temp_board_info.reset();
            ARCHIVE_PRINTS(temp_board_info);
        }else{
            ARCHIVE_PRINTS("board matches current build");
        }
    }

    if(!abort){
        for(size_t i = 0; i < odo.map().size(); i++){
            int16_t item_i = archive.cali_map[i];
            odo.map()[i] = real_t(item_i) / 16384;
            elecrad_zerofix = 0;
        }

        // setNodeId(archive.node_id);
        
        memcpy(&archive_, &archive, sizeof(archive));
        ARCHIVE_PRINTS("load successfully!");
    }else{
        ARCHIVE_PRINTS("load aborted because data is corrupted");
    }
    ARCHIVE_PRINTS("======");
    return (!abort);
}

void FOCStepper::saveArchive(const bool outen){
    Archive archive;

    ARCHIVE_PRINTS("======");
    ARCHIVE_PRINTS("generating archive...");
    ARCHIVE_PRINTS("current board info:");
    ARCHIVE_PRINTS(archive.board_info);

    memcpy(&archive, &archive_, sizeof(Archive));
    uint32_t hashcode = archive.hash();
    archive.hashcode = hashcode;

    for(size_t i = 0; i < odo.map().size(); i++){
        scexpr auto ratio = real_t(1 / TAU);
        auto item_i = int16_t((odo.map()[i] - (elecrad_zerofix / poles * ratio)) * 16384);
        archive.cali_map[i] = item_i;
    }

    archive.node_id = uint8_t(node_id);

    ARCHIVE_PRINTS("generate done");
    ARCHIVE_PRINTS("hash of archive is ", hashcode);

    ARCHIVE_PRINTS("saving archive...");
    ARCHIVE_PRINTS("please keep power supporting");

    memory.store(archive);

    ARCHIVE_PRINTS("save done, veritfing");

    ARCHIVE_PRINTS("there is no verification currently");

    ARCHIVE_PRINTS("verification done");
    ARCHIVE_PRINTS("======");
}


void FOCStepper::removeArchive(const bool outen){
    Archive archive;
    archive.clear();

    ARCHIVE_PRINTS("======");
    ARCHIVE_PRINTS("removing archive...");

    memory.store(archive);

    ARCHIVE_PRINTS("done");
    ARCHIVE_PRINTS("======");
}


OutputStream & operator<<(OutputStream & os, const StepperUtils::BoardInfo & bi){
    #ifdef ARCHIVE_PRINTS
    os << "======\r\n";
    os << "build version:\t\t" << bi.bver << "\r\n";
    os << "build time:\t\t20" << 
            bi.y << '/' << bi.m << '/' << 
            bi.d << '\t' << bi.h << ':' << bi.mi << "\r\n";

    os << "driver type:\t\t" << bi.dtype << "\r\n";
    os << "driver branch:\t\t" << bi.dbranch << "\r\n";
    os << "======\r\n";
    #endif
    return os;
}

#undef ARCHIVE_PRINTS