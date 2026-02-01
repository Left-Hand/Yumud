#pragma once

#include "middlewares/nvcv2/nvcv2.hpp"
#include "core/tmp/std_container.hpp"
#include <queue>


namespace ymd::nvcv2{

// BFS 寻路算法 图片中0为空 非0为障碍
template<template<typename> typename Allocator>
std::vector<math::Vec2<uint16_t>, Allocator<math::Vec2<uint16_t>>> bfs_pathfind(
    Allocator<math::Vec2<uint16_t>>& alloc,
    const Image<uint8_t>& image, 
    const math::Vec2<uint16_t>& start_point, 
    const math::Vec2<uint16_t>& stop_point
){
    auto _check_boundary = [&](
        const math::Vec2<uint16_t> size
    ){
        // 边界检查
        if(start_point.x >= size.x || start_point.y >= size.y ||
            stop_point.x >= size.x || stop_point.y >= size.y
        ) {
            PANIC("invalid: out of range");
        }
    };
    _check_boundary(image.size(), start_point, stop_point);
    if(start_point == stop_point) {
        std::vector<math::Vec2<uint16_t>, Allocator<math::Vec2<uint16_t>>> result(alloc);
        result.reserve(1);
        result.push_back(start_point);
        return result;
    }

    const uint16_t width = static_cast<uint16_t>(image.width());
    const uint16_t height = static_cast<uint16_t>(image.height());
    const size_t total_nodes = static_cast<size_t>(width) * height;
    
    // 使用一维数组存储节点信息，提高缓存局部性
    struct alignas(4) NodeInfo {
        uint16_t parent_idx = 0; // 使用索引而不是坐标，避免 optional 开销
        bool visited = false;
    };
    
    // 创建分配器实例用于NodeInfo
    using NodeAllocator = Allocator<NodeInfo>;
    NodeAllocator node_alloc;
    
    // 分配节点数组
    std::vector<NodeInfo, NodeAllocator> nodes(total_nodes, node_alloc);
    
    // 坐标到索引的转换函数
    auto pos_to_idx = [width](uint16_t x, uint16_t y) -> size_t {
        return static_cast<size_t>(y) * width + x;
    };
    
    // 索引到坐标的转换函数
    auto idx_to_pos = [width](size_t idx) -> math::Vec2<uint16_t> {
        return {
            static_cast<uint16_t>(idx % width),
            static_cast<uint16_t>(idx / width)
        };
    };
    
    // BFS队列，存储节点索引（使用默认分配器即可）
    std::queue<size_t> q;
    
    // 初始化起始节点
    size_t start_idx = pos_to_idx(start_point.x, start_point.y);
    nodes[start_idx].visited = true;
    nodes[start_idx].parent_idx = start_idx; // 根节点的父节点指向自己
    q.push(start_idx);
    
    size_t stop_idx = pos_to_idx(stop_point.x, stop_point.y);
    
    // 方向向量（上、右、下、左）
    constexpr int16_t dx[4] = {0, 1, 0, -1};
    constexpr int16_t dy[4] = {-1, 0, 1, 0};
    
    bool path_found = false;
    
    while (!q.empty()) {
        size_t current_idx = q.front();
        q.pop();
        
        // 检查是否到达目标
        if (current_idx == stop_idx) {
            path_found = true;
            break;
        }
        
        math::Vec2<uint16_t> current_pos = idx_to_pos(current_idx);
        
        // 探索邻居
        for (size_t i = 0; i < 4; ++i) {
            // 检查加法是否溢出
            int32_t new_x = static_cast<int32_t>(current_pos.x) + dx[i];
            int32_t new_y = static_cast<int32_t>(current_pos.y) + dy[i];
            
            // 检查边界
            if (new_x < 0 || new_x >= width || 
                new_y < 0 || new_y >= height) {
                continue;
            }
            
            uint16_t neighbor_x = static_cast<uint16_t>(new_x);
            uint16_t neighbor_y = static_cast<uint16_t>(new_y);
            
            // 跳过障碍物
            if (image(neighbor_x, neighbor_y) != 0) {
                continue;
            }
            
            size_t neighbor_idx = pos_to_idx(neighbor_x, neighbor_y);
            
            // 跳过已访问节点
            if (nodes[neighbor_idx].visited) {
                continue;
            }
            
            // 标记为已访问并设置父节点
            nodes[neighbor_idx].visited = true;
            nodes[neighbor_idx].parent_idx = current_idx;
            q.push(neighbor_idx);
        }
    }
    
    // 未找到路径
    if (!path_found) {
        return std::vector<math::Vec2<uint16_t>, Allocator<math::Vec2<uint16_t>>>(alloc);
    }
    
    // 重建路径（从终点到起点），使用传入的分配器
    std::vector<math::Vec2<uint16_t>, Allocator<math::Vec2<uint16_t>>> path(alloc);
    size_t current_idx = stop_idx;
    
    // 反向追踪直到起点
    while (current_idx != start_idx) {
        path.push_back(idx_to_pos(current_idx));
        current_idx = nodes[current_idx].parent_idx;
    }
    
    // 添加起点
    path.push_back(start_point);
    
    // 反转路径（起点到终点）
    std::reverse(path.begin(), path.end());
    
    return path;
}


template<template<typename> typename Allocator>
std::vector<math::Vec2<uint16_t>, Allocator<math::Vec2<uint16_t>>> bfs_pathfind_8(
    Allocator<math::Vec2<uint16_t>>& alloc,
    const Image<uint8_t>& image, 
    const math::Vec2<uint16_t>& start_point, 
    const math::Vec2<uint16_t>& stop_point
){
    auto _check_boundary = [&](
        const math::Vec2<uint16_t> size
    ){
        // 边界检查
        if(start_point.x >= size.x || start_point.y >= size.y ||
            stop_point.x >= size.x || stop_point.y >= size.y
        ) {
            PANIC("invalid: out of range");
        }
    };
    _check_boundary(image.size());
    if(start_point == stop_point) {
        std::vector<math::Vec2<uint16_t>, Allocator<math::Vec2<uint16_t>>> result(alloc);
        result.reserve(1);
        result.push_back(start_point);
        return result;
    }

    const uint16_t width = static_cast<uint16_t>(image.width());
    const uint16_t height = static_cast<uint16_t>(image.height());
    const size_t total_nodes = static_cast<size_t>(width) * height;
    
    // 使用一维数组存储节点信息
    struct alignas(4) NodeInfo {
        uint16_t parent_idx = 0;
        bool visited = false;
    };
    
    // 创建分配器实例用于NodeInfo
    using NodeAllocator = typename std::allocator_traits<Allocator<math::Vec2<uint16_t>>>::
        template rebind_alloc<NodeInfo>;
    NodeAllocator node_alloc(alloc);
    
    // 分配节点数组
    std::vector<NodeInfo, NodeAllocator> nodes(total_nodes, node_alloc);
    
    // 坐标到索引的转换函数
    auto pos_to_idx = [width](uint16_t x, uint16_t y) -> size_t {
        return static_cast<size_t>(y) * width + x;
    };
    
    // 索引到坐标的转换函数
    auto idx_to_pos = [width](size_t idx) -> math::Vec2<uint16_t> {
        return {
            static_cast<uint16_t>(idx % width),
            static_cast<uint16_t>(idx / width)
        };
    };
    
    // BFS队列
    std::queue<size_t> q;
    
    // 初始化起始节点
    size_t start_idx = pos_to_idx(start_point.x, start_point.y);
    nodes[start_idx].visited = true;
    nodes[start_idx].parent_idx = static_cast<uint16_t>(start_idx); // 根节点的父节点指向自己
    q.push(start_idx);
    
    size_t stop_idx = pos_to_idx(stop_point.x, stop_point.y);
    
    // 8方向向量 - 修改顺序，优先检查水平/垂直方向
    // 顺序：上、下、左、右（水平/垂直方向），然后是对角线方向
    constexpr int16_t dx[8] = {0, 0, -1, 1, -1, 1, 1, -1};
    constexpr int16_t dy[8] = {-1, 1, 0, 0, -1, -1, 1, 1};
    // 前4个是水平/垂直方向，后4个是对角线方向
    
    bool path_found = false;
    
    while (!q.empty()) {
        size_t current_idx = q.front();
        q.pop();
        
        // 检查是否到达目标
        if (current_idx == stop_idx) {
            path_found = true;
            break;
        }
        
        math::Vec2<uint16_t> current_pos = idx_to_pos(current_idx);
        
        // 探索邻居 (8个方向)
        for (size_t i = 0; i < 8; ++i) {
            int32_t new_x = static_cast<int32_t>(current_pos.x) + dx[i];
            int32_t new_y = static_cast<int32_t>(current_pos.y) + dy[i];
            
            // 检查边界
            if (new_x < 0 || new_x >= width || 
                new_y < 0 || new_y >= height) {
                continue;
            }
            
            uint16_t neighbor_x = static_cast<uint16_t>(new_x);
            uint16_t neighbor_y = static_cast<uint16_t>(new_y);
            
            // 检查是否为障碍物（0表示可通行）
            if (image(neighbor_x, neighbor_y) != 0) {
                continue;
            }
            
            size_t neighbor_idx = pos_to_idx(neighbor_x, neighbor_y);
            
            // 跳过已访问节点
            if (nodes[neighbor_idx].visited) {
                continue;
            }
            
            // 对于对角线移动，检查相邻的水平/垂直方向是否可通行
            // 防止"切角"通过障碍物
            if (i >= 4) { // 对角线方向（后4个）
                // 检查两个相邻的水平/垂直方向是否可通行
                uint16_t check_x1 = static_cast<uint16_t>(current_pos.x);
                uint16_t check_y1 = neighbor_y;
                uint16_t check_x2 = neighbor_x;
                uint16_t check_y2 = static_cast<uint16_t>(current_pos.y);
                
                // 如果任一相邻方向是障碍物，则不允许对角线移动
                if (image(check_x1, check_y1) != 0 || 
                    image(check_x2, check_y2) != 0) {
                    continue;
                }
            }
            
            // 标记为已访问并设置父节点
            nodes[neighbor_idx].visited = true;
            nodes[neighbor_idx].parent_idx = static_cast<uint16_t>(current_idx);
            q.push(neighbor_idx);
        }
    }
    
    // 未找到路径
    if (!path_found) {
        return std::vector<math::Vec2<uint16_t>, Allocator<math::Vec2<uint16_t>>>(alloc);
    }
    
    // 重建路径
    std::vector<math::Vec2<uint16_t>, Allocator<math::Vec2<uint16_t>>> path(alloc);
    size_t current_idx = stop_idx;
    
    // 反向追踪直到起点
    while (current_idx != start_idx) {
        path.push_back(idx_to_pos(current_idx));
        current_idx = nodes[current_idx].parent_idx;
    }
    
    // 添加起点
    path.push_back(start_point);
    
    // 反转路径（起点到终点）
    std::reverse(path.begin(), path.end());
    
    return path;
}

//双向BFS 寻路算法 图片中0为空 非0为障碍
template<template<typename> typename Allocator>
std::vector<math::Vec2<uint16_t>, Allocator<math::Vec2<uint16_t>>> bidirectional_bfs(
    const Image<uint8_t> image, 
    const math::Vec2<uint16_t> start_point, 
    const math::Vec2<uint16_t> stop_point
){
    auto _check_boundary = [&](
        const math::Vec2<uint16_t> size
    ){
        // 边界检查
        if(start_point.x >= size.x || start_point.y >= size.y ||
            stop_point.x >= size.x || stop_point.y >= size.y
        ) {
            PANIC("invalid: out of range");
        }
    };
    _check_boundary(image.size(), start_point, stop_point);

    if(start_point == stop_point) {
        std::vector<math::Vec2<uint16_t>, Allocator<math::Vec2<uint16_t>>> result;
        result.reserve(1);
        result.push_back(start_point);
        return result;
    }

    struct Node {
        bool visited_forward = false;
        bool visited_backward = false;
        std::optional<math::Vec2<uint16_t>> parent_forward;
        std::optional<math::Vec2<uint16_t>> parent_backward;
    };

    const uint16_t width = static_cast<uint16_t>(image.width());
    const uint16_t height = static_cast<uint16_t>(image.height());
    
    // Pre-allocate all required memory at once
    tmp::std_vector_t<tmp::std_vector_t<Node, Allocator>, Allocator> nodes;
    nodes.reserve(height);
    for(uint16_t i = 0; i < height; ++i) {
        nodes.emplace_back(width);
    }
    
    // Queues for forward and backward search
    std::queue<math::Vec2<uint16_t>> q_forward, q_backward;
    
    // Initialize start and goal nodes
    nodes[start_point.y][start_point.x].visited_forward = true;
    nodes[stop_point.y][stop_point.x].visited_backward = true;
    
    q_forward.push(start_point);
    q_backward.push(stop_point);

    // Direction vectors for 4-directional movement
    constexpr int8_t dx[4] = {0, 1, 0, -1};
    constexpr int8_t dy[4] = {-1, 0, 1, 0};

    math::Vec2<uint16_t> meeting_point{0, 0};
    bool path_found = false;
    
    while (!q_forward.empty() || !q_backward.empty()) {
        // Forward search step
        if (!q_forward.empty()) {
            math::Vec2<uint16_t> current_pos = q_forward.front();
            q_forward.pop();
            
            // Check if this node was also visited by backward search
            if (nodes[current_pos.y][current_pos.x].visited_backward) {
                meeting_point = current_pos;
                path_found = true;
                break;
            }

            // Explore neighbors
            for (size_t i = 0; i < 4; ++i) {
                math::Vec2<uint16_t> neighbor_pos{
                    static_cast<uint16_t>(current_pos.x + dx[i]),
                    static_cast<uint16_t>(current_pos.y + dy[i])
                };

                if (neighbor_pos.x >= width || neighbor_pos.y >= height) {
                    continue;
                }

                if (image(neighbor_pos.x, neighbor_pos.y) != 0 ||  
                    nodes[neighbor_pos.y][neighbor_pos.x].visited_forward) {
                    continue;
                }

                nodes[neighbor_pos.y][neighbor_pos.x].visited_forward = true;
                nodes[neighbor_pos.y][neighbor_pos.x].parent_forward = current_pos;
                
                q_forward.push(neighbor_pos);
            }
        }
        
        // Backward search step
        if (!q_backward.empty()) {
            math::Vec2<uint16_t> current_pos = q_backward.front();
            q_backward.pop();
            
            // Check if this node was also visited by forward search
            if (nodes[current_pos.y][current_pos.x].visited_forward) {
                meeting_point = current_pos;
                path_found = true;
                break;
            }

            // Explore neighbors
            for (size_t i = 0; i < 4; ++i) {
                math::Vec2<uint16_t> neighbor_pos{
                    static_cast<uint16_t>(current_pos.x + dx[i]),
                    static_cast<uint16_t>(current_pos.y + dy[i])
                };

                if (neighbor_pos.x >= width || neighbor_pos.y >= height) {
                    continue;
                }

                if (image(neighbor_pos.x, neighbor_pos.y) != 0 ||  
                    nodes[neighbor_pos.y][neighbor_pos.x].visited_backward) {
                    continue;
                }

                nodes[neighbor_pos.y][neighbor_pos.x].visited_backward = true;
                nodes[neighbor_pos.y][neighbor_pos.x].parent_backward = current_pos;
                
                q_backward.push(neighbor_pos);
            }
        }
    }

    if (!path_found) {
        // No path found
        tmp::std_vector_t<math::Vec2<uint16_t>, Allocator> empty_result;
        return empty_result;
    }

    // Reconstruct path
    tmp::std_vector_t<math::Vec2<uint16_t>, Allocator> path;
    
    // Build path from start to meeting point
    math::Vec2<uint16_t> pos = meeting_point;
    std::vector<math::Vec2<uint16_t>> path_part1;
    path_part1.push_back(pos);
    
    while (pos != start_point) {
        pos = nodes[pos.y][pos.x].parent_forward.value();
        path_part1.push_back(pos);
    }
    std::reverse(path_part1.begin(), path_part1.end());
    
    // Build path from meeting point to end
    pos = meeting_point;
    std::vector<math::Vec2<uint16_t>> path_part2;
    
    while (pos != stop_point) {
        pos = nodes[pos.y][pos.x].parent_backward.value();
        path_part2.push_back(pos);
    }
    
    // Combine both parts
    path.insert(path.end(), path_part1.begin(), path_part1.end());
    path.insert(path.end(), path_part2.begin() + 1, path_part2.end()); // Skip duplicate meeting point
    
    return path;
}


}