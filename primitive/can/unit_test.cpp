#include "bxcan_frame.hpp"
#include "can_event.hpp"

using namespace ymd;

// Test hal::CanStdId
static_assert(sizeof(hal::CanStdId) == 2, "hal::CanStdId should be 2 bytes");
static_assert(std::is_standard_layout_v<hal::CanStdId>);
static_assert(std::is_trivially_copyable_v<hal::CanStdId>);

// Test hal::CanStdId constants
static_assert(hal::CanStdId::MAX_VALUE == 0x7ff);
static_assert(hal::CanStdId::NUM_BITS == 11);

// Test hal::CanStdId::from_bits
static_assert([]{
    constexpr auto id = hal::CanStdId::from_bits(0x123);
    return id.to_u11() == 0x123;
}());

// Test hal::CanStdId::from_bits with valid value
static_assert([]{
    constexpr auto id = hal::CanStdId::from_bits(0x7ff);
    return id.to_u11() == 0x7ff;
}());

// Test hal::CanStdId::try_from_u11 with valid value
static_assert([]{
    constexpr auto maybe_id = hal::CanStdId::try_from_u11(0x123);
    return maybe_id.is_some() && maybe_id.unwrap().to_u11() == 0x123;
}());

// Test hal::CanStdId::try_from_u11 with invalid value
static_assert([]{
    constexpr auto maybe_id = hal::CanStdId::try_from_u11(0x800);
    return maybe_id.is_none();
}());

// Test hal::CanStdId comparison
static_assert([]{
    constexpr auto id1 = hal::CanStdId::from_bits(0x100);
    constexpr auto id2 = hal::CanStdId::from_bits(0x200);
    return id1 == id1 && id1 != id2;
}());

// Test hal::CanStdId priority comparison
static_assert([]{
    constexpr auto high_priority = hal::CanStdId::from_bits(0x100);
    constexpr auto low_priority = hal::CanStdId::from_bits(0x200);
    return high_priority.is_senior_than(low_priority);
}());

// Test CanExtId
static_assert(sizeof(hal::CanExtId) == 4, "CanExtId should be 4 bytes");
static_assert(std::is_standard_layout_v<hal::CanExtId>);
static_assert(std::is_trivially_copyable_v<hal::CanExtId>);

// Test CanExtId constants
static_assert(hal::CanExtId::MAX_VALUE == 0x1fffffff);
static_assert(hal::CanExtId::NUM_BITS == 29);

// Test CanExtId::from_bits
static_assert([]{
    constexpr auto id = hal::CanExtId::from_bits(0x12345678);
    return id.to_u29() == 0x12345678;
}());

// Test CanExtId::from_bits with valid value
static_assert([]{
    constexpr auto id = hal::CanExtId::from_bits(0x1fffffff);
    return id.to_u29() == 0x1fffffff;
}());

// Test CanExtId::try_from_u29 with valid value
static_assert([]{
    constexpr auto maybe_id = hal::CanExtId::try_from_u29(0x12345678);
    return maybe_id.is_some() && maybe_id.unwrap().to_u29() == 0x12345678;
}());

// Test CanExtId::try_from_u29 with invalid value
static_assert([]{
    constexpr auto maybe_id = hal::CanExtId::try_from_u29(0x20000000);
    return maybe_id.is_none();
}());

// Test CanExtId comparison
static_assert([]{
    constexpr auto id1 = hal::CanExtId::from_bits(0x10000000);
    constexpr auto id2 = hal::CanExtId::from_bits(0x20000000);
    return id1 == id1 && id1 != id2;
}());

// Test CanExtId priority comparison
static_assert([]{
    constexpr auto high_priority = hal::CanExtId::from_bits(0x10000000);
    constexpr auto low_priority = hal::CanExtId::from_bits(0x20000000);
    return high_priority.is_senior_than(low_priority);
}());

// Test is_canid concept
static_assert(hal::details::is_canid<hal::CanStdId>);
static_assert(hal::details::is_canid<hal::CanExtId>);
static_assert(!hal::details::is_canid<uint32_t>);
static_assert(!hal::details::is_canid<int>);

// Test basic type properties
static_assert(std::is_standard_layout_v<hal::BxCanFrame>);
static_assert(std::is_trivially_copyable_v<hal::BxCanFrame>);
static_assert(sizeof(hal::BxCanFrame) == 16, "hal::BxCanFrame should be 16 bytes due to alignment");


static_assert(hal::CanStdId::try_from_u11(0xABCU).is_none());
static_assert(hal::CanExtId::try_from_u29(0xABCU).is_some());
static_assert(hal::CanExtId::try_from_u29(UINT32_MAX).is_none());

// Test constructors and factory methods
static_assert([]{
    constexpr auto frame = hal::BxCanFrame::from_remote(hal::CanStdId::from_bits(0x123U));
    return frame.is_remote() && frame.id_u32() == 0x123U;
}());

static_assert([]{
    constexpr auto frame = hal::BxCanFrame::from_empty_data(hal::CanStdId::from_bits(0x456U));
    return (!frame.is_remote()) && frame.length() == 0;
}());



// Test byte array construction with sized range
static_assert([]{
    constexpr std::array<uint8_t, 3> data{1, 2, 3};
    constexpr auto frame = hal::BxCanFrame::from_parts(
        hal::CanStdId::from_bits(0x123U), 
        hal::BxCanPayload::from_bytes(data));
    return frame.length() == 3 && frame[0] == 1 && frame[1] == 2 && frame[2] == 3;
}());

