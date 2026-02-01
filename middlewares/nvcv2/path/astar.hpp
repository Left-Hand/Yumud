#pragma once

#include "middlewares/nvcv2/nvcv2.hpp"
#include "core/tmp/std_container.hpp"
#include <queue>


namespace ymd::nvcv2{



//A star 寻路算法 图片中0为空 非0为障碍

template<template<typename> typename Allocator>
std::vector<math::Vec2<uint16_t>, Allocator<math::Vec2<uint16_t>>> astar(
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
        int32_t g = std::numeric_limits<int32_t>::max();
        int32_t h = 0;
        bool in_open_set = false;
        bool in_closed_set = false;
        std::optional<math::Vec2<uint16_t>> parent;
        
        inline int32_t f() const { return g + h; }
    };

    const uint16_t width = static_cast<uint16_t>(image.width());
    const uint16_t height = static_cast<uint16_t>(image.height());
    
    // Pre-allocate all required memory at once
    tmp::std_vector_t<tmp::std_vector_t<Node, Allocator>, Allocator> nodes;
    nodes.reserve(height);
    for(uint16_t i = 0; i < height; ++i) {
        nodes.emplace_back(width);
    }
    
    // Heuristic function: Manhattan distance
    auto heuristic = [](const math::Vec2<uint16_t>& a, const math::Vec2<uint16_t>& b) -> int32_t {
        int32_t dx = std::abs(static_cast<int32_t>(a.x) - static_cast<int32_t>(b.x));
        int32_t dy = std::abs(static_cast<int32_t>(a.y) - static_cast<int32_t>(b.y));
        return dx + dy;
    };

    // Initialize start node
    nodes[start_point.y][start_point.x].g = 0;
    nodes[start_point.y][start_point.x].h = heuristic(start_point, stop_point);
    nodes[start_point.y][start_point.x].in_open_set = true;

    // Custom priority queue that uses pre-allocated storage
    // We'll use a vector-based implementation instead of std::priority_queue
    std::vector<math::Vec2<uint16_t>, Allocator<math::Vec2<uint16_t>>> open_list_vec;
    open_list_vec.reserve(width * height / 4); // Estimate initial capacity
    open_list_vec.push_back(start_point);

    // Helper function to maintain heap property
    auto heapify_up = [&](size_t index) {
        while(index > 0) {
            size_t parent_idx = (index - 1) / 2;
            if(nodes[open_list_vec[parent_idx].y][open_list_vec[parent_idx].x].f() <= 
               nodes[open_list_vec[index].y][open_list_vec[index].x].f()) {
                break;
            }
            std::swap(open_list_vec[parent_idx], open_list_vec[index]);
            index = parent_idx;
        }
    };
    
    auto heapify_down = [&](size_t index) {
        size_t size = open_list_vec.size();
        while(true) {
            size_t smallest = index;
            size_t left_child = 2 * index + 1;
            size_t right_child = 2 * index + 2;
            
            if(left_child < size && 
               nodes[open_list_vec[left_child].y][open_list_vec[left_child].x].f() < 
               nodes[open_list_vec[smallest].y][open_list_vec[smallest].x].f()) {
                smallest = left_child;
            }
            
            if(right_child < size && 
               nodes[open_list_vec[right_child].y][open_list_vec[right_child].x].f() < 
               nodes[open_list_vec[smallest].y][open_list_vec[smallest].x].f()) {
                smallest = right_child;
            }
            
            if(smallest == index) break;
            
            std::swap(open_list_vec[smallest], open_list_vec[index]);
            index = smallest;
        }
    };
    
    auto push_to_heap = [&](const math::Vec2<uint16_t>& pos) {
        open_list_vec.push_back(pos);
        heapify_up(open_list_vec.size() - 1);
    };
    
    auto pop_from_heap = [&]() {
        if(open_list_vec.empty()) {
            return math::Vec2<uint16_t>{0, 0}; // Should not happen in normal flow
        }
        std::swap(open_list_vec[0], open_list_vec.back());
        auto result = open_list_vec.back();
        open_list_vec.pop_back();
        if(!open_list_vec.empty()) {
            heapify_down(0);
        }
        return result;
    };

    // Direction vectors for 4-directional movement (up, right, down, left)
    constexpr int8_t dx[4] = {0, 1, 0, -1};
    constexpr int8_t dy[4] = {-1, 0, 1, 0};

    while (!open_list_vec.empty()) {
        math::Vec2<uint16_t> current_pos = pop_from_heap();
        
        // Early exit if this node was already processed with a better path
        if(nodes[current_pos.y][current_pos.x].in_closed_set) {
            continue;
        }
        
        Node& current_node = nodes[current_pos.y][current_pos.x];
        current_node.in_open_set = false;
        current_node.in_closed_set = true;

        // Check if we reached the goal
        if (current_pos == stop_point) {
            // Reconstruct path - reserve space to minimize allocations
            tmp::std_vector_t<math::Vec2<uint16_t>, Allocator> path;
            size_t estimated_path_length = heuristic(start_point, stop_point);
            path.reserve(estimated_path_length); // Conservative estimate
            
            math::Vec2<uint16_t> pos = stop_point;
            path.push_back(pos);
            
            while (pos != start_point) {
                pos = nodes[pos.y][pos.x].parent.value();
                path.push_back(pos);
            }
            
            std::reverse(path.begin(), path.end());
            return path;
        }

        // Explore neighbors
        for (size_t i = 0; i < 4; ++i) {
            math::Vec2<uint16_t> neighbor_pos{
                static_cast<uint16_t>(current_pos.x + dx[i]),
                static_cast<uint16_t>(current_pos.y + dy[i])
            };

            // Check bounds
            if (neighbor_pos.x >= width || neighbor_pos.y >= height) {
                continue;
            }

            // Skip if it's an obstacle or already in closed set
            if (image(neighbor_pos.x, neighbor_pos.y) != 0 ||  
                nodes[neighbor_pos.y][neighbor_pos.x].in_closed_set) {
                continue;
            }

            Node& neighbor_node = nodes[neighbor_pos.y][neighbor_pos.x];
            
            int32_t tentative_g = current_node.g + 1;

            if (tentative_g < neighbor_node.g) {
                neighbor_node.g = tentative_g;
                if(neighbor_node.h == 0) {
                    neighbor_node.h = heuristic(neighbor_pos, stop_point);
                }
                neighbor_node.parent = current_pos;

                if (!neighbor_node.in_open_set) {
                    neighbor_node.in_open_set = true;
                    push_to_heap(neighbor_pos);
                } else {
                    // If node is already in open set, re-heapify to maintain heap property
                    for(size_t idx = 0; idx < open_list_vec.size(); ++idx) {
                        if(open_list_vec[idx] == neighbor_pos) {
                            heapify_up(idx);
                            break;
                        }
                    }
                }
            }
        }
    }

    // No path found
    tmp::std_vector_t<math::Vec2<uint16_t>, Allocator> empty_result;
    return empty_result;
}





}