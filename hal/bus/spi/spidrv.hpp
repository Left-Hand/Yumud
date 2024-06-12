#ifndef __SPI_DRV_HPP__

#define __SPI_DRV_HPP__

#include "spi.hpp"
#include "../busdrv.hpp"
#include <type_traits>
#include <initializer_list>


class SpiDrv:public BusDrv{
public:
    SpiDrv(Spi & _bus, const uint8_t & _index, const uint32_t & _wait_time = 320):BusDrv(_bus, _index,_wait_time){;}

    // void write(const uint8_t & data, bool discontinuous = true){
    //     if(!bus.begin(index)){
    //         bus.write(data);
    //         if(discontinuous) bus.end();
    //     }
    // }



    // void write(const uint16_t & data, bool discontinuous = true){
    //     if(!bus.begin(index)){
    //         bus.configDataSize(16);
    //         bus.write(data);
    //         if(discontinuous) bus.end();
    //         bus.configDataSize(8);
    //     }
    // }

    // void write(const uint32_t & data, bool discontinuous = true){
    //     if(!bus.begin(index)){
    //         bus.write(data);
    //         if(discontinuous) bus.end();
    //     }
    // }

    // void write(std::initializer_list<const uint8_t> datas, bool discontinuous = true){
    //     if(!bus.begin(index)){
    //         for(auto data_item : datas) bus.write(data_item);
    //         if(discontinuous) bus.end();
    //     }
    // }

    // void write(const uint8_t & data, const size_t & len, bool discontinuous = true){
    //     if(!bus.begin(index)){
    //         for(size_t i = 0; i < len; i++) bus.write(data);
    //         if(discontinuous) bus.end();
    //     }
    // }

};

#endif