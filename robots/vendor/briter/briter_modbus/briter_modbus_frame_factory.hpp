#pragma once

#include "briter_modbus_primitive.hpp"
#include "../briter_api_facade.hpp"
#include "../briter_erased_frame.hpp"
#include "middlewares/protocols/modbus/modbus_msgs.hpp"

namespace ymd::robots::briter{


struct [[nodiscard]] ModbusFrameFactoryBackend{
    using ItemId = ModbusRegAddr;

    struct [[nodiscard]] State{
        const uint8_t node_id;

        constexpr ErasedFrame write_16(ItemId reg_addr, const uint16_t reg_val) const {
            return ErasedFrame::from_write_16(node_id, uint16_t(reg_addr), reg_val);
        }

        constexpr ErasedFrame write_32(ItemId reg_addr, const uint32_t reg_val) const {
            return ErasedFrame::from_write_32(node_id, uint16_t(reg_addr), reg_val);
        }

        constexpr ErasedFrame read_16(ItemId reg_addr) const {
            return ErasedFrame::from_read_16(node_id, uint16_t(reg_addr));
        }

        constexpr ErasedFrame read_32(ItemId reg_addr) const {
            return ErasedFrame::from_read_32(node_id, uint16_t(reg_addr));
        }
    };


};

using ModbusFrameFactory = ClientApiFacade<ModbusFrameFactoryBackend>;

}