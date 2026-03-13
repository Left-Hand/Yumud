#include "jedec.hpp"
#include "core/stream/ostream.hpp"


using namespace ymd;

OutputStream & ymd::operator<<(OutputStream &os, const JedecManufacturer man){

    switch (man) {
        case JedecManufacturer::Cypress:return os << "Cypress";
        case JedecManufacturer::Fujitsu:return os << "Fujitsu";
        case JedecManufacturer::Eon:return os << "Eon";
        case JedecManufacturer::Atmel:return os << "Atmel";
        case JedecManufacturer::Micron:return os << "Micron";
        case JedecManufacturer::Amic:return os << "Amic";
        case JedecManufacturer::NorMem:return os << "NorMem";
        case JedecManufacturer::Sanyo:return os << "Sanyo";
        case JedecManufacturer::Intel:return os << "Intel";
        case JedecManufacturer::Esmt:return os << "Esmt";
        case JedecManufacturer::Fudan:return os << "Fudan";
        case JedecManufacturer::Hyundai:return os << "Hyundai";
        case JedecManufacturer::SST:return os << "SST";
        case JedecManufacturer::Macronix:return os << "Macronix";
        case JedecManufacturer::GigaDevice:return os << "GigaDevice";
        case JedecManufacturer::Issi:return os << "Issi";
        case JedecManufacturer::Winbond:return os << "Winbond";
        case JedecManufacturer::Puya:return os << "Puya";
    }
    return os << "Unknown";
}

OutputStream & ymd::operator<<(OutputStream &os, const JedecStorageType type){
    switch(type){
        case JedecStorageType::Sram:
            return os << "Sram";
        case JedecStorageType::Nor:
            return os << "Nor";
        case JedecStorageType::Nand:
            return os << "Nand";
        case JedecStorageType::FastReadFlash:
            return os << "FastReadFlash";
        case JedecStorageType::SectorEraseFlash:
            return os << "SectorEraseFlash";
        case JedecStorageType::BlockEraseFlash:
            return os << "BlockEraseFlash";
        case JedecStorageType::PageProgramFlash:
            return os << "PageProgramFlash";
    }
    return os << "Unknown";
}
