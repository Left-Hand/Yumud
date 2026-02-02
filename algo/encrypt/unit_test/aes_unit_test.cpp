#include "../aes.hpp"


using namespace ymd::encrypt;


namespace {
// Test basic constants
static_assert(aes::BLOCK_BYTES_SIZE == 16, "Block size should be 16 bytes");
static_assert(aes::SEED_KEY_SIZE == 32, "Seed key size should be 32 bytes");
static_assert(aes::KEYSCHEDULE_MAX_ROW == 60, "Key schedule max rows should be 60");
static_assert(aes::KEYSCHEDULE_MAX_COLUMN == 4, "Key schedule max columns should be 4");
static_assert(aes::STATE_MATRIX_SIZE == 4, "State matrix size should be 4");

// Test KeySize enum values
static_assert(static_cast<uint8_t>(aes::KeySize::_128Bits) == 0, "aes::KeySize::_128Bits should be 0");
static_assert(static_cast<uint8_t>(aes::KeySize::_192Bits) == 1, "aes::KeySize::_192Bits should be 1");
static_assert(static_cast<uint8_t>(aes::KeySize::_256Bits) == 2, "aes::KeySize::_256Bits should be 2");

// Test GF(2^8) multiplication operations
static_assert(aes::details::gfmultby01(0x57) == 0x57, "gfmultby01 failed");
static_assert(aes::details::gfmultby02(0x57) == 0xae, "gfmultby02 failed");
// 对于输入 0x57:
static_assert(aes::details::gfmultby03(0x57) == 0xF9, "gfmultby03 failed");
static_assert(aes::details::gfmultby09(0x57) == 0xD9, "gfmultby09 failed");

// Test S-box values (known constants from aes::AesCipher specification)
static_assert(aes::details::Sbox[0][0] == 0x63, "S-box [0][0] incorrect");
static_assert(aes::details::Sbox[0][1] == 0x7c, "S-box [0][1] incorrect");
static_assert(aes::details::Sbox[15][15] == 0x16, "S-box [15][15] incorrect");

// Test inverse S-box values
static_assert(aes::details::iSbox[0][0] == 0x52, "Inverse S-box [0][0] incorrect");
static_assert(aes::details::iSbox[0][1] == 0x09, "Inverse S-box [0][1] incorrect");
static_assert(aes::details::iSbox[15][15] == 0x7d, "Inverse S-box [15][15] incorrect");

// Test Rcon values
static_assert(aes::details::Rcon[0][0] == 0x00, "Rcon[0][0] incorrect");
static_assert(aes::details::Rcon[1][0] == 0x01, "Rcon[1][0] incorrect");
static_assert(aes::details::Rcon[10][0] == 0x36, "Rcon[10][0] incorrect");

using State = aes::State;
using Row = aes::Row;

using namespace aes;
// Test word operations
static_assert([]() constexpr {
    Row word = {0x12, 0x34, 0x56, 0x78};
    auto rot = aes::AesCipher::rot_word(word);
    return rot[0] == 0x34 && rot[1] == 0x56 && rot[2] == 0x78 && rot[3] == 0x12;
}(), "rot_word operation failed");

static_assert([]() constexpr {
    Row word = {0x00, 0x01, 0x02, 0x03};
    auto sub = aes::AesCipher::sub_word(word);
    return sub[0] == 0x63 && sub[1] == 0x7c && sub[2] == 0x77 && sub[3] == 0x7b;
}(), "sub_word operation failed");

// Test row rotation functions
static_assert([]() constexpr {
    Row row = {0x01, 0x02, 0x03, 0x04};
    auto left1 = aes::AesCipher::rotate_row_left<1>(row);
    return left1[0] == 0x02 && left1[1] == 0x03 && left1[2] == 0x04 && left1[3] == 0x01;
}(), "Rotate row left by 1 failed");

static_assert([]() constexpr {
    Row row = {0x01, 0x02, 0x03, 0x04};
    auto left2 = aes::AesCipher::rotate_row_left<2>(row);
    return left2[0] == 0x03 && left2[1] == 0x04 && left2[2] == 0x01 && left2[3] == 0x02;
}(), "Rotate row left by 2 failed");

static_assert([]() constexpr {
    Row row = {0x01, 0x02, 0x03, 0x04};
    auto right1 = aes::AesCipher::rotate_row_right<1>(row);
    return right1[0] == 0x04 && right1[1] == 0x01 && right1[2] == 0x02 && right1[3] == 0x03;
}(), "Rotate row right by 1 failed");

// Test aes::AesCipher-128 with FIPS-197 test vector
static_assert([]() constexpr {
    // aes::AesCipher-128 test vector from FIPS-197
    uint8_t key[16] = {
        0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6,
        0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c
    };
    
    uint8_t plaintext[16] = {
        0x32, 0x43, 0xf6, 0xa8, 0x88, 0x5a, 0x30, 0x8d,
        0x31, 0x31, 0x98, 0xa2, 0xe0, 0x37, 0x07, 0x34
    };
    
    uint8_t expected_ciphertext[16] = {
        0x39, 0x25, 0x84, 0x1d, 0x02, 0xdc, 0x09, 0xfb,
        0xdc, 0x11, 0x85, 0x97, 0x19, 0x6a, 0x0b, 0x32
    };
    
    auto aes = aes::AesCipher::from_128bits(std::span(key));
    uint8_t ciphertext[16];
    
    aes.cipher_block(plaintext, ciphertext);
    
    for (size_t i = 0; i < 16; ++i) {
        if (ciphertext[i] != expected_ciphertext[i]) {
            return false;
        }
    }
    return true;
}(), "aes::AesCipher-128 FIPS-197 test vector failed");

// Test aes::AesCipher-128 decryption with FIPS-197 test vector
static_assert([]() constexpr {
    // aes::AesCipher-128 test vector from FIPS-197 (decrypt)
    uint8_t key[16] = {
        0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6,
        0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c
    };
    
    uint8_t ciphertext[16] = {
        0x39, 0x25, 0x84, 0x1d, 0x02, 0xdc, 0x09, 0xfb,
        0xdc, 0x11, 0x85, 0x97, 0x19, 0x6a, 0x0b, 0x32
    };
    
    uint8_t expected_plaintext[16] = {
        0x32, 0x43, 0xf6, 0xa8, 0x88, 0x5a, 0x30, 0x8d,
        0x31, 0x31, 0x98, 0xa2, 0xe0, 0x37, 0x07, 0x34
    };
    
    auto aes = aes::AesCipher::from_128bits(std::span(key));
    uint8_t plaintext[16];
    
    aes.inv_cipher_block(ciphertext, plaintext);
    
    for (size_t i = 0; i < 16; ++i) {
        if (plaintext[i] != expected_plaintext[i]) {
            return false;
        }
    }
    return true;
}(), "aes::AesCipher-128 FIPS-197 decryption test vector failed");

// Test aes::AesCipher internal operations with simple data
static_assert([]() constexpr {
    // Create a simple state for testing
    State test_state{};
    test_state[0][0] = 0x12;
    test_state[0][1] = 0x34;
    test_state[0][2] = 0x56;
    test_state[0][3] = 0x78;
    test_state[1][0] = 0x9a;
    test_state[1][1] = 0xbc;
    test_state[1][2] = 0xde;
    test_state[1][3] = 0xf0;
    test_state[2][0] = 0x11;
    test_state[2][1] = 0x22;
    test_state[2][2] = 0x33;
    test_state[2][3] = 0x44;
    test_state[3][0] = 0x55;
    test_state[3][1] = 0x66;
    test_state[3][2] = 0x77;
    test_state[3][3] = 0x88;
    
    // Test shift_rows operation
    auto shifted = aes::AesCipher::shift_rows(test_state);
    // Row 0 should be unchanged
    bool row0_ok = (shifted[0][0] == test_state[0][0]) &&
                   (shifted[0][1] == test_state[0][1]) &&
                   (shifted[0][2] == test_state[0][2]) &&
                   (shifted[0][3] == test_state[0][3]);

    // Test add_round_key with zero round key
    aes::AesCipher aes_test{};
    for (size_t i = 0; i < KEYSCHEDULE_MAX_ROW; ++i) {
        for (size_t j = 0; j < KEYSCHEDULE_MAX_COLUMN; ++j) {
            aes_test.w_[i][j] = 0;
        }
    }
    aes_test.Nk_ = 4;
    aes_test.Nr_ = 10;
    
    auto added = aes_test.add_round_key(test_state, 0);
    bool add_round_key_ok = true;
    for (size_t r = 0; r < 4; ++r) {
        for (size_t c = 0; c < 4; ++c) {
            if (added[r][c] != test_state[r][c]) {
                add_round_key_ok = false;
            }
        }
    }
    
    return row0_ok && add_round_key_ok;
}(), "aes::AesCipher internal operations test");

// Test key expansion for aes::AesCipher-192
static_assert([]() constexpr {
    uint8_t key[24] = {
        0x8e, 0x73, 0xb0, 0xf7, 0xda, 0x0e, 0x64, 0x52,
        0xc8, 0x10, 0xf3, 0x2b, 0x80, 0x90, 0x79, 0xe5,
        0x62, 0xf8, 0xea, 0xd2, 0x52, 0x2c, 0x6b, 0x7b
    };
    
    auto aes = aes::AesCipher::from_192bits(std::span(key));
    
    // Verify some properties of the key schedule
    return aes.Nk_ == 6 && aes.Nr_ == 12;
}(), "aes::AesCipher-192 key expansion test");

// Test key expansion for aes::AesCipher-256
static_assert([]() constexpr {
    uint8_t key[32] = {
        0x60, 0x3d, 0xeb, 0x10, 0x15, 0xca, 0x71, 0xbe,
        0x2b, 0x73, 0xae, 0xf0, 0x85, 0x7d, 0x77, 0x81,
        0x1f, 0x35, 0x2c, 0x07, 0x3b, 0x61, 0x08, 0xd7,
        0x2d, 0x98, 0x10, 0xa3, 0x09, 0x14, 0xdf, 0xf4
    };
    
    auto aes = aes::AesCipher::from_256bits(std::span(key));
    
    // Verify some properties of the key schedule
    return aes.Nk_ == 8 && aes.Nr_ == 14;
}(), "aes::AesCipher-256 key expansion test");

// Test full encryption/decryption round-trip
static_assert([]() constexpr {
    uint8_t key[16] = {
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
        0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f
    };
    
    uint8_t plaintext[16] = {
        0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
        0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff
    };
    
    auto aes = aes::AesCipher::from_128bits(std::span(key));
    
    uint8_t ciphertext[16];
    uint8_t decrypted[16];
    
    aes.cipher_block(plaintext, ciphertext);
    aes.inv_cipher_block(ciphertext, decrypted);
    
    // Check that we get back the original plaintext
    for (size_t i = 0; i < 16; ++i) {
        if (decrypted[i] != plaintext[i]) {
            return false;
        }
    }
    return true;
}(), "aes::AesCipher encrypt/decrypt round-trip test");

// 测试全零、全1等特殊情况
static_assert([]() constexpr {
    uint8_t all_zero_key[16] = {0};
    uint8_t all_zero_plaintext[16] = {0};
    auto aes = aes::AesCipher::from_128bits(std::span(all_zero_key));
    uint8_t ciphertext[16];
    aes.cipher_block(all_zero_plaintext, ciphertext);
    return true; // 主要检查是否崩溃/编译
}(), "All-zero test");
};

