#pragma once

#include "tamagawa_primitive.hpp"

// 参考资料
// http://gjwdoc.longbos.com/#/g?q=2506b479c8fc

namespace ymd::drivers::tamagawa{


namespace req_msgs{
using namespace primitive;


// REQ[0x1A] 全部信息
struct [[nodiscard]] GetAllInfo final {
    using Self = GetAllInfo;

    static constexpr CfCode CF_CODE = CfCode::GetAllInfo;
    static constexpr size_t CONTEXT_LENGTH = 0;
};


// REQ[0x02] ABS信息
struct [[nodiscard]] GetAbs final{
    using Self = GetAbs;

    static constexpr CfCode CF_CODE = CfCode::GetAbs;
    static constexpr size_t CONTEXT_LENGTH = 0;
};

// REQ[0x8A] ABM信息
struct [[nodiscard]] GetAbm final {
    using Self = GetAbm;

    static constexpr CfCode CF_CODE = CfCode::GetAbm;
    static constexpr size_t CONTEXT_LENGTH = 0;
};

// REQ[0x92] 型号与版本信息
struct [[nodiscard]] GetVersion final {
    using Self = GetVersion;

    static constexpr CfCode CF_CODE = CfCode::GetVersion;
    static constexpr size_t CONTEXT_LENGTH = 0;
};


// REQ[0x62] ABM清零，故障清除
struct [[nodiscard]] ClearAbmAndFault final {
    using Self = ClearAbmAndFault;

    static constexpr CfCode CF_CODE = CfCode::ClearAbmAndFault;
    static constexpr size_t CONTEXT_LENGTH = 0;
};


// REQ[0xC2] ABS清零
struct [[nodiscard]] ClearAbs final {
    using Self = ClearAbs;

    static constexpr CfCode CF_CODE = CfCode::ClearAbs;
    static constexpr size_t CONTEXT_LENGTH = 0;
};

// REQ[0x32] 写指定地址数据
struct [[nodiscard]] WriteEEprom final {
    using Self = WriteEEprom;

    static constexpr CfCode CF_CODE = CfCode::WriteEEprom;
    static constexpr size_t CONTEXT_LENGTH = 2;

    uint8_t address;
    uint8_t val;

    template<typename Serializer>
    constexpr Result<void, typename Serializer::Error> 
    serialize_context(Serializer & serializer) const noexcept{
        auto & self = *this;
        const uint8_t buffer[] = {self.address, self.val};
        if(const auto res = serializer.push_bytes(std::span(buffer));
            res.is_err()) return Err(res.unwrap_err());

        return Ok();
    }

    static constexpr Self from_bytes(
        std::span<const uint8_t, CONTEXT_LENGTH> bytes
    ) noexcept {
        return Self{
            .address = bytes[0],
            .val = bytes[1]
        };
    }
};

// REQ[0xEA] 读指定地址数据
struct [[nodiscard]] ReadEEprom final {
    using Self = ReadEEprom;

    static constexpr CfCode CF_CODE = CfCode::ReadEEprom;
    static constexpr size_t CONTEXT_LENGTH = 1;

    uint8_t address;

    template<typename Serializer>
    constexpr Result<void, typename Serializer::Error> 
    serialize_context(Serializer & serializer) const noexcept{
        auto & self = *this;
        const uint8_t buffer[] = {self.address};
        if(const auto res = serializer.push_bytes(std::span(buffer));
            res.is_err()) return Err(res.unwrap_err());

        return Ok();
    }

    static constexpr Self from_bytes(
        std::span<const uint8_t, CONTEXT_LENGTH> bytes
    ) noexcept {
        return Self{
            .address = bytes[0]
        };
    }
};


}


namespace resp_msgs{
using namespace primitive;



// RESP[0x1A] 全部信息
struct [[nodiscard]] GetAllInfo final {
    using Self = GetAllInfo;

    static constexpr CfCode CF_CODE = CfCode::GetAllInfo;
    static constexpr size_t CONTEXT_LENGTH = 9;

    StatusField sf;
    Abs24 abs;
    uint8_t enc_id;
    Abm24 abm;
    Almc almc;

    template<typename Serializer>
    constexpr Result<void, typename Serializer::Error> 
    serialize_context(Serializer & serializer) const noexcept{
        auto & self = *this;
        const uint8_t buffer[] = {
            std::bit_cast<uint8_t>(self.sf),
            self.abs.bytes[0], self.abs.bytes[1], self.abs.bytes[2],
            self.enc_id,
            self.abm.bytes[0], self.abm.bytes[1], self.abm.bytes[2],
            std::bit_cast<uint8_t>(self.almc)
        };
        if(const auto res = serializer.push_bytes(std::span(buffer));
            res.is_err()) return Err(res.unwrap_err());

        return Ok();
    }

