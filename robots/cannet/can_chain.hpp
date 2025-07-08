#pragma once

#include "core/utils/Option.hpp"
#include "core/utils/handle_status.hpp"
#include "hal/bus/can/can_msg.hpp"

namespace ymd::robots{


class CanMsgHandlerIntf{ 
public:
    virtual HandleStatus handle(const hal::CanMsg & msg) = 0;
};


class CanMsgHandlerChainlink final: public CanMsgHandlerIntf{ 
public:
    CanMsgHandlerChainlink(
        Some<CanMsgHandlerIntf *> curr, 
        Option<CanMsgHandlerIntf &> next
    ):
        curr_handler_(*curr.get()),
        next_handler_(next){;}

    HandleStatus handle(const hal::CanMsg & msg){ 
        HandleStatus res = curr_handler_.handle(msg);
        if(next_handler_.is_none()) return res;
        return next_handler_.unwrap().handle(msg);
    }
private:
    CanMsgHandlerIntf & curr_handler_;
    Option<CanMsgHandlerIntf &> next_handler_ = None;
};



class CanHandlerTerminator final: 
public CanMsgHandlerIntf{ 
    HandleStatus handle(const hal::CanMsg & msg){ 
        return HandleStatus::from_handled();
    }
};

}