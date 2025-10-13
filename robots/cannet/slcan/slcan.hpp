#pragma once

// https://blog.csdn.net/weifengdq/article/details/128823317

#include "../asciican_utils.hpp"
#include "core/utils/Match.hpp"

namespace ymd::robots::slcan{


namespace operations{
    struct SendCanMsg{
        hal::CanMsg msg;

        friend OutputStream & operator<<(OutputStream & os, const SendCanMsg & self){ 
            return os << os.scoped("SendCanMsg")(os 
                << os.field("msg")(os << self.msg)
            );
        }
    };

    struct SendText{
        static constexpr size_t MAX_TEXT_LEN = 16;

        char str[MAX_TEXT_LEN];

        static constexpr SendText from_str(const StringView strv){
            SendText ret;
            if(strv.size() > MAX_TEXT_LEN) 
                __builtin_trap();

            for(size_t i = 0; i < strv.size(); i++){
                ret.str[i] = strv[i];
            }
            return ret;
        }

        friend OutputStream & operator<<(OutputStream & os, const SendText & self){ 
            return os << os.scoped("SendText")(os 
                << os.field("str")(os << StringView(self.str))
            );
        }
    };

    struct SetSerialBaud{
        uint32_t baudrate;

        friend OutputStream & operator<<(OutputStream & os, const SetSerialBaud & self){ 
            return os << os.scoped("SetSerialBaud")(os 
                << os.field("baudrate")(os << self.baudrate)
            );
        }
    };

    struct SetCanBaud{
        hal::CanBaudrate baudrate;

        friend OutputStream & operator<<(OutputStream & os, const SetCanBaud & self){ 
            return os << os.scoped("SetCanBaud")(os 
                << os.field("baudrate")(os << self.baudrate)
            );
        }
    };

    struct Open{
        friend OutputStream & operator<<(OutputStream & os, const Open & self){ 
            return os << os.scoped("Open")(os);
        }
    };

    struct Close{
        friend OutputStream & operator<<(OutputStream & os, const Close & self){ 
            return os << os.scoped("Close")(os);
        }
    };

    struct SetTimestamp{
        Enable enabled;
        friend OutputStream & operator<<(OutputStream & os, const SetTimestamp & self){ 
            return os << os.scoped("SetTimestamp")(os 
                << os.field("enabled")(os << self.enabled)
            );
        }
    };
}


struct Operation:public Sumtype<
    operations::SendCanMsg, 
    operations::SendText,
    operations::SetSerialBaud,
    operations::SetCanBaud,
    operations::Open,
    operations::Close
>
{    

};

class Slcan final{
public:
    using Msg = asciican::AsciiCanPhy::Msg;
    using Error = asciican::AsciiCanPhy::Error;
    using Flags = asciican::AsciiCanPhy::Flags;

    using StdId = hal::CanStdId;
    using ExtId = hal::CanExtId;

    template<typename T = void>
    using IResult = Result<T, Error>;


    [[nodiscard]] IResult<Operation> handle_line(const StringView str) const;
private:

    [[nodiscard]] operations::SendText response_version() const ;
    [[nodiscard]] operations::SendText response_serial_idx() const ;
    [[nodiscard]] Flags get_flag() const;
    [[nodiscard]] operations::SendText response_flag() const ;
};

}