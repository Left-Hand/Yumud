#pragma once

#include<cstdint>


namespace ymd{


enum class JedecManufacturer:uint8_t{
    Cypress = 0x01,
    Fujitsu = 0x04,
    Eon = 0x1C,
    Atmel = 0x1F,
    Micron = 0x20,
    Amic = 0x37,
    NorMem = 0x52,
    Sanyo = 0x62,
    Intel = 0x89,
    Esmt = 0x8C,
    Fudan = 0xA1,
    Hyundai = 0xAD,
    SST = 0xBF,
    Macronix = 0xC2,
    GigaDevice = 0xC8,
    Issi = 0xD5,
    Winbond = 0xEF,
    Puya = 0x85
};

enum class JedecStorageType:uint8_t{
    SRAM = 0x01,
    NOR_Flash = 0x02,
    NAND_Flash = 0x04,
    Fast_Read_Flash = 0x08,
    Sector_Erase_Flash = 0x10,
    Block_Erase_Flash = 0x20,
    Page_Program_Flash = 0x40,
};

class OutputStream;

OutputStream & operator<<(OutputStream & os, const JedecManufacturer man);
OutputStream & operator<<(OutputStream & os, const JedecStorageType type);
}
// ymd::OutputStream & operator<<(ymd::OutputStream & os, const ymd::JedecManufacturer man);
// ymd::OutputStream & operator<<(ymd::OutputStream & os, const ymd::JedecStorageType type);