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

#include "core/io/regs.hpp"
#include "core/utils/Result.hpp"

#include "hal/bus/spi/spidrv.hpp"

namespace ymd::drivers{

class AD9910{
protected:
    hal::SpiDrv spi_drv;
    
    static constexpr uint8_t cfr1[4] = {0x00,0x40,0x00,0x00};
    static constexpr uint8_t cfr2[4] = {0x01,0x00,0x00,0x00};
    static constexpr uint8_t ramprofile0[8] = {0};

    struct Profile{
    protected:
        uint8_t data[8] = {0x3f,0xff,0x00,0x00,0x25,0x09,0x7b,0x42};
    public: 
        Profile() = default;
        uint8_t & operator [](const size_t idx){return data[idx];}
        std::span<const uint8_t> as_bytes() const {
            return std::span<const uint8_t>(data);
        }
    };

    struct DrgParamenter {
    protected:
        uint8_t data[20];
    public: 
        DrgParamenter () = default;
        uint8_t & operator [](const size_t idx){return data[idx];}
        std::span<const uint8_t> as_bytes() const {
            return std::span<const uint8_t>(data);
        }
    };



    hal::HalResult write_burst(const uint8_t, const std::span<const uint8_t> pbuf);
    hal::HalResult write_data(const uint8_t);

    void write_profile(const Profile & profile);
    void write_ramprofile();
    void write_drg(const DrgParamenter & drg);


public:
    AD9910(const hal::SpiDrv & _spi_drv):
        spi_drv(_spi_drv){;}
    AD9910(hal::SpiDrv && _spi_drv):
        spi_drv(std::move(_spi_drv)){;}
    AD9910(hal::Spi & _spi, const hal::SpiSlaveRank index):
        spi_drv(hal::SpiDrv(_spi, index)){;}

    void init(void);
    void freq_convert(uint32_t Freq);

    void set_amplitude(uint32_t);
    void freq_sweep(uint32_t, uint32_t, uint32_t, uint32_t);
    void send_sample(const std::span<const uint8_t> pbuf);
};

}