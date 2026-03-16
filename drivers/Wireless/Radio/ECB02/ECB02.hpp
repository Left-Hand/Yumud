#pragma once

#include "core/string/view/string_view.hpp"
#include "hal/conn/uart/hw_singleton.hpp"

namespace ymd::drivers{






class ECB02{
enum class TxPower:uint8_t{
    _n20dBm,
    _n15dBm,
    _n10dBm,
    _n6dBm,
    _n5dBm,
    _n2dBm,
    _0dBm,
    _3dBm,
    _4dBm,
    _8dBm,
};

};
}