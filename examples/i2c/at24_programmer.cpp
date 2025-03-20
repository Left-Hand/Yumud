#include "core/debug/debug.hpp"

#include "hal/flash/flash.hpp"
#include "hal/bkp/bkp.hpp"
#include "hal/bus/i2c/i2csw.hpp"
#include "hal/bus/i2c/i2cdrv.hpp"
#include "hal/bus/uart/uarthw.hpp"

#include "drivers/Memory/EEprom/AT24CXX/at24cxx.hpp"
#include "drivers/Memory/Flash/W25QXX/w25q16.hpp"

#include "algo/random/random_generator.hpp"

using namespace ymd;
using namespace ymd::hal;
using namespace ymd::drivers;


static constexpr auto eeprom_data = std::to_array<uint8_t>({
    0xA5, 0x5A, 0x06, 0xBC, 0x05, 0x10, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0x20, 0x07, 0x29, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0x90, 0xDE, 0x0B, 0xEF, 0xF0, 0x12, 0xD0, 0x3A, 0x90, 0xDE, 0x0B, 0xE0,
    0xFF, 0x02, 0xD1, 0x06, 0x90, 0xDE, 0x05, 0xEB, 0xF0, 0xA3, 0xEA, 0xF0,
    0xA3, 0xE9, 0xF0, 0x12, 0x63, 0x45, 0x22, 0xFF, 0xE5, 0x33, 0x30, 0xE2,
    0x09, 0x53, 0x33, 0xFB, 0x12, 0xD2, 0xB6, 0x12, 0xCF, 0x5E, 0xE5, 0x32,
    0x20, 0xE3, 0x03, 0x02, 0xCD, 0x51, 0xE5, 0x36, 0xC3, 0x94, 0x06, 0x40,
    0x03, 0x02, 0xCD, 0x51, 0xE5, 0x37, 0x64, 0x3C, 0x60, 0x03, 0x02, 0xCD,
    0x51, 0x53, 0x32, 0xF7, 0xE5, 0x30, 0x20, 0xE1, 0x04, 0x7F, 0x01, 0x80,
    0x02, 0x7F, 0x00, 0xE5, 0x38, 0xB4, 0x01, 0x04, 0x7E, 0x01, 0x80, 0x02,
    0x7E, 0x00, 0xEE, 0x5F, 0x60, 0x08, 0x90, 0xDE, 0x0C, 0x74, 0x01, 0xF0,
    0x80, 0x2E, 0xE5, 0x38, 0x70, 0x14, 0x90, 0xDE, 0x0C, 0xE0, 0xB4, 0x01,
    0x0D, 0xE5, 0x30, 0x30, 0xE1, 0x08, 0x75, 0x38, 0x02, 0x74, 0x02, 0xF0,
    0x80, 0x16, 0xE5, 0x38, 0xB4, 0x02, 0x11, 0x90, 0xDE, 0x0C, 0xE0, 0xB4,
    0x02, 0x0A, 0xE5, 0x30, 0x20, 0xE1, 0x05, 0xE4, 0xF0, 0x75, 0x38, 0x03,
    0x75, 0x93, 0x03, 0xD2, 0x80, 0xE5, 0x30, 0x30, 0xE1, 0x41, 0x53, 0x30,
    0xFD, 0xE5, 0x30, 0x20, 0xE6, 0x06, 0x90, 0xF1, 0xFC, 0xE0, 0x60, 0x1A,
    0x53, 0x30, 0xBF, 0x90, 0xF0, 0x00, 0xE0, 0x44, 0x02, 0xF0, 0x90, 0xF1,
    0xFF, 0xE4, 0xF0, 0x90, 0xF1, 0xFC, 0x04, 0xF0, 0x12, 0xD2, 0xB2, 0x75,
    0x38, 0x01, 0x85, 0xBC, 0x1B, 0x85, 0xBD, 0x1C, 0x85, 0xBE, 0x1D, 0x85,
    0xBF, 0x1E, 0xE5, 0x30, 0x30, 0xE0, 0x05, 0x53, 0x30, 0xFE, 0x80, 0x03,
    0x43, 0x30, 0x01, 0xE5, 0x38, 0xB4, 0x01, 0x0C, 0x90, 0xF1, 0xFB, 0xE0,
    0xB4, 0x03, 0x0D, 0xE4, 0xF5, 0x38, 0x80, 0x05, 0xE5, 0x38, 0xB4, 0x03,
    0x03, 0x43, 0x30, 0x02, 0x90, 0xF1, 0xFA, 0xE0, 0x60, 0x06, 0x74, 0x01,
    0xF0, 0x43, 0x30, 0x40, 0x75, 0xAE, 0x0C, 0x85, 0x30, 0xAE, 0x85, 0x1B,
    0xAE, 0x85, 0x1C, 0xAE, 0x85, 0x1D, 0xAE, 0x85, 0x1E, 0xAE, 0x85, 0xBC,
    0xAE, 0x85, 0xBD, 0xAE, 0x85, 0xBE, 0xAE, 0x85, 0xBF, 0xAE, 0x85, 0x9E,
    0xAE, 0x85, 0x9F, 0xAE, 0xE5, 0x38, 0xB4, 0x02, 0x0E, 0x90, 0xDE, 0x0C,
    0xE0, 0xB4, 0x02, 0x07, 0x75, 0xAE, 0xFF, 0x75, 0xAE, 0xD8, 0x22, 0xE5,
    0x38, 0xB4, 0x03, 0x09, 0xE4, 0xF5, 0x38, 0x75, 0xAE, 0xFF, 0x75, 0xAE,
    0xD9, 0x22, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x21, 0x57,
    0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x1D, 0x01, 0x04, 0x82, 0x46,
    0x27, 0x78, 0x2A, 0xD9, 0xB0, 0xA3, 0x57, 0x49, 0x9C, 0x25, 0x11, 0x49,
    0x4B, 0x21, 0x08, 0x00, 0x71, 0x40, 0x81, 0xC0, 0x81, 0x00, 0x81, 0x40,
    0x81, 0x80, 0x95, 0x00, 0xA9, 0xC0, 0xB3, 0x00, 0x02, 0x3A, 0x80, 0x18,
    0x71, 0x38, 0x2D, 0x40, 0x58, 0x2C, 0x45, 0x00, 0xC4, 0x8E, 0x21, 0x00,
    0x00, 0x1E, 0x00, 0x00, 0x00, 0xFD, 0x00, 0x17, 0x55, 0x1E, 0x64, 0x1E,
    0x04, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0xF7,
    0x00, 0x0A, 0x00, 0x40, 0xC6, 0x44, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0xFC, 0x00, 0x48, 0x44, 0x4D, 0x49, 0x20,
    0x54, 0x4F, 0x20, 0x55, 0x53, 0x42, 0x0A, 0x20, 0x01, 0xD4, 0x02, 0x03,
    0x2C, 0xF1, 0x51, 0x01, 0x02, 0x03, 0x04, 0x11, 0x12, 0x13, 0x1F, 0x20,
    0x21, 0x22, 0x3C, 0x3D, 0x3E, 0x90, 0x5F, 0x64, 0x23, 0x09, 0x07, 0x07,
    0x83, 0x01, 0x00, 0x00, 0x67, 0x03, 0x0C, 0x00, 0x10, 0x00, 0xB8, 0x3C,
    0xE5, 0x0E, 0x61, 0x60, 0x66, 0x65, 0x01, 0x1D, 0x00, 0x72, 0x51, 0xD0,
    0x1E, 0x20, 0x6E, 0x28, 0x55, 0x00, 0xC4, 0x8E, 0x21, 0x00, 0x00, 0x1E,
    0x8C, 0x0A, 0xD0, 0x8A, 0x20, 0xE0, 0x2D, 0x10, 0x10, 0x3E, 0x96, 0x00,
    0xC4, 0x8E, 0x21, 0x00, 0x00, 0x18, 0x8C, 0x0A, 0xD0, 0x90, 0x20, 0x40,
    0x31, 0x20, 0x0C, 0x40, 0x55, 0x00, 0xC4, 0x8E, 0x21, 0x00, 0x00, 0x18,
    0x4E, 0x1F, 0x00, 0x80, 0x51, 0x00, 0x1E, 0x30, 0x40, 0x80, 0x37, 0x00,
    0xC4, 0x8E, 0x21, 0x00, 0x00, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x22, 0x0A, 0x8B, 0x02, 0x00, 0x05, 0x0A,
    0x8B, 0x02, 0x00, 0x15, 0x16, 0x05, 0x00, 0x80, 0x1A, 0x06, 0x00, 0x20,
    0xA1, 0x07, 0x00, 0x40, 0x42, 0x0F, 0x00, 0xA9, 0x07, 0x90, 0xC6, 0xC4,
    0xE0, 0xFE, 0xA3, 0xE0, 0xFF, 0x90, 0xC6, 0xAF, 0xE0, 0xFC, 0xA3, 0xE0,
    0xFD, 0xC3, 0x9F, 0xEC, 0x9E, 0x50, 0x06, 0xAE, 0x04, 0xAF, 0x05, 0x80,
    0x00, 0xAB, 0x07, 0xAA, 0x06, 0xD3, 0xEB, 0x94, 0xA0, 0xEA, 0x94, 0x05,
    0x40, 0x07, 0xE9, 0x60, 0x04, 0x7A, 0x05, 0x7B, 0xA0, 0x90, 0xF8, 0xE6,
    0xEB, 0xF0, 0xEA, 0xF9, 0xA3, 0xF0, 0xD3, 0xED, 0x9B, 0xEC, 0x9A, 0x50,
    0x08, 0x90, 0xF8, 0xE1, 0x74, 0x47, 0xF0, 0x80, 0x1F, 0xEB, 0x25, 0xE0,
    0xFF, 0xEA, 0x33, 0xFE, 0xD3, 0x90, 0xC6, 0xB0, 0xE0, 0x9F, 0x90, 0xC6,
    0xAF, 0xE0, 0x9E, 0x90, 0xF8, 0xE1, 0x50, 0x05, 0x74, 0x4B, 0xF0, 0x80,
    0x03, 0x74, 0x53, 0xF0, 0x90, 0xC6, 0xAA, 0xE0, 0x90, 0xF9, 0x26, 0x30,
    0xE0, 0x11, 0x74, 0x0F, 0xF0, 0x90, 0xF9, 0x29, 0x74, 0x1F, 0xF0, 0x90,
    0xF9, 0x36, 0x74, 0x0F, 0xF0, 0x80, 0x0F, 0x74, 0x05, 0xF0, 0x90, 0xF9,
    0x29, 0x74, 0x0B, 0xF0, 0x90, 0xF9, 0x36, 0x74, 0x05, 0xF0, 0x90, 0xF9,
    0x25, 0x74, 0xEF, 0xF0, 0x90, 0xF9, 0x28, 0x74, 0xDE, 0xF0, 0x90, 0xF9,
    0x35, 0x74, 0xEF, 0xF0, 0x90, 0xF9, 0x47, 0x74, 0x0C, 0xF0, 0x90, 0xF9,
    0x4A, 0x74, 0x12, 0xF0, 0xEA, 0xC3, 0x13, 0xEB, 0x13, 0xFD, 0x90, 0xF9,
    0x37, 0xF0, 0xEA, 0xC3, 0x13, 0xFF, 0xA3, 0xF0, 0xA3, 0xED, 0xF0, 0xA3,
    0xEF, 0xF0, 0x90, 0xFD, 0x40, 0xEB, 0xF0, 0xA3, 0xE9, 0xF0, 0xAF, 0x03,
    0xAE, 0x02, 0x90, 0xC6, 0xC4, 0xE0, 0xFC, 0xA3, 0xE0, 0xFD, 0xE4, 0xFB,
    0x12, 0xD2, 0x72, 0x90, 0xFD, 0x3B, 0xEF, 0xF0, 0xEE, 0xA3, 0xF0, 0x90,
    0xF8, 0xE0, 0x74, 0x01, 0xF0, 0x90, 0xFD, 0x00, 0xF0, 0x22, 0xE4, 0xFF,
    0xFE, 0xE4, 0x90, 0xDE, 0x08, 0xF0, 0xA3, 0xF0, 0x90, 0xDE, 0x08, 0xE0,
    0xFC, 0xA3, 0xE0, 0xFD, 0xC3, 0x94, 0x19, 0xEC, 0x64, 0x80, 0x94, 0x80,
    0x50, 0x46, 0x74, 0x52, 0x2D, 0xF5, 0x82, 0x74, 0xCE, 0x3C, 0xF5, 0x83,
    0xE4, 0x93, 0xFD, 0x90, 0xDE, 0x08, 0xE0, 0xFA, 0xA3, 0xE0, 0xFB, 0x8F,
    0x82, 0x8E, 0x83, 0x74, 0x2E, 0x12, 0xD2, 0x09, 0xE5, 0x82, 0x2B, 0xF5,
    0x82, 0xE5, 0x83, 0x3A, 0xF5, 0x83, 0xE5, 0x82, 0x24, 0xBF, 0xF5, 0x82,
    0xE5, 0x83, 0x34, 0xC0, 0xF5, 0x83, 0xED, 0xF0, 0x90, 0xDE, 0x09, 0xE0,
    0x04, 0xF0, 0x70, 0xB0, 0x90, 0xDE, 0x08, 0xE0, 0x04, 0xF0, 0x80, 0xA8,
    0x0F, 0xBF, 0x00, 0x01, 0x0E, 0xEF, 0x64, 0x05, 0x4E, 0x70, 0x96, 0x7E,
    0x00, 0x7F, 0x05, 0xE4, 0x90, 0xDE, 0x08, 0xF0, 0xA3, 0xF0, 0x90, 0xDE,
    0x08, 0xE0, 0xFC, 0xA3, 0xE0, 0xFD, 0xC3, 0x94, 0x04, 0xEC, 0x64, 0x80,
    0x94, 0x80, 0x50, 0x46, 0x74, 0x52, 0x2D, 0xF5, 0x82, 0x74, 0xCE, 0x3C,
    0xF5, 0x83, 0xE4, 0x93, 0xFD, 0x90, 0xDE, 0x08, 0xE0, 0xFA, 0xA3, 0xE0,
    0xFB, 0x8F, 0x82, 0x8E, 0x83, 0x74, 0x2E, 0x12, 0xD2, 0x09, 0xE5, 0x82,
    0x2B, 0xF5, 0x82, 0xE5, 0x83, 0x3A, 0xF5, 0x83, 0xE5, 0x82, 0x24, 0xBF,
    0xF5, 0x82, 0xE5, 0x83, 0x34, 0xC0, 0xF5, 0x83, 0xED, 0xF0, 0x90, 0xDE,
    0x09, 0xE0, 0x04, 0xF0, 0x70, 0xB0, 0x90, 0xDE, 0x08, 0xE0, 0x04, 0xF0,
    0x80, 0xA8, 0x0F, 0xBF, 0x00, 0x01, 0x0E, 0xEF, 0x64, 0x0B, 0x4E, 0x70,
    0x96, 0x22, 0xEF, 0x12, 0xD2, 0x1A, 0xD0, 0x5D, 0x00, 0xD0, 0x84, 0x01,
    0xD0, 0xB0, 0x02, 0xD0, 0xE3, 0x0A, 0xD0, 0x84, 0x0B, 0xD0, 0xD0, 0x0C,
    0xD0, 0xE0, 0x0E, 0xD0, 0xEA, 0x0F, 0xD0, 0xFD, 0x14, 0x00, 0x00, 0xD1,
    0x05, 0x90, 0xC7, 0x81, 0x74, 0x04, 0xF0, 0x90, 0xC7, 0x83, 0x74, 0x20,
    0xF0, 0x90, 0xC7, 0x82, 0x74, 0x1C, 0xF0, 0x90, 0xF8, 0x0B, 0x74, 0xC1,
    0xF0, 0x90, 0xF8, 0x14, 0x74, 0x23, 0xF0, 0x90, 0xC6, 0x97, 0x74, 0x02,
    0xF0, 0xC2, 0x0C, 0x22, 0x90, 0xFE, 0x20, 0x74, 0x2A, 0xF0, 0x12, 0xD2,
    0x87, 0x90, 0xF9, 0x2C, 0x74, 0x20, 0xF0, 0x90, 0xE3, 0x3C, 0x74, 0xF5,
    0xF0, 0xA3, 0x74, 0x0F, 0xF0, 0xA3, 0xE4, 0xF0, 0xA3, 0xF0, 0x90, 0xF8,
    0x05, 0xE0, 0x54, 0xFC, 0xF0, 0x90, 0xFE, 0xBA, 0x74, 0x08, 0xF0, 0x22,
    0x12, 0xD2, 0x40, 0x90, 0xEF, 0xE0, 0xE0, 0x30, 0xE6, 0x4B, 0x90, 0xEF,
    0xE8, 0x74, 0x40, 0xF0, 0xA3, 0xE4, 0xF0, 0xA3, 0xF0, 0xA3, 0xF0, 0x90,
    0xC7, 0x76, 0xF0, 0xA3, 0x74, 0xC8, 0xF0, 0x22, 0x30, 0x00, 0x03, 0x12,
    0xD1, 0xC2, 0x90, 0xF9, 0xAF, 0x74, 0x22, 0xF0, 0x74, 0x02, 0xF0, 0x22,
    0x02, 0xD2, 0x87, 0x90, 0xF9, 0xAF, 0x74, 0x22, 0xF0, 0x22, 0xE4, 0x90,
    0xC5, 0x4F, 0xF0, 0x90, 0xEF, 0xD0, 0xF0, 0xA3, 0x74, 0x30, 0xF0, 0xA3,
    0xE4, 0xF0, 0xA3, 0xF0, 0x22, 0x12, 0xD1, 0xC2, 0x7F, 0xFF, 0x12, 0xD2,
    0xA1, 0x22, 0xEF, 0x24, 0xF8, 0x60, 0x15, 0x24, 0xFC, 0x60, 0x1A, 0x24,
    0xF8, 0x60, 0x16, 0x24, 0x14, 0x60, 0x03, 0x02, 0xD1, 0xC1, 0xE4, 0x90,
    0xDE, 0x0C, 0xF0, 0x22, 0x7B, 0xFF, 0x7A, 0xCD, 0x79, 0x52, 0x02, 0xCC,
    0x10, 0xE5, 0x37, 0x64, 0x3C, 0x60, 0x03, 0x02, 0xD1, 0xC1, 0xE5, 0x36,
    0xB4, 0x01, 0x0C, 0x90, 0xFD, 0x03, 0x74, 0x09, 0xF0, 0x90, 0xFD, 0x02,
    0x74, 0x42, 0xF0, 0xE5, 0x36, 0xB4, 0x02, 0x0C, 0x90, 0xFD, 0x03, 0x74,
    0x08, 0xF0, 0x90, 0xFD, 0x02, 0x74, 0x55, 0xF0, 0xE5, 0x36, 0xB4, 0x03,
    0x12, 0x90, 0xFD, 0x03, 0x74, 0x07, 0xF0, 0x90, 0xFD, 0x02, 0x74, 0x25,
    0xF0, 0x90, 0xF8, 0x07, 0x74, 0x0A, 0xF0, 0xE5, 0x36, 0xB4, 0x04, 0x12,
    0x90, 0xFD, 0x03, 0x74, 0x06, 0xF0, 0x90, 0xFD, 0x02, 0x74, 0x83, 0xF0,
    0x90, 0xF8, 0x07, 0x74, 0x08, 0xF0, 0xE5, 0x36, 0xB4, 0x05, 0x12, 0x90,
    0xFD, 0x03, 0x74, 0x06, 0xF0, 0x90, 0xFD, 0x02, 0x74, 0x49, 0xF0, 0x90,
    0xF8, 0x07, 0x74, 0x09, 0xF0, 0xE5, 0x36, 0xC3, 0x94, 0x06, 0x50, 0x21,
    0x90, 0xC6, 0xB6, 0xE0, 0x94, 0xB8, 0x90, 0xC6, 0xB5, 0xE0, 0x94, 0x0B,
    0x40, 0x13, 0x90, 0xC6, 0xAA, 0xE0, 0x30, 0xE0, 0x0C, 0x90, 0xF9, 0x2E,
    0x74, 0x22, 0xF0, 0x90, 0xF9, 0x22, 0x74, 0x82, 0xF0, 0x22, 0x90, 0xF8,
    0x06, 0x74, 0x30, 0xF0, 0x90, 0xC6, 0xC6, 0xE0, 0xFE, 0xA3, 0xE0, 0xFF,
    0xC3, 0x90, 0xC6, 0xB2, 0xE0, 0x9F, 0x90, 0xC6, 0xB1, 0xE0, 0x9E, 0x40,
    0x26, 0x90, 0xC6, 0xAA, 0xE0, 0x30, 0xE0, 0x1F, 0xE5, 0x36, 0x94, 0x03,
    0x50, 0x15, 0xE5, 0x37, 0x94, 0x0A, 0x40, 0x0F, 0x90, 0xC6, 0xB5, 0xE0,
    0xD3, 0x94, 0x0B, 0x40, 0x06, 0x90, 0xF8, 0x06, 0x74, 0x20, 0xF0, 0xE4,
    0xFF, 0x80, 0x02, 0x7F, 0x01, 0x12, 0xCE, 0x6B, 0x22, 0xF5, 0xF0, 0xC5,
    0x82, 0xA4, 0xC5, 0x82, 0xC5, 0xF0, 0xC5, 0x83, 0xA4, 0x25, 0x83, 0xF5,
    0x83, 0x22, 0xD0, 0x83, 0xD0, 0x82, 0xF8, 0xE4, 0x93, 0x70, 0x12, 0x74,
    0x01, 0x93, 0x70, 0x0D, 0xA3, 0xA3, 0x93, 0xF8, 0x74, 0x01, 0x93, 0xF5,
    0x82, 0x88, 0x83, 0xE4, 0x73, 0x74, 0x02, 0x93, 0x68, 0x60, 0xEF, 0xA3,
    0xA3, 0xA3, 0x80, 0xDF, 0x90, 0xF1, 0xF0, 0xE0, 0x70, 0x2B, 0xE5, 0x3B,
    0x60, 0x27, 0xC2, 0xAF, 0x90, 0xC6, 0xA0, 0xE0, 0x90, 0xFE, 0x90, 0xF0,
    0x90, 0xC6, 0xA2, 0xE0, 0x90, 0xFE, 0x91, 0xF0, 0x90, 0xC6, 0xA4, 0xE0,
    0x90, 0xFE, 0x93, 0xF0, 0x90, 0xC6, 0xA6, 0xE0, 0x90, 0xFE, 0x92, 0xF0,
    0xE4, 0xF5, 0x3B, 0xD2, 0xAF, 0x22, 0x90, 0xDE, 0x00, 0xEE, 0xF0, 0xA3,
    0xEF, 0xF0, 0xA3, 0xEC, 0xF0, 0xA3, 0xED, 0xF0, 0xA3, 0xEB, 0xF0, 0x12,
    0x52, 0xDD, 0x22, 0x12, 0xD2, 0xAA, 0x90, 0xF8, 0x14, 0x74, 0x33, 0xF0,
    0x14, 0xF0, 0x02, 0xD2, 0xAE, 0x78, 0xFF, 0xE4, 0xF6, 0xD8, 0xFD, 0x75,
    0x81, 0x3C, 0x02, 0xD2, 0xBA, 0x90, 0xDE, 0x0A, 0xEF, 0xF0, 0x12, 0x69,
    0xFB, 0x22, 0x12, 0x60, 0x3C, 0x22, 0x12, 0x58, 0x84, 0x22, 0x12, 0x48,
    0xE6, 0x22, 0x12, 0x60, 0x69, 0x22, 0x80, 0xFE, 0x27, 0x02, 0x52, 0x8D,
});