    static constexpr GetAllInfo from_bytes(
        std::span<const uint8_t, CONTEXT_LENGTH> bytes
    ) noexcept {
        return GetAllInfo{
            .sf = StatusField::from_u8(bytes[0]),
            .abs = Abs24{.bytes = {bytes[1], bytes[2], bytes[3]}},
            .enc_id = bytes[4],
            .abm = Abm24{.bytes = {bytes[5], bytes[6], bytes[7]}},
            .almc = std::bit_cast<Almc>(bytes[8]),
        };
    }
};


// RESP[0x02] ABS信息
struct [[nodiscard]] GetAbs final{
    using Self = GetAbs;

    static constexpr CfCode CF_CODE = CfCode::GetAbs;
    static constexpr size_t CONTEXT_LENGTH = 4;

    StatusField sf;
    Abs24 abs;

    template<typename Serializer>
    constexpr Result<void, typename Serializer::Error> 
    serialize_context(Serializer & serializer) const noexcept{
        auto & self = *this;
        const uint8_t buffer[] = {
            std::bit_cast<uint8_t>(self.sf),
            self.abs.bytes[0], self.abs.bytes[1], self.abs.bytes[2]
        };
        if(const auto res = serializer.push_bytes(std::span(buffer));
            res.is_err()) return Err(res.unwrap_err());

        return Ok();
    }

    static constexpr GetAbs from_bytes(
        std::span<const uint8_t, CONTEXT_LENGTH> bytes
    ) noexcept{
        return GetAbs{
            .sf = StatusField::from_u8(bytes[0]),
            .abs = Abs24{.bytes = {bytes[1], bytes[2], bytes[3]}},
        };
    }
};

// RESP[0x8A] ABM信息
struct [[nodiscard]] GetAbm final {
    using Self = GetAbm;

    static constexpr CfCode CF_CODE = CfCode::GetAbm;
    static constexpr size_t CONTEXT_LENGTH = 4;

    StatusField sf;
    Abm24 abm;

    template<typename Serializer>
    constexpr Result<void, typename Serializer::Error> 
    serialize_context(Serializer & serializer) const noexcept{
        auto & self = *this;
        const uint8_t buffer[] = {
            std::bit_cast<uint8_t>(self.sf),
            self.abm.bytes[0], self.abm.bytes[1], self.abm.bytes[2]
        };
        if(const auto res = serializer.push_bytes(std::span(buffer));
            res.is_err()) return Err(res.unwrap_err());

        return Ok();
    }

    static constexpr GetAbm from_bytes(
        std::span<const uint8_t, CONTEXT_LENGTH> bytes
    ) noexcept {
        return GetAbm{
            .sf = StatusField::from_u8(bytes[0]),
            .abm = Abm24{.bytes = {bytes[1], bytes[2], bytes[3]}},
        };
    }
};

// RESP[0x92] 型号与版本信息
struct [[nodiscard]] GetVersion final {
    using Self = GetVersion;

    static constexpr CfCode CF_CODE = CfCode::GetVersion;
    static constexpr size_t CONTEXT_LENGTH = 8;

    struct{
        uint16_t low;
        uint16_t high;
    }encoder_product_code;

    struct{
        uint16_t low;
        uint16_t high;
    } firmware_version;

    template<typename Serializer>
    constexpr Result<void, typename Serializer::Error> 
    serialize_context(Serializer & serializer) const noexcept{
        auto & self = *this;
        const auto buffer = std::bit_cast<std::array<uint8_t, 8>>(self);
        if(const auto res = serializer.push_bytes(std::span(buffer));
            res.is_err()) return Err(res.unwrap_err());

        return Ok();
    }

    static constexpr GetVersion from_bytes(
        std::span<const uint8_t, CONTEXT_LENGTH> bytes
    ) noexcept {
        std::array<uint8_t, 8> buffer;
        std::copy(bytes.begin(), bytes.end(), buffer.data());
        return std::bit_cast<GetVersion>(buffer);
    }
};


// RESP[0x62] ABM清零，故障清除
struct [[nodiscard]] ClearAbmAndFault final {
    using Self = ClearAbmAndFault;

    static constexpr CfCode CF_CODE = CfCode::ClearAbmAndFault;
    static constexpr size_t CONTEXT_LENGTH = 4;

