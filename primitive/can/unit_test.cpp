#include "bxcan_frame.hpp"

using namespace ymd;
using namespace ymd::hal;



// Test CanStdId
static_assert(sizeof(ymd::hal::CanStdId) == 2, "CanStdId should be 2 bytes");
static_assert(std::is_standard_layout_v<ymd::hal::CanStdId>);
static_assert(std::is_trivially_copyable_v<ymd::hal::CanStdId>);

// Test CanStdId constants
static_assert(ymd::hal::CanStdId::MAX_VALUE == 0x7ff);
static_assert(ymd::hal::CanStdId::NUM_BITS == 11);

// Test CanStdId::from_bits
static_assert([]{
    constexpr auto id = ymd::hal::CanStdId::from_bits(0x123);
    return id.to_u11() == 0x123;
}());

// Test CanStdId::from_bits with valid value
static_assert([]{
    constexpr auto id = ymd::hal::CanStdId::from_bits(0x7ff);
    return id.to_u11() == 0x7ff;
}());

// Test CanStdId::try_from_u11 with valid value
static_assert([]{
    constexpr auto maybe_id = ymd::hal::CanStdId::try_from_u11(0x123);
    return maybe_id.is_some() && maybe_id.unwrap().to_u11() == 0x123;
}());

// Test CanStdId::try_from_u11 with invalid value
static_assert([]{
    constexpr auto maybe_id = ymd::hal::CanStdId::try_from_u11(0x800);
    return maybe_id.is_none();
}());

// Test CanStdId comparison
static_assert([]{
    constexpr auto id1 = ymd::hal::CanStdId::from_bits(0x100);
    constexpr auto id2 = ymd::hal::CanStdId::from_bits(0x200);
    return id1 == id1 && id1 != id2;
}());

// Test CanStdId priority comparison
static_assert([]{
    constexpr auto high_priority = ymd::hal::CanStdId::from_bits(0x100);
    constexpr auto low_priority = ymd::hal::CanStdId::from_bits(0x200);
    return high_priority.is_senior_than(low_priority);
}());

// Test CanExtId
static_assert(sizeof(ymd::hal::CanExtId) == 4, "CanExtId should be 4 bytes");
static_assert(std::is_standard_layout_v<ymd::hal::CanExtId>);
static_assert(std::is_trivially_copyable_v<ymd::hal::CanExtId>);

// Test CanExtId constants
static_assert(ymd::hal::CanExtId::MAX_VALUE == 0x1fffffff);
static_assert(ymd::hal::CanExtId::NUM_BITS == 29);

// Test CanExtId::from_bits
static_assert([]{
    constexpr auto id = ymd::hal::CanExtId::from_bits(0x12345678);
    return id.to_u29() == 0x12345678;
}());

// Test CanExtId::from_bits with valid value
static_assert([]{
    constexpr auto id = ymd::hal::CanExtId::from_bits(0x1fffffff);
    return id.to_u29() == 0x1fffffff;
}());

// Test CanExtId::try_from_u29 with valid value
static_assert([]{
    constexpr auto maybe_id = ymd::hal::CanExtId::try_from_u29(0x12345678);
    return maybe_id.is_some() && maybe_id.unwrap().to_u29() == 0x12345678;
}());

// Test CanExtId::try_from_u29 with invalid value
static_assert([]{
    constexpr auto maybe_id = ymd::hal::CanExtId::try_from_u29(0x20000000);
    return maybe_id.is_none();
}());

// Test CanExtId comparison
static_assert([]{
    constexpr auto id1 = ymd::hal::CanExtId::from_bits(0x10000000);
    constexpr auto id2 = ymd::hal::CanExtId::from_bits(0x20000000);
    return id1 == id1 && id1 != id2;
}());

// Test CanExtId priority comparison
static_assert([]{
    constexpr auto high_priority = ymd::hal::CanExtId::from_bits(0x10000000);
    constexpr auto low_priority = ymd::hal::CanExtId::from_bits(0x20000000);
    return high_priority.is_senior_than(low_priority);
}());

// Test is_canid concept
static_assert(ymd::hal::details::is_canid<ymd::hal::CanStdId>);
static_assert(ymd::hal::details::is_canid<ymd::hal::CanExtId>);
static_assert(!ymd::hal::details::is_canid<uint32_t>);
static_assert(!ymd::hal::details::is_canid<int>);

// Test basic type properties
static_assert(std::is_standard_layout_v<BxCanFrame>);
static_assert(std::is_trivially_copyable_v<BxCanFrame>);
static_assert(sizeof(BxCanFrame) == 16, "BxCanFrame should be 16 bytes due to alignment");


