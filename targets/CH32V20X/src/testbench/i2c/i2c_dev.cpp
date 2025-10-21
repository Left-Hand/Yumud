#include "core/io/regs.hpp"
#include "core/utils/Option.hpp"
#include "hal/bus/i2c/i2cdrv.hpp"
#include "core/utils/Errno.hpp"

namespace ymd::hal::i2c_oper{

struct Start{};
struct Stop{};
struct SendDeviceAddr{};

template<typename Iterator>
struct SendData{
    Iterator iter;
};

template<typename Iterator>
struct ReceiveData{
    Iterator iter;
};
}

namespace ymd::hal::spi_oper{
struct Start{};
struct Stop{};
template<typename Iterator>
struct SendData{
    Iterator iter;
};

template<typename Iterator>
struct ReceiveData{
    Iterator iter;
};


template<typename Iterator>
struct TransreceiveData{
    Iterator iter;
};

};