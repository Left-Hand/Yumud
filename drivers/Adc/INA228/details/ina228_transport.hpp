#include "INA228_Prelude.hpp"

namespace ymd::drivers{
class INA228_Transport final:public INA228_Prelude{ 
private:
    hal::I2cDrv i2c_drv;
};
}