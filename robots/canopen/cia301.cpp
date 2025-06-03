#include "cia402.hpp"

using namespace ymd::canopen;

std::optional<SubEntry> Cia301ObjectDict::find(const Didx didx) {
    const auto [idx, subidx] = didx;

    switch (idx) {
            
        case 0x1000: switch (subidx) {
            case 0x0:
                // return make_ro_subentry(control_word_reg);

                TODO();
        }break;

        case 0x1001: switch (subidx) {
            case 0x0:
                // return make_ro_subentry(error_reg);
                TODO();
        }break;

        default: break;
    }
    return std::nullopt;
}

SdoAbortCode Cia301ObjectDict::write(const std::span<const uint8_t> pbuf, const Didx didx){
    const auto [idx, subidx] = didx;
    switch (idx) {
        case 0x1003:
            return perdef_err_field_reg.write(pbuf, subidx);
        // default:
        //     return StaticObjectDictBase::write(pbuf, didx);
    }

    return SdoAbortCode::None;
}
    
SdoAbortCode Cia301ObjectDict::read(const std::span<uint8_t> pbuf, const Didx didx) const {
    const auto [idx, subidx] = didx;
    switch (idx) {
        case 0x1003:
            return perdef_err_field_reg.read(pbuf, subidx);
        // default:
        //     return StaticObjectDictBase::read(pbuf, didx);
    }

    return SdoAbortCode::None;
}