#include "modbus_msgs.hpp"

namespace ymd::modbus{



template<typename Backend>
struct [[nodiscard]] ReqApiFacade final{

    constexpr auto read_coils(const req_msgs::ReadCoils & msg)  const {
        return Backend::convert(*this, msg);
    }

    constexpr auto read_discrete_inputs(const req_msgs::ReadDiscreteInputs & msg)  const {
        return Backend::convert(*this, msg);
    }


};


}