#include "ads7830.hpp"

using namespace ymd;
using namespace ymd::drivers;

using Error = ADS7830::Error;

template<typename T = void>
using IResult = Result<T, Error>;

IResult<> ADS7830::init(){
    return Ok();
}

IResult<> ADS7830::validate(){
    return Ok();
}


IResult<ADS7830::ConvData> ADS7830::read_channel(const ChannelSelection sel){
    const auto cmd = CommandByte{
        .pd = PowerDownSel::RefOn_AdcOn,
        .sel = sel.kind()
    };

    return phy_.fs_read(cmd);
}

IResult<ADS7830::ConvData> ADS7830::read_pos_channel(const ChannelNth nth){
    const auto sel = ({
        const auto may_sel = ChannelSelection::from_pos(nth);
        if(may_sel.is_none()) return Err(Error::NoChannelCombination);
        may_sel.unwrap();
    });

    return read_channel(sel);
}