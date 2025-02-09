#include "cia402.hpp"

using namespace ymd::canopen;

std::optional<SubEntry> Cia301ObjectDict::find(const std::pair<const Index, const SubIndex> didx) {
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

