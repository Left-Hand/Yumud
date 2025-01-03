#include "../stepper.hpp"
#include "archive.hpp"
//FIXME need to remove
#define ARCHIVE_PRINTS(...) DEBUG_PRINTLN(__VA_ARGS__)


using namespace ymd::foc;
using namespace ymd;

static void getDefaultArchive(MotorUtils::Archive & archive){
    archive.pos_config = {
        // .kp = real_t(3),
        // .kd = 170

        .kp = real_t(2.3),
        .kd = 40
        // .kp = real_t(3),
        // .kd = 70

        //OVERSPEED
        // .kp = real_t(1.8),
        // .kd = 20

        // .kp = 6,
        // .kd = 116
        
        // .kp = 5,
        // .kd = 23*7
    };

    archive.spd_config = {
        // .kp = real_t(205.0/256),
        .kp = 7,
        .kd = 70,
        // .kd = 0,

        // .kp = 8,
        // .kd = 30,
    };
}

bool FOCStepper::loadArchive(){
    using BoardInfo = MotorUtils::BoardInfo;

    Archive archive;
    memory.load(0,archive);

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
        odo.decompress(archive.cali_map);
        // setNodeId(archive.node_id);

        getDefaultArchive(archive);
        std::swap(archive_, archive);
        ARCHIVE_PRINTS("load successfully!");
    }else{
        // std::swap(archive_, getDefaultArchive());
        getDefaultArchive(archive_);
        ARCHIVE_PRINTS("load aborted because data is corrupted");
    }
    ARCHIVE_PRINTS("======");
    return (!abort);
}

void FOCStepper::saveArchive(){
    Archive archive;
    archive.board_info.reset();

    archive.hashcode = archive.hash();

    {
        auto map = odo.compress(meta.radfix);
        for(size_t i = 0; i < map.size(); i++){
            archive.cali_map[i] = map[i];
        }
    }

    archive.node_id = uint8_t(node_id);
    memory.store(0,archive);
}


void FOCStepper::removeArchive(){
    memory.store(0,Archive());
}

OutputStream &ymd::operator<<(OutputStream &os, const foc::MotorUtils::BoardInfo &bi){
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


