#include "../stepper.hpp"


bool Stepper::loadArchive(const bool outen){
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
            
            BoardInfo m_board_info;
            m_board_info.construct();
            ARCHIVE_PRINTS(m_board_info);
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

        setNodeId(archive.node_id);
        ARCHIVE_PRINTS("load successfully!");
    }else{
        ARCHIVE_PRINTS("load aborted because data is corrupted");
    }
    ARCHIVE_PRINTS("======");
    return (!abort);
}

void Stepper::saveArchive(const bool outen){
    Archive archive;

    static_assert(sizeof(Archive) <= 256, "archive size overflow");

    ARCHIVE_PRINTS("======");
    ARCHIVE_PRINTS("generating archive...");
    ARCHIVE_PRINTS("current board info:");
    ARCHIVE_PRINTS(archive.board_info);

    archive.board_info.construct();
    archive.switches = m_switches;
    uint32_t hashcode = archive.hash();
    archive.hashcode = hashcode;

    for(size_t i = 0; i < odo.map().size(); i++){
        auto item_i = int16_t((odo.map()[i] - (elecrad_zerofix / real_t(poles * TAU))) * 16384);
        archive.cali_map[i] = item_i;
    }

    archive.node_id = node_id;

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


void Stepper::removeArchive(const bool outen){
    Archive archive;
    archive.clear();
    ARCHIVE_PRINTS("======");
    ARCHIVE_PRINTS("removing archive...");

    memory.store(archive);

    ARCHIVE_PRINTS("done");
    ARCHIVE_PRINTS("======");
}

#undef ARCHIVE_PRINTS

OutputStream & operator<<(OutputStream & os, const StepperUtils::BoardInfo & bi){
    os << "build version:\t\t" << bi.bver << "\r\n";
    os << "build time:\t\t20" << 
            bi.y << '/' << bi.m << '/' << 
            bi.d << '\t' << bi.h << ':' << bi.mi << "\r\n";

    os << "driver type:\t\t" << bi.dtype << "\r\n";
    os << "driver branch:\t\t" << bi.dbranch << "\r\n";
    return os;
}
