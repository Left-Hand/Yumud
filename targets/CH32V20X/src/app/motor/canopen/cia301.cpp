#include "cia402.hpp"

using namespace ymd::canopen;

std::optional<SubEntry> Cia301ObjectDict::find(const Didx didx) {
    const auto [idx, subidx] = didx;

    switch (idx) {
            
        case 0x1000: switch (subidx) {
            case 0x0:
                return make_ro_subentry(control_word_reg);
        }break;

        case 0x1001: switch (subidx) {
            case 0x0:
                return make_ro_subentry(error_reg);
        }break;

        default: break;
    }
    return std::nullopt;
}

SdoAbortCode Cia301ObjectDict::write(const std::span<const uint8_t> pdata, const Didx didx){
    const auto [idx, subidx] = didx;
    switch (idx) {
        case 0x1003:
            return perdef_err_field_reg.write(pdata, subidx);
        default:
            return StaticObjectDictBase::write(pdata, didx);
    }

    return SdoAbortCode::None;
}
    
SdoAbortCode Cia301ObjectDict::read(const std::span<uint8_t> pdata, const Didx didx) const {
    const auto [idx, subidx] = didx;
    switch (idx) {
        case 0x1003:
            return perdef_err_field_reg.read(pdata, subidx);
        default:
            return StaticObjectDictBase::read(pdata, didx);
    }

    return SdoAbortCode::None;
}