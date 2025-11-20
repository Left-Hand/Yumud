#pragma once


#include "canopen_primitive_base.hpp"
#include "core/tmp/bits/width.hpp"
#include "core/tmp/implfor.hpp"
#include "core/utils/Option.hpp"
#include "core/utils/Result.hpp"


namespace ymd{
template<typename T>
static constexpr bool is_not_implemented_v = requires { 
    T::NOT_IMPLEMENTED;
};

template<typename T>
static constexpr bool is_implemented_v = not is_not_implemented_v<T>;
}
namespace ymd::convert{
template<typename To>
struct TryInto{
    static constexpr bool NOT_IMPLEMENTED = true;
};

template<typename From>
struct TryFrom{
    static constexpr bool NOT_IMPLEMENTED = true;
};



template<typename To, typename From>
static constexpr auto try_cast(const From& from){
    if constexpr(is_implemented_v<ImplFor<TryFrom<To>, From>>)
        return ImplFor<TryFrom<From>, To>::try_from(from);
    else if constexpr(is_implemented_v<ImplFor<TryInto<To>, From>>)
        return ImplFor<TryInto<To>, From>::try_into(from);
    else
        static_assert(tmp::false_v<To>, "can convert");
}

}




namespace ymd::canopen::primitive{

enum class NodeState:uint8_t{
    BootUp = 0x00,
    Stopped = 0x04,
    PreOperational = 0x05,
    Operating = 0x07,
};


enum class NmtCommand:uint8_t{
    StartRemoteNode = 0x01,
    StopRemoteNode = 0x02,
    EnterPreOperational = 0x80,
    ResetNode = 0x81,
    ResetCommunication = 0x82
};


}

namespace ymd{

template<>
struct ImplFor<convert::TryFrom<uint8_t>, canopen::primitive::NodeState>{
    using Self = canopen::primitive::NodeState;
    using Error = void;
    static constexpr Result<Self, Error> try_from(uint8_t int_val){
        switch(int_val){
            case static_cast<uint8_t>(Self::BootUp): 
            case static_cast<uint8_t>(Self::Stopped): 
            case static_cast<uint8_t>(Self::PreOperational): 
            case static_cast<uint8_t>(Self::Operating): 
                return Ok(static_cast<Self>(int_val));
        }
        return Err();
    }
};

template<>
struct ImplFor<convert::TryFrom<uint8_t>, canopen::primitive::NmtCommand>{
    using Self = canopen::primitive::NmtCommand;
    using Error = void;
    static constexpr Result<Self, Error> try_from(uint8_t int_val){
        switch(static_cast<Self>(int_val)){
            case Self::StartRemoteNode:   
            case Self::StopRemoteNode:    
            case Self::EnterPreOperational:   
            case Self::ResetNode: 
            case Self::ResetCommunication:    
                return Ok(static_cast<Self>(int_val));
        }
        return Err();
    }
};

static constexpr bool c = is_implemented_v<ImplFor<convert::TryFrom<uint8_t>, canopen::primitive::NmtCommand>>;

}