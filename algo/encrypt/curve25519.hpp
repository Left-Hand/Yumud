#pragma once

#include "curve25519_cstyle.hpp"
#include "core/utils/result.hpp"

// Curve25519 是目前最高水平的 Diffie-Hellman函数，适用于广泛的场景，由Daniel J. Bernstein教授设计。
// 在密码学中，Curve25519是一个椭圆曲线提供128位安全性，设计用于椭圆曲线Diffie-Hellman（ECDH）密钥协商方案。
// 它是最快的ECC曲线之一，并未被任何已知专利所涵盖。


namespace ymd::encrypt::curve25519{


enum class Curve25519Error : uint8_t {
    Success = 0,
    InvalidPublicKeySize,
    InvalidSecretKeySize,
    InvalidBasepointSize,
    NullptrProvided,
    KeyGenerationFailed,
    InvalidKeyFormat
};


// 输出流重载 for error
inline OutputStream & operator<<(OutputStream & os, Curve25519Error error) {
    switch (error) {
        case Curve25519Error::Success: return os << "Success";
        case Curve25519Error::InvalidPublicKeySize: return os << "InvalidPublicKeySize";
        case Curve25519Error::InvalidSecretKeySize: return os << "InvalidSecretKeySize";
        case Curve25519Error::InvalidBasepointSize: return os << "InvalidBasepointSize";
        case Curve25519Error::NullptrProvided: return os << "NullptrProvided";
        case Curve25519Error::KeyGenerationFailed: return os << "KeyGenerationFailed";
        case Curve25519Error::InvalidKeyFormat: return os << "InvalidKeyFormat";
    }
    __builtin_unreachable();
}
// Curve25519 面向对象封装
struct Curve25519 {
public:
    static constexpr size_t KEY_SIZE = 32;
    using Key = std::array<uint8_t, KEY_SIZE>;

    
    // 共享密钥类型
    using SharedSecret = std::array<uint8_t, KEY_SIZE>;
    

    
    // 生成随机密钥对
    [[nodiscard]] static constexpr Result<Key, Curve25519Error> generate_public_from_private(
        std::span<const uint8_t, KEY_SIZE> private_key
    ) {
        const auto public_key = derive_public_key(private_key);
        
        return Ok(public_key);
    }
    
    // 从私钥推导公钥
    [[nodiscard]] static constexpr std::array<uint8_t, KEY_SIZE>
    derive_public_key(std::span<const uint8_t, KEY_SIZE> private_key) {
        std::array<uint8_t, KEY_SIZE> public_key{};
        std::array<uint8_t, KEY_SIZE> basepoint = {9}; // Curve25519 默认基点
        
        generate_core(public_key, private_key, basepoint);
        return public_key;
    }
    
    // 计算共享密钥
    [[nodiscard]] constexpr Result<SharedSecret,  Curve25519Error> 
    compute_shared_secret(
        std::span<const uint8_t, KEY_SIZE> their_public_key,
        std::span<const uint8_t, KEY_SIZE> private_key
    ) const noexcept {
        SharedSecret shared_secret{};
        generate_core(shared_secret, private_key, their_public_key);
        return Ok(shared_secret);
    }
    
    
    // 验证密钥对是否匹配
    [[nodiscard]] static constexpr Result<void, void> validate_keypair(
        std::span<const uint8_t, KEY_SIZE> public_key, 
        std::span<const uint8_t, KEY_SIZE> private_key
    ) {
        auto derived_public = derive_public_key(private_key);
        if(equal_key(derived_public, public_key) == false) return Err();
        return Ok();
    }
    
private:
    static constexpr bool equal_key(
        std::span<const uint8_t, KEY_SIZE> a, 
        std::span<const uint8_t, KEY_SIZE> b
    ){
        for(size_t i = 0; i < KEY_SIZE; i++){
            if(a[i] != b[i]) return false;
        }
        return true;
    }
    // 核心生成函数
    static constexpr void generate_core(
        std::span<uint8_t, KEY_SIZE> mypublic, 
        std::span<const uint8_t, KEY_SIZE> secret, 
        std::span<const uint8_t, KEY_SIZE> basepoint
    ) {
        int64_t bp[10], x[10], z[11], zmone[10];
        uint8_t e[KEY_SIZE];

        // 复制并清理 secret
        for(size_t i = 0; i < KEY_SIZE; ++i) e[i] = secret[i];
        e[0] &= 248;
        e[31] &= 127;
        e[31] |= 64;

        details::fexpand(bp, basepoint.data());
        details::cmult(x, z, e, bp);
        details::crecip(zmone, z);
        details::fmul(z, x, zmone);
        details::fcontract(mypublic.data(), z);
    }
};

}