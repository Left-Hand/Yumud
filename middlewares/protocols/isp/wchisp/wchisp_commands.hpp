#pragma once

#include <cstdint>
#include "wchisp_primitive.hpp"
#include <span>
#include "core/utils/Result.hpp"


namespace ymd::wchisp::commands{


//0xa1
struct [[nodiscard]] Identify final{
    static constexpr CommandKind COMMAND_KIND = CommandKind{0xa1};

    uint8_t device_id;
    uint8_t device_type;

    static constexpr uint8_t KEY[16] = {
        // "MCU ISP & WCH.CN"
        'M', 'C', 'U', ' ',
        'I', 'S', 'P', ' ',
        '&', ' ', 'W', 'C',
        'H', '.', 'C', 'N',
    };

    template<typename Receiver>
    constexpr Result<void, typename Receiver::Error> serialize() const{
        auto & self = *this;

        {
            const uint8_t buffer[] = {
                static_cast<uint8_t>(COMMAND_KIND),
                0x12, 0x00,
                self.device_id, self.device_type
            };

            if(const auto res = Receiver::push_bytes(buffer); 
                res.is_err()) return Err(res.unwrap_err());
        }

        {
            if(const auto res = Receiver::push_bytes(std::span(KEY)); 
                res.is_err()) return Err(res.unwrap_err());
        }

        return Ok();
    }
};


//0xa2
struct [[nodiscard]] IspEnd final{
    static constexpr CommandKind COMMAND_KIND = CommandKind{0xa2};

    uint8_t reason;

    template<typename Receiver>
    constexpr Result<void, typename Receiver::Error> serialize() const{
        auto & self = *this;

        {
            const uint8_t buffer[] = {
                static_cast<uint8_t>(COMMAND_KIND),
                0x01, 0x00,
                self.reason
            };

            if(const auto res = Receiver::push_bytes(buffer); 
                res.is_err()) return Err(res.unwrap_err());
        }


        return Ok();
    }
};


//0xa3
struct [[nodiscard]] IspKey final{
    static constexpr CommandKind COMMAND_KIND = CommandKind{0xa3};

    std::span<const uint8_t> key;

    template<typename Receiver>
    constexpr Result<void, typename Receiver::Error> serialize() const{
        auto & self = *this;

        {
            const uint8_t buffer[] = {
                static_cast<uint8_t>(COMMAND_KIND),
                static_cast<uint8_t>(key.size()),
                0x00
            };

            if(const auto res = Receiver::push_bytes(buffer); 
                res.is_err()) return Err(res.unwrap_err());
        }

        {
            if(const auto res = Receiver::push_bytes(std::span(key)); 
                res.is_err()) return Err(res.unwrap_err());
        }



        return Ok();
    }
};


//0xa4
struct [[nodiscard]] Erase final{
    static constexpr CommandKind COMMAND_KIND = CommandKind{0xa4};

    uint32_t sectors;


    template<typename Receiver>
    constexpr Result<void, typename Receiver::Error> serialize() const{
        auto & self = *this;

        {
            const uint8_t buffer[] = {
                static_cast<uint8_t>(COMMAND_KIND),
                0x04, 0x00,
                static_cast<uint8_t>(self.sectors & 0xFF),
                static_cast<uint8_t>((self.sectors >> 8) & 0xFF),
                static_cast<uint8_t>((self.sectors >> 16) & 0xFF),
                static_cast<uint8_t>((self.sectors >> 24) & 0xFF),
            };

            if(const auto res = Receiver::push_bytes(std::span(buffer)); 
                res.is_err()) return Err(res.unwrap_err());
        }


        return Ok();
    }
};


//0xa5
struct [[nodiscard]] Program final{
    static constexpr CommandKind COMMAND_KIND = CommandKind{0xa5};

    uint32_t address;
    uint8_t padding;
    std::span<const uint8_t> data;

