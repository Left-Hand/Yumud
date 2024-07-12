#ifndef __BKP_HPP__

#define __BKP_HPP__

#include "../sys/core/platform.h"

struct BkpItem;

class Bkp {
private:
    Bkp(){;}
public:
    Bkp(const Bkp&) = delete;
    Bkp& operator=(const Bkp&) = delete;

    static Bkp& getInstance() {
        static Bkp instance;
        return instance;
    }
    void init();

    static void writeData(uint8_t index, uint16_t data);

    static uint16_t readData(uint8_t index);

    BkpItem operator [] (uint8_t index);
};


extern Bkp & bkp;

struct BkpItem{
private:
    uint8_t index;
public:
    BkpItem(uint8_t _index) : index(_index){;}

    BkpItem & operator = (const uint16_t & data){
        bkp.writeData(index, data);
        return *this;
    }
};



#endif