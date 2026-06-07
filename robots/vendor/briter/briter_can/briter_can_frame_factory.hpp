#pragma once

#include "briter_can_primitive.hpp"
#include "../briter_api_facade.hpp"
#include "../briter_erased_frame.hpp"
#include "middlewares/protocols/modbus/modbus_msgs.hpp"


namespace ymd::robots::briter{

struct [[nodiscard]] CanFrameFactoryBackend{

    struct ItemId{
        using enum CanCommand;
        using enum CanEnqueryId;
    }; 

    struct [[nodiscard]] State{
        const uint8_t node_id;

        constexpr ErasedFrame write_0(CanCommand item_id) const {
            return ErasedFrame::from_write_0(node_id, uint16_t(item_id));
        }

        constexpr ErasedFrame write_16(CanCommand item_id, const uint16_t reg_val) const {
            return ErasedFrame::from_write_16(node_id, uint16_t(item_id), reg_val);
        }

        constexpr ErasedFrame write_32(CanCommand item_id, const uint32_t reg_val) const {
            return ErasedFrame::from_write_32(node_id, uint16_t(item_id), reg_val);
        }

        constexpr ErasedFrame read_16(CanEnqueryId item_id) const {
            return ErasedFrame::from_read_16(node_id, uint16_t(item_id));
        }

        constexpr ErasedFrame read_32(CanEnqueryId item_id) const {
            return ErasedFrame::from_read_32(node_id, uint16_t(item_id));
        }
    };


};

using CanFrameFactory = ClientApiFacade<CanFrameFactoryBackend>;

}