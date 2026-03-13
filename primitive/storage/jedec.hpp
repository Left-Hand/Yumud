#pragma once

#include<cstdint>


namespace ymd{


enum class [[nodiscard]] JedecManufacturer:uint8_t{
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

enum class [[nodiscard]] JedecStorageType:uint8_t{
    Sram = 0x01,
    Nor = 0x02,
    Nand = 0x04,
    FastReadFlash = 0x08,
    SectorEraseFlash = 0x10,
    BlockEraseFlash = 0x20,
    PageProgramFlash = 0x40,
};

class OutputStream;

OutputStream & operator<<(OutputStream & os, const JedecManufacturer man);
OutputStream & operator<<(OutputStream & os, const JedecStorageType type);
}
