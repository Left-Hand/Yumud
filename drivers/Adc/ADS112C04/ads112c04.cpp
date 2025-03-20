#include "ads112c04.hpp"



#ifdef ADS112C04_DEBUG
#define ADS112C04_DEBUG(...) DEBUG_LOG(__VA_ARGS__)
#else
#define ADS112C04_DEBUG(...)
#endif


using namespace ymd::drivers;