#include "../stepper.hpp"

#define ARCHIVE_LOG(...) if(outen) logger.prints(__VA_ARGS__);

bool Stepper::loadArchive(const bool outen){
    using BoardInfo = StepperUtils::BoardInfo;
    Archive archive;
    memory.load(archive);

    bool match = true;
    bool abort = false;

    ARCHIVE_LOG("======");
    {
        const auto & board_info = archive.board_info;

        match &= board_info.is_latest();

        ARCHIVE_LOG("reading board information...");
        ARCHIVE_LOG(board_info);

        if(!match){
            ARCHIVE_LOG("!!!board does not match current build!!!");
            ARCHIVE_LOG("we suggest you to save data once to cover useless data by typing\r\n->save");

            if(board_info.is_invalid()){
                abort = true;
                ARCHIVE_LOG("!!!board eeprom seems to be trash!!!\r\n");
            }else if(board_info.is_empty()){
                abort = true;
                ARCHIVE_LOG("!!!board eeprom is empty!!!\r\n");
            }

            ARCHIVE_LOG("current build is:");
            
            BoardInfo m_board_info;
            m_board_info.construct();
            ARCHIVE_LOG(m_board_info);
        }else{
            ARCHIVE_LOG("\r\nboard matches current build");
        }
    }

    if(!abort){
        for(size_t i = 0; i < odo.map().size(); i++){
            int16_t item_i = archive.cali_map[i];
            odo.map()[i] = real_t(item_i) / 16384;
            elecrad_zerofix = 0;
        }

        setNodeId(archive.node_id);
        ARCHIVE_LOG("load successfully!");
    }else{
        ARCHIVE_LOG("load aborted because data is corrupted");
    }
    ARCHIVE_LOG("======");
    return (!abort);
}

void Stepper::saveArchive(const bool outen){
    Archive archive;

    static_assert(sizeof(Archive) <= 256, "archive size overflow");

    ARCHIVE_LOG("======");
    ARCHIVE_LOG("generating archive...");
    ARCHIVE_LOG("current board info:");
    ARCHIVE_LOG(archive.board_info);

    archive.board_info.construct();
    archive.switches = m_switches;
    uint32_t hashcode = archive.hash();
    archive.hashcode = hashcode;

    for(size_t i = 0; i < odo.map().size(); i++){
        auto item_i = int16_t((odo.map()[i] - (elecrad_zerofix / real_t(poles * TAU))) * 16384);
        archive.cali_map[i] = item_i;
    }

    archive.node_id = node_id;

    ARCHIVE_LOG("generate done");
    ARCHIVE_LOG("hash of archive is ", toString(hashcode, 16));

    ARCHIVE_LOG("saving archive...");
    ARCHIVE_LOG("please keep power supporting");

    memory.store(archive);

    ARCHIVE_LOG("save done, veritfing");

    ARCHIVE_LOG("there is no verification currently");

    ARCHIVE_LOG("verification done");
    ARCHIVE_LOG("======");
}


void Stepper::removeArchive(const bool outen){
    Archive archive;
    archive.clear();
    ARCHIVE_LOG("======");
    ARCHIVE_LOG("removing archive...");

    memory.store(archive);

    ARCHIVE_LOG("done");
    ARCHIVE_LOG("======");
}

#undef ARCHIVE_LOG

OutputStream & operator<<(OutputStream & os, const StepperUtils::BoardInfo & bi){
    os << "build version:\t\t" << bi.bver << "\r\n";
    os << "build time:\t\t20" << 
            bi.y << '/' << bi.m << '/' << 
            bi.d << '\t' << bi.h << ':' << bi.mi << "\r\n";

    os << "driver type:\t\t" << bi.dtype << "\r\n";
    os << "driver branch:\t\t" << bi.dbranch << "\r\n";
    return os;
}
