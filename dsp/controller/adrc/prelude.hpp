#pragma once

#include "core/math/realmath.hpp"
#include "core/string/view/string_view.hpp"
#include "core/math/iq/fixed_t.hpp"
#include "core/utils/Result.hpp"
#include "dsp/state_vector.hpp"
#include "utils.hpp"


namespace ymd::dsp::adrc{

// 基于韩京清先生原始论文的非线性跟踪微分器，使用经典fhan函子计算加减速输入
template<typename T, size_t NUM_ODRS>
struct NonlinearTrackingDifferentiator;

// 基于韩京清先生原始论文的非线性拓展状态观测器
template<typename T, size_t NUM_ODRS>
struct NonlinearExtendedStateObserver;

// 基于高志强先生改进的线性跟踪微分器
template<typename T, size_t NUM_ODRS>
struct LinearTrackingDifferentiator;

// 基于高志强先生改进的线性拓展状态观测器
template<typename T, size_t NUM_ODRS>
struct LinearExtendedStateObserver;
}