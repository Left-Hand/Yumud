#include "core/io/regs.hpp"
#include "core/utils/Option.hpp"
#include "core/utils/Errno.hpp"
#include "hal/bus/i2c/i2cdrv.hpp"

namespace ymd::hal::i2c_oper{

struct Start{};
struct Stop{};
struct SendDeviceAddr{};

template<typename Iterator>
struct Send{
    Iterator iter;
};

template<typename Iterator>
struct Receive{
    Iterator iter;
};
}

namespace ymd::hal::spi_oper{
struct Start{};
struct Stop{};
template<typename Iterator>
struct Send{
    Iterator iter;
};

template<typename Iterator>
struct Receive{
    Iterator iter;
};


template<typename Iterator>
struct Transreceive{
    Iterator iter;
};

};