    template<typename Receiver>
    constexpr Result<void, typename Receiver::Error> serialize() const{
        auto & self = *this;

        const uint16_t payload_size = static_cast<uint16_t>(1 + 2 + 4 + 1 + self.data.size() - 3);
        
        {
            const uint8_t buffer_header[] = {
                static_cast<uint8_t>(COMMAND_KIND),
                static_cast<uint8_t>(payload_size & 0xFF),
                static_cast<uint8_t>((payload_size >> 8) & 0xFF),
                static_cast<uint8_t>(self.address & 0xFF),
                static_cast<uint8_t>((self.address >> 8) & 0xFF),
                static_cast<uint8_t>((self.address >> 16) & 0xFF),
                static_cast<uint8_t>((self.address >> 24) & 0xFF),
                self.padding
            };

            if(const auto res = Receiver::push_bytes(std::span(buffer_header)); 
                res.is_err()) return Err(res.unwrap_err());
        }

        {
            if(self.data.size() > 0){
                if(const auto res = Receiver::push_bytes(self.data); 
                    res.is_err()) return Err(res.unwrap_err());
            }
        }

        return Ok();
    }
};


//0xa6
struct [[nodiscard]] Verify final{
    static constexpr CommandKind COMMAND_KIND = CommandKind{0xa6};


    uint32_t address;
    uint8_t padding;
    std::span<const uint8_t> data;

    template<typename Receiver>
    constexpr Result<void, typename Receiver::Error> serialize() const{
        auto & self = *this;

        const uint16_t payload_size = static_cast<uint16_t>(1 + 2 + 4 + 1 + self.data.size() - 3);
        
        {
            const uint8_t buffer_header[] = {
                static_cast<uint8_t>(COMMAND_KIND),
                static_cast<uint8_t>(payload_size & 0xFF),
                static_cast<uint8_t>((payload_size >> 8) & 0xFF),
                static_cast<uint8_t>(self.address & 0xFF),
                static_cast<uint8_t>((self.address >> 8) & 0xFF),
                static_cast<uint8_t>((self.address >> 16) & 0xFF),
                static_cast<uint8_t>((self.address >> 24) & 0xFF),
                self.padding
            };

            if(const auto res = Receiver::push_bytes(std::span(buffer_header)); 
                res.is_err()) return Err(res.unwrap_err());
        }

        {
            if(self.data.size() > 0){
                if(const auto res = Receiver::push_bytes(self.data); 
                    res.is_err()) return Err(res.unwrap_err());
            }
        }

        return Ok();
    }
};


//0xa7
struct [[nodiscard]] ReadConfig final{
    static constexpr CommandKind COMMAND_KIND = CommandKind{0xa7};

    uint8_t bit_mask;

    template<typename Receiver>
    constexpr Result<void, typename Receiver::Error> serialize() const{
        auto & self = *this;

        {
            const uint8_t buffer[] = {
                static_cast<uint8_t>(COMMAND_KIND),
                0x02, 0x00,
                self.bit_mask,
                0x00
            };

            if(const auto res = Receiver::push_bytes(std::span(buffer)); 
                res.is_err()) return Err(res.unwrap_err());
        }

        return Ok();
    }
};


//0xa8
struct [[nodiscard]] WriteConfig final{
    static constexpr CommandKind COMMAND_KIND = CommandKind{0xa8};

    uint8_t bit_mask;
    std::span<const uint8_t> data;

    template<typename Receiver>
    constexpr Result<void, typename Receiver::Error> serialize() const{
        auto & self = *this;

        const uint16_t payload_size = static_cast<uint16_t>(2 + self.data.size());

        {
            const uint8_t buffer_header[] = {
                static_cast<uint8_t>(COMMAND_KIND),
                static_cast<uint8_t>(payload_size & 0xFF),
                static_cast<uint8_t>((payload_size >> 8) & 0xFF),
                self.bit_mask,
                0x00
            };

            if(const auto res = Receiver::push_bytes(std::span(buffer_header)); 
                res.is_err()) return Err(res.unwrap_err());
        }

        {
            if(self.data.size() > 0){
                if(const auto res = Receiver::push_bytes(self.data); 
                    res.is_err()) return Err(res.unwrap_err());
            }
        }

        return Ok();
    }
};


//0xa9
struct [[nodiscard]] DataErase final{
    static constexpr CommandKind COMMAND_KIND = CommandKind{0xa9};

    uint32_t sectors;

    template<typename Receiver>
    constexpr Result<void, typename Receiver::Error> serialize() const{
        auto & self = *this;

        {
            const uint8_t buffer[] = {
                static_cast<uint8_t>(COMMAND_KIND),
                0x05, 0x00,
                0x00, 0x00, 0x00, 0x00,
                0x00
            };
            // Write sectors at position 7 (as single byte per Rust reference)
            buffer[7] = static_cast<uint8_t>(self.sectors & 0xFF);

            if(const auto res = Receiver::push_bytes(std::span(buffer)); 
                res.is_err()) return Err(res.unwrap_err());
        }

        return Ok();
    }
};


//0xaa
struct [[nodiscard]] DataProgram final{
    static constexpr CommandKind COMMAND_KIND = CommandKind{0xaa};

