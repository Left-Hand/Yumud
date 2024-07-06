// real_t MT6816readLapPostion()
// {

// }

#ifndef __MT6816_HPP__
#define __MT6816_HPP__

#include "bus/spi/spidrv.hpp"
#include "drivers/Encoder/MagEncoder.hpp"

class MT6816:public MagEncoder{
protected:
    SpiDrv bus_drv;
    real_t last_position;
    uint32_t errcnt = 0;

    struct Semantic{
        union{
            struct{
                uint16_t pc:1;
                uint16_t no_mag:1;
                uint16_t data_14bit:14;
            };
            uint16_t m_raw;
        };
        Semantic(const uint16_t raw):m_raw(raw){;}
    };

    Semantic last_semantic{0};
public:
    MT6816(SpiDrv & _bus_drv):bus_drv(_bus_drv){;}

    void init() override{
        last_position = real_t(-1); // not possible before init done;
        while(getLapPosition() < 0); // while reading before get correct position
    }

    real_t getLapPosition() override{

        uint16_t raw = getPositionData();
        last_semantic = Semantic(raw);

        uint8_t count = 0;

        raw -= last_semantic.pc;
        while(raw){//Brian Kernighan algorithm
            raw &= raw - 1;
            ++count;
        }

        if(count % 2 == last_semantic.pc){
            u16_to_uni(last_semantic.data_14bit << 2, last_position);
        }else{
            errcnt++;
        }

        return last_position;
    }

    uint16_t getPositionData(){
        uint16_t dataTx[2];
        uint16_t dataRx[2] = {0, 0};

        dataTx[0] = (0x80 | 0x03) << 8;
        dataTx[1] = (0x80 | 0x04) << 8;

        bus_drv.transfer(dataRx[0], dataTx[0]);
        bus_drv.transfer(dataRx[1], dataTx[1]);

        return((dataRx[0] & 0x00FF) << 8) | (dataRx[1]);
    }

    uint32_t getErrCnt() const {
        return errcnt;
    }

    bool stable() const override {
        return not last_semantic.no_mag;
    }
};

#endif