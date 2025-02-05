#pragma once

#include "hal/bus/can/can.hpp"
#include <variant>
#include <unordered_map>
#include "sys/string/String.hpp"

namespace ymd::canopen{

using Can = hal::Can;
using CanMsg = hal::CanMsg;

class CanDriver {
public:
    // using Can = hal::Can;
    // using CanMsg = hal::CanMsg;

    virtual ~CanDriver() = default;
    virtual bool write(const CanMsg & frame) = 0;
    virtual bool read(CanMsg & frame) = 0;
};




// class ObjectDictionary {
// public:
//     using ObjectDictValue = std::variant<uint8_t, uint16_t, uint32_t, int8_t, int16_t, int32_t, float>;

//     struct Index {
//         uint16_t index;
//         uint8_t subindex;

//         // Define equality operator for use in unordered_map
//         bool operator==(const Index& other) const {
//             return index == other.index && subindex == other.subindex;
//         }

//         struct Hasher {
//             size_t operator()(const Index& idx) const {
//                 return std::hash<uint16_t>{}(idx.index) ^ std::hash<uint8_t>{}(idx.subindex);
//             }
//         };
//     };

//     // Set a value in the object dictionary
//     void set(const Index idx, const ObjectDictValue& value) {
//         dict[idx] = value;
//     }

//     // Get a value from the object dictionary
//     std::optional<ObjectDictValue> get(const Index idx) const {
//         auto it = dict.find(idx);
//         if (it != dict.end()) {
//             return it->second;
//         }
//         return std::nullopt; // Return nullopt if key not found
//     }

// private:
//     // Custom hash function for Index


//     std::unordered_map<Index, ObjectDictValue, Index::Hasher> dict;
// };


// class NmtService{
//     NmtService(CanDriver& driver, uint8_t nodeId) : driver_(driver), nodeId_(nodeId) {}

//     void sendHeartbeat() {
//         // Send heartbeat message
//     }

//     void setState(uint8_t newState) {
//         // Set new state
//     }
// };
// class Sdo{

// };

// class Pdo{

// };

}