// Test try_from_bytes with valid sized range
static_assert([]{
    constexpr std::array<uint8_t, 5> data{1, 2, 3, 4, 5};
    constexpr auto frame = hal::BxCanFrame::from_parts(
        hal::CanStdId::from_bits(0x123U), 
        hal::BxCanPayload::try_from_bytes(data).unwrap()
    );
    return frame.length() == 5;
}());

// Test initializer list construction
static_assert([]{
    constexpr auto frame = hal::BxCanFrame::from_parts(
        hal::CanStdId::from_bits(0x123U), 
        hal::BxCanPayload::from_list({1, 2, 3, 4})
    );
    return frame.length() == 4 && frame[0] == 1 && frame[3] == 4;
}());


// Test try_from_list with oversized data (should return None)
static_assert([]{
    constexpr auto may_payload = hal::BxCanPayload::try_from_list({1, 2, 3, 4, 5, 6, 7, 8, 9});
    return may_payload.is_none();
}());



// Test accessors
static_assert([]{
    constexpr auto frame = hal::BxCanFrame::from_remote(hal::CanStdId::from_bits(0x123U));
    return frame.identifier().id_u32() == 0x123U;
}());


// Test span-based payload access
static_assert([]{
    constexpr auto may_frame = hal::BxCanFrame::from_parts(hal::CanStdId::from_bits(0x123U), hal::BxCanPayload::from_list({1, 2, 3}));
    constexpr auto frame = may_frame;
    constexpr auto payload_bits = frame.payload_u64();
    static_assert(frame.dlc().length() == 3);   
    static_assert(payload_bits == 0x0000000000030201ULL);
    static_assert(frame[0] == 1);
    static_assert(frame[1] == 2);
    static_assert(frame[2] == 3);
    static_assert(frame.at_or(3,9) == 9);
    return true;
}());

// Test direct array access operators
static_assert([]{
    constexpr auto frame = hal::BxCanFrame::from_parts(
        hal::CanStdId::from_bits(0x123U), 
        hal::BxCanPayload::from_list({10, 20, 30}));
    return frame[0] == 10 && frame[1] == 20 && frame[2] == 30;
}());

// Test at() method with valid indices
static_assert([]{
    constexpr auto frame = hal::BxCanFrame::from_parts(
        hal::CanStdId::from_bits(0x123U), 
        hal::BxCanPayload::from_list({5, 10, 15}));
    return frame.at(0) == 5 && frame.at(1) == 10 && frame.at(2) == 15;
}());

// Test try_at() with valid indices
static_assert([]{
    constexpr auto frame = hal::BxCanFrame::from_parts(
        hal::CanStdId::from_bits(0x123U), 
        hal::BxCanPayload::from_list({7, 14, 21}));
    constexpr auto val0 = frame.try_at(0);
    constexpr auto val1 = frame.try_at(1);
    return val0.is_some() && val0.unwrap() == 7 && val1.is_some() && val1.unwrap() == 14;
}());

// Test try_at() with invalid index (should return None)
static_assert([]{
    constexpr auto frame = hal::BxCanFrame::from_parts(
        hal::CanStdId::from_bits(0x123U), 
        hal::BxCanPayload::from_list({1, 2, 3}));
    constexpr auto result = frame.try_at(5); // Index out of bounds
    return result.is_none();
}());



// Test frame type identification
static_assert([]{
    constexpr auto remote_msg = hal::BxCanFrame::from_remote(hal::CanStdId::from_bits(0x123U));
    constexpr auto data_msg = hal::BxCanFrame::from_empty_data(hal::CanStdId::from_bits(0x456U));
    return remote_msg.is_remote() && !data_msg.is_remote();
}());

// Test standard vs extended ID
static_assert([]{
    constexpr auto std_msg = hal::BxCanFrame::from_empty_data(hal::CanStdId::from_bits(0x123U));
    constexpr auto ext_msg = hal::BxCanFrame::from_empty_data(hal::CanExtId::from_bits(0x123456U));
    return std_msg.is_standard() && !std_msg.is_extended() && 
           ext_msg.is_extended() && !ext_msg.is_standard();
}());


namespace {

[[maybe_unused]] void test_event(){
    {
        static constexpr auto ev = hal::CanEvent::from(hal::CanTransmitEvent{
            .kind = hal::CanTransmitEvent::Success,
            .mbox_idx = hal::CanMailboxIndex::_0
        });

        static constexpr auto tx_ev = ev.exact_arg<hal::CanTransmitEvent>();
        static_assert(tx_ev.kind == hal::CanTransmitEvent::Success);
        static_assert(tx_ev.mbox_idx == hal::CanMailboxIndex::_0);
    }

    {
        static constexpr auto ev = hal::CanEvent::from(hal::CanReceiveEvent{
            .kind = hal::CanReceiveEvent::Pending,
            .fifo_idx = hal::CanFifoIndex::_0
        });

        static constexpr auto rx_ev = ev.exact_arg<hal::CanReceiveEvent>();
        static_assert(rx_ev.kind == hal::CanReceiveEvent::Pending);
        static_assert(rx_ev.fifo_idx == hal::CanFifoIndex::_0);
    }

}

static_assert(sizeof(hal::CanEvent) == 8);
}