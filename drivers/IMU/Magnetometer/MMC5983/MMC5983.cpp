#include "MMC5983.hpp"
#include "core/debug/debug.hpp"

using namespace ymd;
using namespace ymd::drivers;


using Error = ImuError;
template<typename T = void>
using IResult = Result<T, Error>;

IResult<Vector3_t<q24>> read_mag(){
    return Ok(Vector3_t<q24>{0,0,0});
}