// real_t MT6816readLapPostion()
// {

// }

#ifndef __MT6816_HPP__
#define __MT6816_HPP__

#include "bus/spi/spidrv.hpp"
#include "src/device/Encoder/MagEncoder.hpp"

class MT6816:public MagEncoder{
protected:
    SpiDrv & bus_drv;
    uint16_t last_data;
    enum class RegAddress:uint8_t{

    };
    // void writeReg()
public:
    MT6816(SpiDrv & _bus_drv):bus_drv(_bus_drv){;}

    real_t getPosition() override{
        real_t position;
        u16_to_uni(getPositionData(), position);
        return position;
    }

    uint16_t getPositionData(){
        uint16_t dataTx[2];
        uint16_t dataRx[2];

        dataTx[0] = (0x80 | 0x03) << 8;
        dataTx[1] = (0x80 | 0x04) << 8;

        bus_drv.transmit(dataRx[0], dataTx[0]);
        bus_drv.transmit(dataRx[1], dataTx[1]);

        uint16_t new_data = ((dataRx[0] & 0x00FF) << 8) | (dataRx[1] & 0x00FC);
        // if((new_data > 300)){
            last_data = new_data;
        // }
        return last_data;
    }

};

#endif