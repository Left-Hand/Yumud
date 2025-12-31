#pragma once

#include "core/platform.hpp"


namespace ymd::hal{
class Bkp;

struct BkpItem{
private:
    uint8_t rank_;
protected:
    BkpItem(uint8_t rank) : rank_(rank){;}
    friend class Bkp;

    void store(const uint16_t);
    [[nodiscard]] uint16_t load();
public:
    BkpItem(const BkpItem & other) = delete;
    BkpItem(BkpItem && other) = delete;
};


class Bkp {
public:
    Bkp(const Bkp &) = delete;
    Bkp& operator=(const Bkp &) = delete;

    static void init();

    static void store(const uint8_t rank,const uint16_t data);

    [[nodiscard]] static uint16_t load(const uint8_t rank);

    static BkpItem get(const uint8_t rank);
};

}