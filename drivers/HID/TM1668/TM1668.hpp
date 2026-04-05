//这个驱动还未完成
//这个驱动还未测试

//TM1668是天微半导体的一款LED矩阵驱动/按键矩阵扫描芯片


#pragma once

#include "tm1668_prelude.hpp"

namespace ymd::drivers{


class TM1668 final:public TM1668_Prelude{
public:
    using DisplayCommand = TM1668_Transport::DisplayCommand;
    using KeyCode = TM1668_Transport::KeyCode;

    static constexpr auto NAME = "TM1668";

    explicit TM1668(TM1668_Transport && phy):
        transport_(std::move(phy)){;}


private:
    TM1668_Transport transport_;
};

}

