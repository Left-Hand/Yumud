#include "INA228_collections.hpp"

namespace ymd::drivers{
class INA228_Phy final:public INA228_Collections{ 
private:
    hal::I2cDrv i2c_drv;
};
}