    StatusField sf;
    Abs24 abs;

    template<typename Serializer>
    constexpr Result<void, typename Serializer::Error> 
    serialize_context(Serializer & serializer) const noexcept{
        auto & self = *this;
        const uint8_t buffer[] = {
            std::bit_cast<uint8_t>(self.sf),
            self.abs.bytes[0], self.abs.bytes[1], self.abs.bytes[2]
        };
        if(const auto res = serializer.push_bytes(std::span(buffer));
            res.is_err()) return Err(res.unwrap_err());

        return Ok();
    }

    static constexpr ClearAbmAndFault from_bytes(
        std::span<const uint8_t, CONTEXT_LENGTH> bytes
    ) noexcept {
        return ClearAbmAndFault{
            .sf = StatusField::from_u8(bytes[0]),
            .abs = Abs24{.bytes = {bytes[1], bytes[2], bytes[3]}},
        };
    }
};


// RESP[0xC2] ABS清零
struct [[nodiscard]] ClearAbs final {
    using Self = ClearAbs;

    static constexpr CfCode CF_CODE = CfCode::ClearAbs;
    static constexpr size_t CONTEXT_LENGTH = 4;

    StatusField sf;
    Abs24 abs;

    template<typename Serializer>
    constexpr Result<void, typename Serializer::Error> 
    serialize_context(Serializer & serializer) const noexcept{
        auto & self = *this;
        const uint8_t buffer[] = {
            std::bit_cast<uint8_t>(self.sf),
            self.abs.bytes[0], self.abs.bytes[1], self.abs.bytes[2]
        };
        if(const auto res = serializer.push_bytes(std::span(buffer));
            res.is_err()) return Err(res.unwrap_err());

        return Ok();
    }

    static constexpr ClearAbs from_bytes(
        std::span<const uint8_t, CONTEXT_LENGTH> bytes
    ) noexcept {
        return ClearAbs{
            .sf = StatusField::from_u8(bytes[0]),
            .abs = Abs24{.bytes = {bytes[1], bytes[2], bytes[3]}},
        };
    }
};

// RESP[0x32] 写指定地址数据
struct [[nodiscard]] WriteEEprom final {
    using Self = WriteEEprom;

    static constexpr CfCode CF_CODE = CfCode::WriteEEprom;
    static constexpr size_t CONTEXT_LENGTH = 2;

    uint8_t address;
    uint8_t val;

    template<typename Serializer>
    constexpr Result<void, typename Serializer::Error> 
    serialize_context(Serializer & serializer) const noexcept{
        auto & self = *this;
        const uint8_t buffer[] = {self.address, self.val};
        if(const auto res = serializer.push_bytes(std::span(buffer));
            res.is_err()) return Err(res.unwrap_err());

        return Ok();
    }

    static constexpr WriteEEprom from_bytes(
        std::span<const uint8_t, CONTEXT_LENGTH> bytes
    ) noexcept {
        return WriteEEprom{
            .address = bytes[0],
            .val = bytes[1],
        };
    }
};

// RESP[0xEA] 读指定地址数据
struct [[nodiscard]] ReadEEprom final {
    using Self = ReadEEprom;

    static constexpr CfCode CF_CODE = CfCode::ReadEEprom;
    static constexpr size_t CONTEXT_LENGTH = 2;

    uint8_t address;
    uint8_t val;

    template<typename Serializer>
    constexpr Result<void, typename Serializer::Error> 
    serialize_context(Serializer & serializer) const noexcept{
        auto & self = *this;
        const uint8_t buffer[] = {self.address, self.val};
        if(const auto res = serializer.push_bytes(std::span(buffer));
            res.is_err()) return Err(res.unwrap_err());

        return Ok();
    }

    static constexpr ReadEEprom from_bytes(
        std::span<const uint8_t, CONTEXT_LENGTH> bytes
    ) noexcept {
        return ReadEEprom{
            .address = bytes[0],
            .val = bytes[1],
        };
    }
};

}

#if 0

struct [[nodiscard]] Response final{
    using Self = Response;

    CfCode cf;

    union{
        GetAbs _0;
        GetAbm _1;
        GetVersion _2;
        GetAllInfo _3;
        ClearFault _7;
        ClearAbs _8;
        ClearAbmAndFault _c;
        std::array<uint8_t, 10> bytes;
    }context;

    [[nodiscard]] std::span<const uint8_t> as_bytes() const noexcept {
        return std::span{reinterpret_cast<const uint8_t *>(this), 11};
    }

};

#endif
}