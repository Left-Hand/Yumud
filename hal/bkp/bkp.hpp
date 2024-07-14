#ifndef __BKP_HPP__

#define __BKP_HPP__

#include "../sys/core/platform.h"
// #include <array>

class Bkp;

struct BkpItem{
private:
    uint8_t index;
protected:
    BkpItem(uint8_t _index) : index(_index){;}
    friend class Bkp;
public:


    BkpItem(const BkpItem & other) = delete;
    BkpItem(BkpItem && other) = delete;

    BkpItem & operator = (const int data){
        this->operator=((uint16_t)data);
        return *this;
    }

    BkpItem & operator = (const uint16_t data);
    operator uint16_t();
};


class Bkp {
private:
    BkpItem items[10];
    Bkp():items{
        BkpItem(1),
        BkpItem(2),
        BkpItem(3),
        BkpItem(4),
        BkpItem(5),
        BkpItem(6),
        BkpItem(7),
        BkpItem(8),
        BkpItem(9),
        BkpItem(10),
    }{;}
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

    BkpItem & operator [] (uint8_t index);
};


extern Bkp & bkp;






#endif