static_assert(CanStdId::try_from_u11(0xABCU).is_none());
static_assert(CanExtId::try_from_u29(0xABCU).is_some());
static_assert(CanExtId::try_from_u29(UINT32_MAX).is_none());

// Test constructors and factory methods
static_assert([]{
    constexpr auto msg = BxCanFrame::from_remote(CanStdId::from_bits(0x123U));
    return msg.is_remote() && msg.id_u32() == 0x123U;
}());

static_assert([]{
    constexpr auto msg = BxCanFrame::from_empty(CanStdId::from_bits(0x456U));
    return (!msg.is_remote()) && msg.length() == 0;
}());



// Test byte array construction with sized range
static_assert([]{
    constexpr std::array<uint8_t, 3> data{1, 2, 3};
    constexpr auto msg = BxCanFrame::from_parts(CanStdId::from_bits(0x123U), BxCanPayload::from_bytes(data));
    return msg.length() == 3 && msg[0] == 1 && msg[1] == 2 && msg[2] == 3;
}());

// Test try_from_bytes with valid sized range
static_assert([]{
    constexpr std::array<uint8_t, 5> data{1, 2, 3, 4, 5};
    constexpr auto msg = BxCanFrame::from_parts(
        CanStdId::from_bits(0x123U), 
        BxCanPayload::try_from_bytes(data).unwrap()
    );
    return msg.length() == 5;
}());

// Test initializer list construction
static_assert([]{
    constexpr auto msg = BxCanFrame::from_parts(
        CanStdId::from_bits(0x123U), 
        BxCanPayload::from_list({1, 2, 3, 4})
    );
    return msg.length() == 4 && msg[0] == 1 && msg[3] == 4;
}());


// Test try_from_list with oversized data (should return None)
static_assert([]{
    constexpr auto may_payload = BxCanPayload::try_from_list({1, 2, 3, 4, 5, 6, 7, 8, 9});
    return may_payload.is_none();
}());



// Test accessors
static_assert([]{
    constexpr auto msg = BxCanFrame::from_remote(CanStdId::from_bits(0x123U));
    return msg.identifier().id_u32() == 0x123U;
}());


// Test span-based payload access
static_assert([]{
    constexpr auto may_msg = BxCanFrame::from_parts(CanStdId::from_bits(0x123U), BxCanPayload::from_list({1, 2, 3}));
    constexpr auto msg = may_msg;
    constexpr auto payload_bits = msg.payload_u64();
    static_assert(msg.dlc().length() == 3);   
    static_assert(payload_bits == 0x0000000000030201ULL);
    static_assert(msg[0] == 1);
    static_assert(msg[1] == 2);
    static_assert(msg[2] == 3);
    static_assert(msg.at_or(3,9) == 9);
    return true;
}());

// Test direct array access operators
static_assert([]{
    constexpr auto msg = BxCanFrame::from_parts(CanStdId::from_bits(0x123U), BxCanPayload::from_list({10, 20, 30}));
    return msg[0] == 10 && msg[1] == 20 && msg[2] == 30;
}());

// Test at() method with valid indices
static_assert([]{
    constexpr auto msg = BxCanFrame::from_parts(CanStdId::from_bits(0x123U), BxCanPayload::from_list({5, 10, 15}));
    return msg.at(0) == 5 && msg.at(1) == 10 && msg.at(2) == 15;
}());

// Test try_at() with valid indices
static_assert([]{
    constexpr auto msg = BxCanFrame::from_parts(CanStdId::from_bits(0x123U), BxCanPayload::from_list({7, 14, 21}));
    constexpr auto val0 = msg.try_at(0);
    constexpr auto val1 = msg.try_at(1);
    return val0.is_some() && val0.unwrap() == 7 && val1.is_some() && val1.unwrap() == 14;
}());

// Test try_at() with invalid index (should return None)
static_assert([]{
    constexpr auto msg = BxCanFrame::from_parts(CanStdId::from_bits(0x123U), BxCanPayload::from_list({1, 2, 3}));
    constexpr auto result = msg.try_at(5); // Index out of bounds
    return result.is_none();
}());



// Test frame type identification
static_assert([]{
    constexpr auto remote_msg = BxCanFrame::from_remote(CanStdId::from_bits(0x123U));
    constexpr auto data_msg = BxCanFrame::from_empty(CanStdId::from_bits(0x456U));
    return remote_msg.is_remote() && !data_msg.is_remote();
}());

// Test standard vs extended ID
static_assert([]{
    constexpr auto std_msg = BxCanFrame::from_empty(CanStdId::from_bits(0x123U));
    constexpr auto ext_msg = BxCanFrame::from_empty(CanExtId::from_bits(0x123456U));
    return std_msg.is_standard() && !std_msg.is_extended() && 
           ext_msg.is_extended() && !ext_msg.is_standard();
}());