    uint32_t address;
    uint8_t padding;
    std::span<const uint8_t> data;

    template<typename Receiver>
    constexpr Result<void, typename Receiver::Error> serialize() const{
        auto & self = *this;

        const uint16_t payload_size = static_cast<uint16_t>(1 + 2 + 4 + 1 + self.data.size() - 3);
        
        {
            const uint8_t buffer_header[] = {
                static_cast<uint8_t>(COMMAND_KIND),
                static_cast<uint8_t>(payload_size & 0xFF),
                static_cast<uint8_t>((payload_size >> 8) & 0xFF),
                static_cast<uint8_t>(self.address & 0xFF),
                static_cast<uint8_t>((self.address >> 8) & 0xFF),
                static_cast<uint8_t>((self.address >> 16) & 0xFF),
                static_cast<uint8_t>((self.address >> 24) & 0xFF),
                self.padding
            };

            if(const auto res = Receiver::push_bytes(std::span(buffer_header)); 
                res.is_err()) return Err(res.unwrap_err());
        }

        {
            if(self.data.size() > 0){
                if(const auto res = Receiver::push_bytes(self.data); 
                    res.is_err()) return Err(res.unwrap_err());
            }
        }

        return Ok();
    }
};


//0xab
struct [[nodiscard]] DataRead final{
    static constexpr CommandKind COMMAND_KIND = CommandKind{0xab};

    uint32_t address;
    uint16_t len;

    template<typename Receiver>
    constexpr Result<void, typename Receiver::Error> serialize() const{
        auto & self = *this;

        {
            const uint8_t buffer[] = {
                static_cast<uint8_t>(COMMAND_KIND),
                0x06, 0x00,
                static_cast<uint8_t>(self.address & 0xFF),
                static_cast<uint8_t>((self.address >> 8) & 0xFF),
                static_cast<uint8_t>((self.address >> 16) & 0xFF),
                static_cast<uint8_t>((self.address >> 24) & 0xFF),
                static_cast<uint8_t>(self.len & 0xFF),
                static_cast<uint8_t>((self.len >> 8) & 0xFF)
            };

            if(const auto res = Receiver::push_bytes(std::span(buffer)); 
                res.is_err()) return Err(res.unwrap_err());
        }

        return Ok();
    }
};


//0xc3
struct [[nodiscard]] WriteOtp final{
    static constexpr CommandKind COMMAND_KIND = CommandKind{0xc3};

    uint8_t bits;

    template<typename Receiver>
    constexpr Result<void, typename Receiver::Error> serialize() const{
        auto & self = *this;

        {
            const uint8_t buffer[] = {
                static_cast<uint8_t>(COMMAND_KIND),
                0x01, 0x00,
                self.bits
            };

            if(const auto res = Receiver::push_bytes(std::span(buffer)); 
                res.is_err()) return Err(res.unwrap_err());
        }

        return Ok();
    }
};


//0xc4
struct [[nodiscard]] ReadOtp final{
    static constexpr CommandKind COMMAND_KIND = CommandKind{0xc4};

    uint8_t bits;

    template<typename Receiver>
    constexpr Result<void, typename Receiver::Error> serialize() const{
        auto & self = *this;

        {
            const uint8_t buffer[] = {
                static_cast<uint8_t>(COMMAND_KIND),
                0x01, 0x00,
                self.bits
            };

            if(const auto res = Receiver::push_bytes(std::span(buffer)); 
                res.is_err()) return Err(res.unwrap_err());
        }

        return Ok();
    }
};


//0xc5
struct [[nodiscard]] SetBaud final{
    static constexpr CommandKind COMMAND_KIND = CommandKind{0xc5};

    uint32_t baudrate;

    template<typename Receiver>
    constexpr Result<void, typename Receiver::Error> serialize() const{
        auto & self = *this;

        {
            const uint8_t buffer[] = {
                static_cast<uint8_t>(COMMAND_KIND),
                0x04, 0x00,
                static_cast<uint8_t>(self.baudrate & 0xFF),
                static_cast<uint8_t>((self.baudrate >> 8) & 0xFF),
                static_cast<uint8_t>((self.baudrate >> 16) & 0xFF),
                static_cast<uint8_t>((self.baudrate >> 24) & 0xFF)
            };

            if(const auto res = Receiver::push_bytes(std::span(buffer)); 
                res.is_err()) return Err(res.unwrap_err());
        }

        return Ok();
    }
};


}