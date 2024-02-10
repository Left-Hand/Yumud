#ifndef __BKP_HPP__

#define __BKP_HPP__

#include "../defines/comm_inc.h"

struct BkpItem;

class Bkp {
private:
    static Bkp* instance;

    Bkp() {
        init();
    }

    ~Bkp() {
        if(instance != nullptr){
            delete instance;
            instance = nullptr;
        }
    }
    Bkp(const Bkp& other) = delete;
    Bkp& operator=(const Bkp& other) = delete;

public:
    static Bkp & getInstance() {
        if (!instance) {
            instance = new Bkp();
        }
        return *instance;
    }
    void init();

    static void writeData(uint8_t index, uint16_t data);

    static uint16_t readData(uint8_t index);

    BkpItem operator [] (uint8_t index);
};

struct BkpItem{
private:
    uint8_t index;
public:
    BkpItem(uint8_t _index) : index(_index){;}

    uint16_t operator = (const uint16_t & data){
        Bkp::writeData(index, data);
        return data;
    }
};

#endif