void programmer_main(){
    DEBUGGER_INST.init(DEBUG_UART_BAUD, CommStrategy::Blocking);
    auto & logger = DEBUGGER;
    logger.setEps(2);
    logger.setRadix(10);
    logger.setSplitter("\t\t");

    
    I2cSw       i2c{portD[2], portC[12]};
    i2c.init(400_KHz);


    AT24C64 eeprom{i2c};

    eeprom.init();
    auto mem = Memory(eeprom);

    const auto & arr = eeprom_data;
    DEBUG_PRINTLN(std::hex);
    scexpr size_t size = arr.size();

    static_assert(size == sizeof(arr));
    std::array<uint8_t, size> buf;


    mem.store(0, arr.begin(), arr.begin() + size);

    mem.load(0,buf.begin(), buf.begin() + size);

    std::vector<size_t> err_index;

    for(size_t i = 0; i < size; ++i){
        if(arr[i] != buf[i]){
            err_index.emplace_back(i);
        }
    }

    if(err_index.size()){
        for(auto i : err_index){
            DEBUG_PRINTLN("error");
            DEBUG_PRINTLN(i, arr[i], buf[i]);
        }
    }else{
        DEBUG_PRINTLN("success");
    }
    DEBUG_PRINTLN("over");
    while(true){
    }
}
