#pragma once

#include "sys/core/platform.h"


namespace ymd::hal{
class Bkp;

struct BkpItem{
private:
    uint8_t index;
protected:
    BkpItem(uint8_t _index) : index(_index){;}
    friend class Bkp;

    void store(const uint16_t);
    uint16_t load();
public:


    BkpItem(const BkpItem & other) = delete;
    BkpItem(BkpItem && other) = delete;

    template<typename T>
    requires (sizeof(T) <= 2)

    BkpItem & operator = (const T data){
        store(*((uint16_t *)&(data)));
        return *this;
    }

    BkpItem & operator = (const uint16_t data){
        store(data);
        return *this;
    }

    template<typename T>
    operator T(){
        return static_cast<T>(load());
    }

    operator uint16_t(){
        return static_cast<uint16_t>(load());
    }
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
    Bkp(const Bkp &) = delete;
    Bkp& operator=(const Bkp &) = delete;

    static Bkp& singleton() {
        static Bkp instance;
        return instance;
    }
    void init();

    static void writeData(const uint8_t index,const uint16_t data);

    static uint16_t readData(const uint8_t index);

    BkpItem & operator [] (const uint8_t index);
};


extern Bkp & bkp;

}