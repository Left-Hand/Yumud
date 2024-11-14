/*
   AD9910.h - AD9910 DDS communication library
   Based on AD9914 by Ben Reschovsky, 2016.
   JQI - Strontium - UMD
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   You should have received a copy of the GNU General Public License
   aunsigned long with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

//// THIS NEEDS TO BE UPDATED!! USING OLD VERSION FOR TESTING

#pragma once

#include "../drivers/device_defs.h"

namespace ymd::drivers{

class AD9910{
protected:
    SpiDrv spi_drv;
    
    uint8_t cfr1[4]={0x00,0x40,0x00,0x00};
    uint8_t cfr2[4]={0x01,0x00,0x00,0x00};
    
    uint8_t profile11[8]={0x3f,0xff,0x00,0x00,0x25,0x09,0x7b,0x42};

    uint8_t drgparameter[20]={0x00};
    uint8_t ramprofile0[8] = {0x00};

    void writeReg(const uint8_t, const uint8_t *, const size_t);
    void writeData(const uint8_t);

    void writeProfile(void);
    void writeRamprofile();
    void writeDrg(void);


public:
    AD9910(SpiDrv & _spi_drv):spi_drv(_spi_drv){;}
    AD9910(SpiDrv && _spi_drv):spi_drv(_spi_drv){;}
    AD9910(Spi & _spi, const uint8_t index):spi_drv(SpiDrv(_spi, index)){;}
    void init(void);
    void freqConvert(uint32_t Freq);

    void setAmplitude(uint32_t);
    void freqSweep(uint32_t, uint32_t, uint32_t, uint32_t);
    void sendSample(const uint8_t *, const size_t);
};

}