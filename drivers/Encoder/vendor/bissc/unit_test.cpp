#include "bissc.hpp"

using namespace ymd;
using namespace ymd::drivers;


#include <bitset>
#include <cmath>
#include "bissc.hpp"

using namespace ymd;
using namespace ymd::drivers;




#if 0
int main() {
    std::cout << "BISS-C Protocol Parser Test" << std::endl;
    std::cout << "===========================" << std::endl;
    

    // 测试2: 不同分辨率测试
    std::cout << "\nTest 2: Different resolution tests" << std::endl;
    {
        struct TestCase {
            size_t resolution;
            uint32_t position;
        };
        
        TestCase test_cases[] = {
            {10, 0x1FF},   // 10位分辨率
            {14, 0x1FFF},  // 14位分辨率
            {16, 0x3FFF},  // 16位分辨率
            {18, 0xFFFF},  // 18位分辨率
            {20, 0x3FFFF}  // 20位分辨率
        };
        
        bool all_passed = true;
        for(auto& tc : test_cases) {
            uint64_t test_data = encode(tc.position, tc.resolution);
            auto result = bissc::parse(test_data, tc.resolution);
            
            if(result.has_value()) {
                uint32_t parsed_pos = result.value().to_bits();
                uint32_t extracted_pos = parsed_pos >> (32 - tc.resolution);
                
                bool passed = (extracted_pos == tc.position);
                std::cout << "Resolution " << tc.resolution << " (" << std::hex << "0x" << tc.position << std::dec 
                         << "): " << (passed ? "PASS" : "FAIL") << std::endl;
                
                if(!passed) all_passed = false;
            } else {
                std::cout << "Resolution " << tc.resolution << ": FAIL - Parse returned None" << std::endl;
                all_passed = false;
            }
        }
        
        std::cout << "Test 2 Overall: " << (all_passed ? "PASS" : "FAIL") << std::endl;
    }
    
    // 测试3: 边界值测试
    std::cout << "\nTest 3: Boundary value tests" << std::endl;
    {
        size_t res = 12;
        
        // 测试最小值 (0)
        {
            uint64_t test_data = encode(0, res);
            auto result = bissc::parse(test_data, res);
            if(result.has_value()) {
                uint32_t parsed_pos = result.value().to_bits();
                uint32_t extracted_pos = parsed_pos >> (32 - res);
                std::cout << "Min value (0): " << (extracted_pos == 0 ? "PASS" : "FAIL") << std::endl;
                assert(extracted_pos == 0 && "Min value test failed");
            } else {
                std::cout << "Min value (0): FAIL" << std::endl;
                assert(false && "Parse should not return None for valid data");
            }
        }
        
        // 测试最大值
        {
            uint32_t max_val = (1 << res) - 1;
            uint64_t test_data = encode(max_val, res);
            auto result = bissc::parse(test_data, res);
            if(result.has_value()) {
                uint32_t parsed_pos = result.value().to_bits();
                uint32_t extracted_pos = parsed_pos >> (32 - res);
                std::cout << "Max value (0x" << std::hex << max_val << std::dec << "): " 
                         << (extracted_pos == max_val ? "PASS" : "FAIL") << std::endl;
                assert(extracted_pos == max_val && "Max value test failed");
            } else {
                std::cout << "Max value (0x" << std::hex << max_val << std::dec << "): FAIL" << std::endl;
                assert(false && "Parse should not return None for valid data");
            }
        }
    }
    
    // 测试4: CRC校验错误检测
    std::cout << "\nTest 4: CRC error detection test" << std::endl;
    {
        uint32_t test_pos = 0x555;
        uint64_t valid_data = encode(test_pos, 12);
        
        // 修改数据使其CRC错误
        uint64_t invalid_data = valid_data ^ (1ULL << 3); // 翻转一位导致CRC错误
        
        auto result = bissc::parse(invalid_data, 12);
        bool crc_detected = !result.has_value();
        
        std::cout << "CRC error detection: " << (crc_detected ? "PASS" : "FAIL") << std::endl;
        assert(crc_detected && "CRC error should be detected");
    }
    
    // 测试5: 特殊情况测试 - 全零输入
    std::cout << "\nTest 5: Special case - zero input" << std::endl;
    {
        auto result = bissc::parse(0, 12);
        // 对于全零输入，由于没有起始位(1)，解析应该失败
        bool handled_correctly = !result.has_value(); // 应该返回None
        std::cout << "Zero input handling: " << (handled_correctly ? "PASS" : "FAIL") << std::endl;
    }
    
    std::cout << "\nAll tests completed successfully!" << std::endl;
    return 0;
}

#endif


namespace{




[[maybe_unused]] static void test_12b(){
    #if 0
    {

        constexpr size_t resolution = 12;
        constexpr uint32_t position_12b = ((0.2f * (1 << resolution)));  // 假设位置值
        constexpr bool error = false;
        constexpr bool warning = false;

        constexpr auto encoded = bissc::encode(
            uq32::from_bits(position_12b << (32 - resolution)), 
            resolution, false, error, warning
        );
        constexpr auto decoded_12b = bissc::parse(encoded, resolution)
            .unwrap().to_bits() >> (32 - resolution);
        
        // assert(decoded_12b == position_12b);
        static_assert(decoded_12b == 3);
    }

    #endif
}

static_assert(bissc::fast_clzll(0x0000'0000'0000'0000) == 64);  // 全零
static_assert(bissc::fast_clzll(0x8000'0000'0000'0000) == 0);   // 最高位为1
static_assert(bissc::fast_clzll(0x4000'0000'0000'0000) == 1);   // 第二位为1
static_assert(bissc::fast_clzll(0x0000'0000'8000'0000) == 32);  // 高32位全零，低32位最高位为1
static_assert(bissc::fast_clzll(0x0000'0001'0000'0000) == 31);  // 跨越32位边界的情况
static_assert(bissc::fast_clzll(0x0000'0000'0000'0001) == 63);  // 仅最低位为1
static_assert(bissc::fast_clzll(0xFFFF'FFFF'FFFF'FFFF) == 0);   // 全部为1
static_assert(bissc::fast_clzll(0x0000'FFFF'FFFF'FFFF) == 16);  // 低16位为0
}