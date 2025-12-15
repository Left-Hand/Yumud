    #if 0
    // 通用算法：动态计算最佳时序参数
    // 1. 计算目标时间份额数（Time Quantum）
    constexpr uint32_t min_tq = 8;   // CAN要求最小8TQ
    constexpr uint32_t max_tq = 25;  // CAN要求最大25TQ

    const uint32_t target_tq_count = aligned_bus_clk_freq / baud_freq;

    if (target_tq_count < min_tq || target_tq_count > max_tq * 1024) { // prescaler最大1024
        __builtin_trap();
    }

    // 2. 寻找合适的预分频值
    uint32_t best_prescale = 1;
    uint32_t best_tq = 0;
    uint32_t best_error = UINT32_MAX;

    for (uint32_t prescale = 1; prescale <= 1024; ++prescale) {
        const uint32_t tq = target_tq_count / prescale;
        
        if (tq < min_tq) break;
        if (tq > max_tq) continue;
        
        // 检查是否能整除
        if ((target_tq_count % prescale) == 0) {
            // 精确匹配，优先选择
            best_prescale = prescale;
            best_tq = tq;
            break;
        }
        
        // 计算误差
        const uint32_t error = (target_tq_count > prescale * tq) 
            ? (target_tq_count - prescale * tq) 
            : (prescale * tq - target_tq_count);
        
        if (error < best_error) {
            best_error = error;
            best_prescale = prescale;
            best_tq = tq;
        }
    }

    if (best_tq == 0) {
        __builtin_trap();
    }

    // 3. 计算实际波特率和误差
    const uint32_t actual_baud = aligned_bus_clk_freq / (best_prescale * best_tq);
    const int32_t baud_error = static_cast<int32_t>(actual_baud) - static_cast<int32_t>(baud_freq);
    const int32_t error_percent = (baud_error * 10000) / static_cast<int32_t>(baud_freq); // 单位：0.01%

    // 允许的波特率误差（通常±1%）
    if (error_percent < -100 || error_percent > 100) {
        __builtin_trap();
    }

    // 4. 根据采样点分配BS1和BS2
    // 总TQ数 = 1(SJW) + BS1 + BS2
    const uint32_t total_tq = best_tq;
    const uint32_t sample_point_tq = (total_tq * sample_percents + 50) / 100; // 四舍五入

    // 根据TQ枚举值分配BS1和BS2
    uint32_t bs1_tq = 0;
    uint32_t bs2_tq = 0;
    Swj swj = Swj::_1tq;

    // 简单的分配策略：BS1包含采样点前的所有TQ（除了同步段）
    // 同步段固定为1TQ，这里已经包含在计算中
    if (sample_point_tq >= 1) {
        bs1_tq = sample_point_tq - 1; // 减去同步段
        bs2_tq = total_tq - sample_point_tq;
    } else {
        bs1_tq = 0;
        bs2_tq = total_tq - 1;
    }

    // 确保满足最小要求：BS1 ≥ 1, BS2 ≥ 1, BS2 ≥ SJW
    bs1_tq = (bs1_tq < 1) ? 1 : bs1_tq;
    bs2_tq = (bs2_tq < 1) ? 1 : bs2_tq;

    // 调整SJW：应为BS1和BS2中较小的1/4
    const uint32_t max_sjw = (bs1_tq < bs2_tq ? bs1_tq : bs2_tq) / 4;
    if (max_sjw >= 4) {
        swj = Swj::_4tq;
    } else if (max_sjw >= 3) {
        swj = Swj::_3tq;
    } else if (max_sjw >= 2) {
        swj = Swj::_2tq;
    } else {
        swj = Swj::_1tq;
    }

    // 5. 转换为枚举值（这里需要根据实际的Bs1、Bs2枚举类型调整）
    // 假设Bs1、Bs2有对应的from_tq方法或类似机制
    return {
        .prescale = best_prescale,
        .swj = swj,
        .bs1 = Bs1::from_tq(bs1_tq), // 需要实现这个方法
        .bs2 = Bs2::from_tq(bs2_tq)  // 需要实现这个方法
    };
    #endif