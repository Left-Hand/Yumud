#include "algo/encrypt/lz77.hpp"

#include "core/debug/debug.hpp"
#include "core/clock/time.hpp"
#include "core/utils/sumtype.hpp"
#include "core/utils/Match.hpp"

#include "hal/bus/uart/hw_singleton.hpp"


using namespace ymd;    
using namespace ymd::encrypt;

void test_simple_compress_decompress() {
    std::vector<uint8_t> src = { 'a', 'b', 'c', 'a', 'b', 'c', 'd' };
    std::vector<uint8_t> compressed(1024);
    std::vector<uint8_t> decompressed(1024);
    
    auto compress_result = lz77::compress(compressed, src);
    ASSERT(compress_result.is_ok());
    
    auto decompress_result = lz77::decompress(decompressed, 
        
        std::span(compressed).subspan(0, compress_result.unwrap()));
    ASSERT(decompress_result.is_ok());
    
    ASSERT(decompress_result.unwrap() == src.size());
    for (size_t i = 0; i < src.size(); ++i) {
        ASSERT(decompressed[i] == src[i]);
    }
    
    DEBUG_PRINTLN("Simple compress/decompress test passed");
}

void test_repeated_pattern() {
    // std::string pattern = "abcabcabc";
    char pattern[] = "abcabcabccnbaijdfvberjksnqjkeacsdnfj;kfwen;v;f3nwecd";
    std::vector<uint8_t> src(std::begin(pattern), std::end(pattern));
    std::vector<uint8_t> compressed(1024);
    std::vector<uint8_t> decompressed(1024);
    
    auto compress_result = lz77::compress(compressed, src);
    ASSERT(compress_result.is_ok());
    
    auto decompress_result = lz77::decompress(decompressed, 
        std::span(compressed).subspan(0, compress_result.unwrap()));
    ASSERT(decompress_result.is_ok());

    DEBUG_PRINTLN(std::span(compressed).subspan(0, compress_result.unwrap()));
    
    ASSERT(decompress_result.unwrap() == src.size());
    for (size_t i = 0; i < src.size(); ++i) {
        ASSERT(decompressed[i] == src[i]);
    }
    
    DEBUG_PRINTLN("Repeated pattern test passed");
}

void test_no_compression() {
    std::vector<uint8_t> src = { 'a', 'b', 'c', 'd', 'e', 'f' }; // No repeated patterns
    std::vector<uint8_t> compressed(1024);
    std::vector<uint8_t> decompressed(1024);
    
    auto compress_result = lz77::compress(compressed, src);
    ASSERT(compress_result.is_ok());
    
    auto decompress_result = lz77::decompress(decompressed, 
        std::span(compressed).subspan(0, compress_result.unwrap()));
    ASSERT(decompress_result.is_ok());
    
    ASSERT(decompress_result.unwrap() == src.size());
    for (size_t i = 0; i < src.size(); ++i) {
        ASSERT(decompressed[i] == src[i]);
    }
    
    DEBUG_PRINTLN("No compression test passed");
}

void test_large_repetition() {
    std::vector<uint8_t> src(1000, 'x'); // Large repetition
    std::vector<uint8_t> compressed(1024);
    std::vector<uint8_t> decompressed(1024);
    
    auto compress_result = lz77::compress(compressed, src);
    ASSERT(compress_result.is_ok());
    
    auto decompress_result = lz77::decompress(decompressed, 
        std::span(compressed).subspan(0, compress_result.unwrap()));
    ASSERT(decompress_result.is_ok());
    
    ASSERT(decompress_result.unwrap() == src.size());
    for (size_t i = 0; i < src.size(); ++i) {
        ASSERT(decompressed[i] == src[i]);
    }
    
    DEBUG_PRINTLN("Large repetition test passed");
}

void test_insufficient_output_buffer() {
    std::vector<uint8_t> src = { 'a', 'b', 'c', 'a', 'b', 'c' };
    std::vector<uint8_t> compressed(3); // Too small buffer
    
    auto compress_result = lz77::compress(compressed, src);
    ASSERT(compress_result.is_err());
    ASSERT(compress_result.unwrap_err() == lz77::Error::OutOfMemory);
    
    DEBUG_PRINTLN("Insufficient output buffer test passed");
}

void test_complex_pattern() {
    std::string text = "ABCDEF_ABCDEF_ABCDEF"; // Pattern with underscore
    std::vector<uint8_t> src(text.begin(), text.end());
    std::vector<uint8_t> compressed(1024);
    std::vector<uint8_t> decompressed(1024);
    
    auto compress_result = lz77::compress(compressed, src);
    ASSERT(compress_result.is_ok());
    
    auto decompress_result = lz77::decompress(decompressed, 
        std::span(compressed).subspan(0, compress_result.unwrap()));
    ASSERT(decompress_result.is_ok());
    
    ASSERT(decompress_result.unwrap() == src.size());
    for (size_t i = 0; i < src.size(); ++i) {
        ASSERT(decompressed[i] == src[i]);
    }
    
    DEBUG_PRINTLN("Complex pattern test passed");
}

void test_empty_input() {
    std::vector<uint8_t> src; // Empty input
    std::vector<uint8_t> compressed(1024);
    std::vector<uint8_t> decompressed(1024);
    
    auto compress_result = lz77::compress(compressed, src);
    ASSERT(compress_result.is_ok());
    ASSERT(compress_result.unwrap() == 0);
    
    auto decompress_result = lz77::decompress(decompressed, 
        std::span(compressed).subspan(0, compress_result.unwrap()));
    ASSERT(decompress_result.is_ok());
    ASSERT(decompress_result.unwrap() == 0);
    
    DEBUG_PRINTLN("Empty input test passed");
}

void lz77_main() {
    auto & DEBUG_UART = hal::usart2;

    auto bsp_init_debug = [&]{
        DEBUG_UART.init({
            .remap = hal::USART2_REMAP_PA2_PA3,
            .baudrate = hal::NearestFreq(576000),
            .tx_strategy = CommStrategy::Blocking
        });

        DEBUGGER.retarget(&DEBUG_UART);
        DEBUGGER.no_brackets(EN);
        DEBUGGER.set_eps(3);
        DEBUGGER.force_sync(EN);
        DEBUGGER.no_fieldname(DISEN);
    };

    bsp_init_debug();


    test_simple_compress_decompress();
    test_repeated_pattern();
    test_no_compression();
    test_large_repetition();
    test_insufficient_output_buffer();
    test_complex_pattern();
    test_empty_input();
    
    PANIC("All tests passed!");
}