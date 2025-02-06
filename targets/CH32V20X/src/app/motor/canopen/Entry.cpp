#include "Entry.hpp"

using namespace ymd::canopen;

// size_t EntryDataType::dsize() const{

//     using enum Enum;
//     switch(e_){
//         default: return 0;
//         case bit:return 1;
//         case int8: return 1;
//         case int16: return 2;
//         case int32: return 4;
//         case uint8: return 1;
//         case uint16: return 2;
//         case uint32: return 4;
//         case real32: return 4;
//     }
// }


bool SubEntry::set(int val) {
    if (access_type_ == AccessType::CONST || access_type_  == AccessType::RO) {
        return false;
    }

    // if (data_type_ == DataType::uint8) {
    //     byte1 = static_cast<uint8_t>(val);
    // } else if (data_type_ == DataType::uint32 || data_type_ == DataType::int32) {
        pObject = val;
    // } else if (data_type_ == DataType::uint16) {
    //     pObject = val & 0x0000FFFF;
    // } else {
    //     return false;
    // }
    return true;
}

bool SubEntry::put(const std::span<const uint8_t> val) {
    if (access_type_ == AccessType::CONST || access_type_  == AccessType::RO) {
        return false;
    }

    if (data_type_ == DataType::uint8) {
        pObject = val[0];
    } else if (data_type_ == DataType::uint32 || data_type_ == DataType::int32) {
        int value = (val[0] & 0xFF) | ((val[1] & 0xFF) << 8) | ((val[2] & 0xFF) << 16) | ((val[3] & 0xFF) << 24);
        pObject = value;
    } else if (data_type_ == DataType::uint16) {
        int value = (val[0] & 0xFF) | ((val[1] & 0xFF) << 8);
        pObject = value & 0x0000FFFF;
    } else {
        return false;
    }
    notifyListeners();
    return true;
}


SubEntry::operator int() const {
    if (data_type_ == DataType::uint8) {
        return std::bit_cast<int>(pObject);
    } else if (data_type_ == DataType::uint32 || data_type_ == DataType::int32) {
        return std::bit_cast<int>(pObject);
    } else if (data_type_ == DataType::uint16) {
        return std::bit_cast<int>(pObject) & 0x0000FFFF;
    }
    return 0;
}
