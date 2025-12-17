#if 0

auto ringbuf_test1 = []{
    RingBuf<int, 8> buf;  // 实际容量7
    
    // 测试空状态
    ASSERT(buf.is_empty());
    ASSERT(!buf.is_full());
    ASSERT(buf.length() == 0);
    ASSERT(buf.free_capacity() == 7);
    
    // 测试单元素操作
    ASSERT(buf.try_push(1));
    ASSERT(buf.length() == 1);
    ASSERT(!buf.is_empty());
    
    int value = 0;
    ASSERT(buf.try_pop(value));
    ASSERT(value == 1);
    ASSERT(buf.is_empty());
    
    // 测试移动语义
    ASSERT(buf.try_push(2));
    ASSERT(buf.try_pop(value));
    ASSERT(value == 2);
    
    // 测试emplace
    ASSERT(buf.try_emplace(42));
    ASSERT(buf.try_pop(value));
    ASSERT(value == 42);
};

auto ringbuf_test2 = []{ 
    RingBuf<int, 8> buf;  // 实际容量7
    
    // 测试空状态
    ASSERT(buf.is_empty());
    ASSERT(!buf.is_full());
    ASSERT(buf.length() == 0);
    ASSERT(buf.free_capacity() == 7);
    
    // 测试单元素操作
    ASSERT(buf.try_push(1));
    ASSERT(buf.length() == 1);
    ASSERT(!buf.is_empty());
    
    int value = 0;
    ASSERT(buf.try_pop(value));
    ASSERT(value == 1);
    ASSERT(buf.is_empty());
    
    // 测试移动语义
    ASSERT(buf.try_push(2));
    ASSERT(buf.try_pop(value));
    ASSERT(value == 2);
    
    // 测试emplace
    ASSERT(buf.try_emplace(42));
    ASSERT(buf.try_pop(value));
    ASSERT(value == 42);
};

auto ringbuf_test3 = []{ 

    
    RingBuf<int, 16> buf;  // 实际容量15
    
    // 准备数据
    std::vector<int> src(10);
    for (int i = 0; i < 10; ++i) {
        src[i] = i + 100;
    }
    
    // 批量写入
    size_t pushed = buf.try_push(src);
    ASSERT(pushed == 10);
    ASSERT(buf.length() == 10);
    
    // 批量读取
    std::vector<int> dst(5);
    size_t popped = buf.try_pop(dst);
    ASSERT(popped == 5);
    ASSERT(buf.length() == 5);
    
    // 验证读取的数据
    for (int i = 0; i < 5; ++i) {
        ASSERT(dst[i] == i + 100);
    }
    
    // 测试分段写入/读取
    RingBuf<int, 8> small_buf;  // 实际容量7
    
    // 写入使写指针接近末尾
    for (int i = 0; i < 5; ++i) {
        (void)small_buf.try_push(i);
    }
    
    // 批量写入需要分段的数据
    std::vector<int> src2 = {10, 11, 12, 13};
    size_t pushed2 = small_buf.try_push(src2);
    ASSERT(pushed2 == 2);  // 只能写入2个（5+2=7，满了）
    

};

ringbuf_test1();
DEBUG_PRINTLN("ringbuf test1 passed");
ringbuf_test2();
DEBUG_PRINTLN("ringbuf test2 passed");
ringbuf_test3();
DEBUG_PRINTLN("ringbuf test3 passed");

PANIC{"ok"};

#endif