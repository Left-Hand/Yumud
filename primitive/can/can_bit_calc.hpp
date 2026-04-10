#pragma once

#include "can_enum.hpp"
#include "core/utils/Option.hpp"

namespace ymd::hal::can{


// 最优优先：迭代从 NTQ = 25 开始向下枚举，确保先返回总 TQ 数最大的组合（通常时序更优）。
// 对于同一 NTQ，按 SJW 从大到小返回（SJW 越大同步能力越强）
struct BitTimmingCalculateIterator {
    using Self = BitTimmingCalculateIterator;

    // 硬件允许的最大 SJW（常见限制为 4，可调整）
    static constexpr uint8_t MAX_SJW = 4;

    struct Config {
        const uint32_t aligned_bus_clk_freq_hz;
        const uint32_t baud_freq_hz;
        const Percentage<uint8_t> sample_point;
    };

    struct Output {
        Option<NominalBitTimmingCoeffs> coeffs;  // 始终为 Some（迭代器保证）
        Percentage<uint8_t> percents;            // 实际采样点百分比（四舍五入）
    };

    struct State {
        uint8_t current_ntq;          // 当前正在处理的 NTQ（有效时才有效）
        uint8_t current_sjw;           // 下一个要返回的 SJW 值（1~4），0 表示无更多
        // 当前 NTQ 的缓存数据（仅当 current_ntq 有效时有效）
        uint16_t prescale;
        uint8_t bs1_tq;
        uint8_t bs2_tq;
        uint8_t sample_tq;

        static constexpr State zero() {
            return State{
                .current_ntq = 0,
                .current_sjw = 0,
                .prescale = 0,
                .bs1_tq = 0,
                .bs2_tq = 0,
                .sample_tq = 0,
            };
        }
    };
    const Config config_;
    State state;



    // 构造函数：从给定配置开始，定位到第一个有效组合
    constexpr BitTimmingCalculateIterator(Config config): 
        config_(config), 
        state(State::zero()) 
    {
        advance_to_next_valid_ntq(25);
    }

    // 检查是否还有更多组合
    constexpr bool has_next() const {
        return state.current_sjw != 0;
    }

    // 返回下一个组合，并更新状态
    constexpr Output next() {
        // 前提：has_next() 为 true
        uint8_t actual_percent = (state.sample_tq * 100 + state.current_ntq / 2) / state.current_ntq;
        Percentage<uint8_t> percents = Percentage<uint8_t>::from_percents_unchecked(actual_percent);

        Output res{
            .coeffs = Some(NominalBitTimmingCoeffs{
                .prescale = state.prescale,
                .swj = hal::CanTq::from_num(state.current_sjw),
                .bs1 = hal::CanTq::from_num(state.bs1_tq),
                .bs2 = hal::CanTq::from_num(state.bs2_tq)
            }),
            .percents = percents
        };

        // 更新状态：尝试当前 NTQ 的下一个 SJW（更小值）
        if (state.current_sjw > 1) {
            --state.current_sjw;
        } else {
            // 当前 NTQ 的所有 SJW 已返回，移动到下一个 NTQ
            advance_to_next_valid_ntq(state.current_ntq - 1);
        }
        return res;
    }

private:
    // 从给定的 start_ntq 开始递减，寻找下一个有效 NTQ，并初始化状态
    // 如果找到，返回 true 并更新 state；否则返回 false，state.current_sjw 置 0
    constexpr bool advance_to_next_valid_ntq(uint8_t start_ntq) {
        const uint32_t clk = config_.aligned_bus_clk_freq_hz;
        const uint32_t baud = config_.baud_freq_hz;
        const uint8_t target_percent = config_.sample_point.percents();

        for (uint8_t ntq = start_ntq; ntq >= 8; --ntq) {
            uint32_t denominator = static_cast<uint32_t>(baud) * ntq;
            if (denominator == 0) continue;
            uint32_t prescale_calc = clk / denominator;
            if (prescale_calc == 0 || prescale_calc > 1024) continue;
            if (clk != prescale_calc * denominator) continue;

            // 计算采样点所在的 TQ 位置（四舍五入）
            uint8_t sample_tq = (ntq * target_percent + 50) / 100;
            if (sample_tq <= 1 || sample_tq >= ntq) continue;  // 同步段固定为 1

            uint8_t bs1_tq = sample_tq - 1;
            uint8_t bs2_tq = ntq - sample_tq;
            if (bs1_tq < 1 || bs1_tq > 16 || bs2_tq < 1 || bs2_tq > 8) continue;

            // 有效 NTQ 找到，填充状态
            state.current_ntq = ntq;
            state.prescale = static_cast<uint16_t>(prescale_calc);
            state.bs1_tq = bs1_tq;
            state.bs2_tq = bs2_tq;
            state.sample_tq = sample_tq;

            // 确定最大允许的 SJW
            uint8_t max_sjw = bs2_tq;
            if (max_sjw > MAX_SJW) max_sjw = MAX_SJW;
            state.current_sjw = max_sjw;
            return true;
        }

        // 没有更多有效 NTQ
        state.current_ntq = 0;
        state.current_sjw = 0;
        return false;
    }
};
}
