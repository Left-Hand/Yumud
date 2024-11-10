#include "jedec.hpp"
#include "sys/stream/ostream.hpp"


using namespace yumud;

OutputStream & yumud::operator<<(OutputStream &os, const JedecManufacturer man){

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
        default:return os << "Unknown";
    }
}

OutputStream & yumud::operator<<(OutputStream &os, const JedecStorageType type){
    return os;
}
