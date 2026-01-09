#include "algo/encrypt/lz77.hpp"

#include "core/debug/debug.hpp"
#include "core/clock/time.hpp"
#include "core/utils/sumtype.hpp"
#include "core/utils/Match.hpp"


using namespace ymd;    
using namespace ymd::encrypt::lz77;

void test_simple_compress_decompress() {
    std::vector<uint8_t> src = { 'a', 'b', 'c', 'a', 'b', 'c', 'd' };
    std::vector<uint8_t> compressed(1024);
    std::vector<uint8_t> decompressed(1024);
    
    auto compress_result = compress(compressed, src);
    ASSERT(compress_result.is_ok());
    
    auto decompress_result = decompress(decompressed, std::span(compressed).subspan(0, compress_result.unwrap()));
    ASSERT(decompress_result.is_ok());
    
    ASSERT(decompress_result.unwrap() == src.size());
    for (size_t i = 0; i < src.size(); ++i) {
        ASSERT(decompressed[i] == src[i]);
    }
    
    DEBUGGER << "Simple compress/decompress test passed\n";
}

void test_repeated_pattern() {
    std::string pattern = "abcabcabc";
    std::vector<uint8_t> src(pattern.begin(), pattern.end());
    std::vector<uint8_t> compressed(1024);
    std::vector<uint8_t> decompressed(1024);
    
    auto compress_result = compress(compressed, src);
    ASSERT(compress_result.is_ok());
    
    auto decompress_result = decompress(decompressed, std::span(compressed).subspan(0, compress_result.unwrap()));
    ASSERT(decompress_result.is_ok());
    
    ASSERT(decompress_result.unwrap() == src.size());
    for (size_t i = 0; i < src.size(); ++i) {
        ASSERT(decompressed[i] == src[i]);
    }
    
    DEBUGGER << "Repeated pattern test passed\n";
}

void test_no_compression() {
    std::vector<uint8_t> src = { 'a', 'b', 'c', 'd', 'e', 'f' }; // No repeated patterns
    std::vector<uint8_t> compressed(1024);
    std::vector<uint8_t> decompressed(1024);
    
    auto compress_result = compress(compressed, src);
    ASSERT(compress_result.is_ok());
    
    auto decompress_result = decompress(decompressed, std::span(compressed).subspan(0, compress_result.unwrap()));
    ASSERT(decompress_result.is_ok());
    
    ASSERT(decompress_result.unwrap() == src.size());
    for (size_t i = 0; i < src.size(); ++i) {
        ASSERT(decompressed[i] == src[i]);
    }
    
    DEBUGGER << "No compression test passed\n";
}

void test_large_repetition() {
    std::vector<uint8_t> src(1000, 'x'); // Large repetition
    std::vector<uint8_t> compressed(1024);
    std::vector<uint8_t> decompressed(1024);
    
    auto compress_result = compress(compressed, src);
    ASSERT(compress_result.is_ok());
    
    auto decompress_result = decompress(decompressed, std::span(compressed).subspan(0, compress_result.unwrap()));
    ASSERT(decompress_result.is_ok());
    
    ASSERT(decompress_result.unwrap() == src.size());
    for (size_t i = 0; i < src.size(); ++i) {
        ASSERT(decompressed[i] == src[i]);
    }
    
    DEBUGGER << "Large repetition test passed\n";
}

void test_insufficient_output_buffer() {
    std::vector<uint8_t> src = { 'a', 'b', 'c', 'a', 'b', 'c' };
    std::vector<uint8_t> compressed(3); // Too small buffer
    
    auto compress_result = compress(compressed, src);
    ASSERT(compress_result.is_err());
    ASSERT(compress_result.unwrap_err() == Error::OutOfMemory);
    
    DEBUGGER << "Insufficient output buffer test passed\n";
}

void test_complex_pattern() {
    std::string text = "ABCDEF_ABCDEF_ABCDEF"; // Pattern with underscore
    std::vector<uint8_t> src(text.begin(), text.end());
    std::vector<uint8_t> compressed(1024);
    std::vector<uint8_t> decompressed(1024);
    
    auto compress_result = compress(compressed, src);
    ASSERT(compress_result.is_ok());
    
    auto decompress_result = decompress(decompressed, std::span(compressed).subspan(0, compress_result.unwrap()));
    ASSERT(decompress_result.is_ok());
    
    ASSERT(decompress_result.unwrap() == src.size());
    for (size_t i = 0; i < src.size(); ++i) {
        ASSERT(decompressed[i] == src[i]);
    }
    
    DEBUGGER << "Complex pattern test passed\n";
}

void test_empty_input() {
    std::vector<uint8_t> src; // Empty input
    std::vector<uint8_t> compressed(1024);
    std::vector<uint8_t> decompressed(1024);
    
    auto compress_result = compress(compressed, src);
    ASSERT(compress_result.is_ok());
    ASSERT(compress_result.unwrap() == 0);
    
    auto decompress_result = decompress(decompressed, std::span(compressed).subspan(0, compress_result.unwrap()));
    ASSERT(decompress_result.is_ok());
    ASSERT(decompress_result.unwrap() == 0);
    
    DEBUGGER << "Empty input test passed\n";
}

int lz77_main() {
    test_simple_compress_decompress();
    test_repeated_pattern();
    test_no_compression();
    test_large_repetition();
    test_insufficient_output_buffer();
    test_complex_pattern();
    test_empty_input();
    
    DEBUGGER << "All tests passed!\n";
    return 0;
}