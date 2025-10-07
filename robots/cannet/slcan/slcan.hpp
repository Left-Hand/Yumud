#pragma once

// https://blog.csdn.net/weifengdq/article/details/128823317

#include "../asciican_utils.hpp"


namespace ymd::robots::slcan{


class Slcan final{
public:
    using Msg = asciican::AsciiCanPhy::Msg;
    using Error = asciican::AsciiCanPhy::Error;
    using Flags = asciican::AsciiCanPhy::Flags;

    using StdId = hal::CanStdId;
    using ExtId = hal::CanExtId;

    template<typename T = void>
    using IResult = Result<T, Error>;

    Slcan(asciican::AsciiCanPhy & phy): phy_(phy){;}

    //不以\r结尾
    IResult<> on_recv_str(const StringView str);
private:

    IResult<> response_version();
    IResult<> response_serial_idx();
    Flags get_flag() const;

    IResult<> response_flag();

    IResult<> on_recv_set_baud(const StringView str);
    IResult<> on_recv_send_std_msg(const StringView str, const bool is_rmt);

    IResult<> on_recv_send_ext_msg(const StringView str, const bool is_rmt);
    IResult<> on_recv_open(const StringView str);

    IResult<> on_recv_close(const StringView str);


private:
    asciican::AsciiCanPhy & phy_;
};

}