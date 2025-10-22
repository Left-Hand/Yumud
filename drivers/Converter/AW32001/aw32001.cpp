#include "aw32001.hpp"

using namespace ymd;
using namespace ymd::drivers;

using Self = AW32001;
using Error = Self::Error;

template<typename T = void>
using IResult = Result<T, Error>;

