#pragma once

#include "utils.hpp"
#include "PdoProtocol.hpp"

namespace ymd::canopen{



class PdoSession {
public:
    using Driver = CanDriver;
    using CobId = uint16_t;

    // Different types of transmission types
    static constexpr int TRANS_SYNC_ACYCLIC = 0;
    static constexpr int TRANS_SYNC_MIN = 1;
    static constexpr int TRANS_SYNC_MAX = 240;
    static constexpr int TRANS_RTR_SYNC = 252;
    static constexpr int TRANS_RTR = 253;
    static constexpr int TRANS_EVENT_SPECIFIC = 254;
    static constexpr int TRANS_EVENT_PROFILE = 255;

    PdoSession(PdoProtocol & pdo, OdEntry& params, OdEntry& mapping)
        : pdo_(pdo), params_(params), mapping_(mapping) {
        cobId = int(params[1].unwarp());
        pdo.addCobId(cobId);
    }

    bool start() {
        if (isEnabled) {
            return false;
        }
        isEnabled = true;
        return true;
    }

    bool stop() {
        if (!isEnabled) {
            return false;
        }
        isEnabled = false;
        return true;
    }

    bool processMessage(const CanMessage& msg) {
        if (!isEnabled) {
            return false;
        }
        if (msg.id() != cobId) {
            return false;
        }

        int numMaps = int(mapping_[0].unwarp());
        int totalBits = 0;
        std::vector<uint8_t> data(msg.data(), msg.data() + msg.size());

        for (int i = 1; i <= numMaps; i++) {
            int map = int(mapping_[i].unwarp());
            int index = 0x0000ffff & (map >> 16);
            int sub = 0x000000ff & (map >> 8);
            int bits = 0x000000ff & (map);
            SubEntry& se = pdo_.getSubEntry(index, sub).unwarp();

            if (bits == 8) {
                uint8_t b1 = data[totalBits / 8];
                se.set(b1);
            } else if (bits == 16) {
                uint16_t s1 = (data[totalBits / 8] | (data[totalBits / 8 + 1] << 8));
                se.set(s1);
            } else if (bits == 32) {
                uint32_t i1 = (data[totalBits / 8] | (data[totalBits / 8 + 1] << 8) |
                              (data[totalBits / 8 + 2] << 16) | (data[totalBits / 8 + 3] << 24));
                se.set(i1);
            }

            totalBits += bits;
            if (totalBits > 64) {
                break;
            }
        }
        return true;
    }

    bool syncEvent() {
        int id = cobId;
        if ((id & 0x8000) == 0x8000) {
            return false;
        }
        int transType = int(params_[2].unwarp());
        if (transType >= TRANS_SYNC_MIN && transType <= TRANS_SYNC_MAX) {
            // Cyclic synchronous type
            transSyncCount++;
            if (transSyncCount >= transType) {
                transSyncCount = 0;
                CanMessage msg = buildMessage();
                lastMsg = msg;
                pdo_.sendMessage(lastMsg);
            }
        } else if (transType == TRANS_RTR_SYNC) {
            CanMessage msg = buildMessage();
            lastMsg = msg;
            // pdo.sendMessage(lastMsg);
        } else if (transType == TRANS_SYNC_ACYCLIC) {
            CanMessage msg = buildMessage();
            if (!msg == lastMsg) {
                lastMsg = msg;
                pdo_.sendMessage(lastMsg);
            }
        }
        // DEBUG_PRINTLN("PdoSession.syncEvent() cobId: 0x" + toIndexFmt(cobId.load()));
        return true;
    }

private:
    PdoProtocol & pdo_;
    OdEntry & params_;
    OdEntry & mapping_;
    CobId cobId;
    bool isEnabled = false;
    int transSyncCount = 0;
    CanMessage lastMsg;

    // static std::string toIndexFmt(int index) {
    //     std::stringstream ss;
    //     ss << std::hex << std::uppercase << std::setw(4) << std::setfill('0') << index;
    //     return ss.str();
    // }

    CanMessage buildMessage() {
        std::vector<uint8_t> data(8, 0);
        int numMaps = int(mapping_[0].unwarp());
        int totalBits = 0;

        for (int i = 1; i <= numMaps; i++) {
            int map = int(mapping_[i].unwarp());
            // DEBUG_PRINTLN("PdoSession.buildMessage() map: 0x" + toIndexFmt(map));
            int index = 0x0000ffff & (map >> 16);
            int sub = 0x000000ff & (map >> 8);
            int bits = 0x000000ff & (map);
            int val = int(pdo_.getSubEntry(index, sub).unwarp());
            // DEBUG_PRINTLN("PdoSession.buildMessage() val: 0x" + toIndexFmt(val) + " bits:" + std::to_string(bits));

            if (bits == 8) {
                data[totalBits / 8] = static_cast<uint8_t>(val);
            } else if (bits == 16) {
                data[totalBits / 8] = static_cast<uint8_t>(val & 0xFF);
                data[totalBits / 8 + 1] = static_cast<uint8_t>((val >> 8) & 0xFF);
            } else if (bits == 32) {
                data[totalBits / 8] = static_cast<uint8_t>(val & 0xFF);
                data[totalBits / 8 + 1] = static_cast<uint8_t>((val >> 8) & 0xFF);
                data[totalBits / 8 + 2] = static_cast<uint8_t>((val >> 16) & 0xFF);
                data[totalBits / 8 + 3] = static_cast<uint8_t>((val >> 24) & 0xFF);
            } else {
            }
            totalBits += bits;
            if (totalBits > 64) {
                break;
            }
        }

        int numBytes = totalBits / 8;
        if ((totalBits % 8) != 0) {
            numBytes++;
        }
        if (numBytes > 8) {
            numBytes = 8;
            totalBits = 8 * 8;
            // std::cout << "Error mapping results in msg overflow, truncating" << std::endl;
        }

        return CanMessage(cobId, data.data(), numBytes);
    }
};

}