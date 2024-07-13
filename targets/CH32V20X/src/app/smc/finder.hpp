#ifndef __FINER_HPP__
#define __FINER_HPP__

#include "../../types/image/image.hpp"

#include "../nvcv2/flood_fill.hpp"
#include "../nvcv2/geometry/geometry.hpp"
#include "../nvcv2/pixels/pixels.hpp"
#include "../nvcv2/shape/shape.hpp"
#include "../nvcv2/template.hpp"

#include <vector>
#include <map>
#include <list>
#include <unordered_set>

using namespace NVCV2;

namespace SMC{
    using Coast = sstl::vector<Vector2i, 80>;
    using Boundary = std::map<int, int>;
    using Pile = std::pair<int, Rangei>;
    using Piles = std::map<int, Rangei>;
    using Point = Vector2i; 
    using Points = sstl::vector<Point, 64>;
    using Ranges = sstl::vector<Rangei, 64>;
    using NVCV2::Shape::Seed;
    using Segment = std::pair<Point, Point>;

    
    // enum TB{
    //     TOP = false,
    //     BOTTOM = true
    // };

    // enum LR{
    //     LEFT = false,
    //     RIGHT = true
    // };

    enum CornerType{
        ACORNER = -1,
        ALL = 0,
        VCORNER = 1
    };

    using Corner = std::pair<CornerType,const Vector2i &>;
    using Corners = sstl::vector<Corner, 8>;

    // namespace Boundary1 {
    //     using BoundaryX = std::map<int, int>;
    // }

    // namespace Boundary2 {
    //     using BoundaryY = std::map<int, int>;
    // }

    // using namespace::Boundary1;
    // using namespace::Boundary2;

    using Coasts = sstl::vector<Coast, 4>;

    Rangei get_h_range(const ImageReadable<Binary> & src, const Vector2i & pos){
        Rangei current_window = {0,0};
        int x = pos.x;
        while(x > 0){
            if(bool(src({x, pos.y})) == true){
                current_window.start = x;
                break;
            }
            x--;
        }

        x = pos.x;
        while(x < src.size.x - 1){
            if(bool(src({x, pos.y})) == true){
                current_window.end = x;
                break;
            }
            x++;
        }

        return current_window;
    }

    Rangei get_side_range(const ImageReadable<Binary> & src, const int & y, const int & minimal_length, const bool & is_right_align){

        sstl::vector<Rangei, 8> windows;
        Rangei current_window = {0,0};
        for(int x = 0; x < src.get_size().x - 2; x++){
            if(bool(src({x,y})) == true and bool(src({x + 1,y}) == false)){
                current_window = {x,x};
            }else if(bool(src({x,y})) == false and bool(src({x + 1,y}) == true)){
                current_window.end = x;
                if(minimal_length < current_window.length()){
                    windows.push_back(current_window);
                }
            }
        }

        switch(windows.size()){
            case 0:
                return {0, 0};
            case 1:
                return windows.front();
            default:
                std::sort(windows.begin(), windows.end(), 
                is_right_align ? [](const Rangei & p1, const Rangei & p2){return p1.get_center() > p2.get_center();} :
                [](const Rangei & p1, const Rangei & p2){return p1.get_center() < p2.get_center();}
                );

                return windows.front();
        }
    }

    Vector2i segment_get_diff(const Segment & segment){
        return Vector2i(segment.second.x - segment.first.x, segment.first.y - segment.second.y);//inverse y
    }
    // Pile get_h_pile(const ImageReadable<Binary> & src, const bool & from_bottom = true){
    //     return Pile{from_bottom ? src.get_size().y - 1 : 0, get_side_range(src, from_bottom)};
    // }

    bool is_pile_valid(const Pile & pile, const Rangei & valid_width){
        return pile.second.length() > valid_width.start || pile.second.length() < valid_width.end;
    }

    bool is_piles_valid(const Piles & piles, const Rangei & valid_width){
        for(const auto & pile : piles){
            if(is_pile_valid(pile, valid_width)) return false;
        }
        return true;
    }


    auto get_entry(const ImageReadable<Binary> & src, const Vector2i & last_seed_pos, const bool & m_right_align){
        auto y = last_seed_pos.y ? last_seed_pos.y : src.size.y - 1;
        static constexpr int bounce_x_backward = 4;
        static constexpr int road_minimal_length = 8;

        Rangei x_range;
        if(last_seed_pos.x == 0){//如果上次没有找到种子 这次就选取最靠近吸附的区域作为种子窗口
            x_range = get_side_range(src, y, road_minimal_length, m_right_align);
            if(x_range.length() < road_minimal_length){//如果最长的区域都小于路宽 那么就视为找不到种子
                return std::make_tuple(Vector2i{}, Rangei{});
            }
        }else{//如果上次有种子
            x_range = get_h_range(src,last_seed_pos);
            //在上次种子的基础上找新窗口
            if(x_range.length() < road_minimal_length){//如果最长的区域都小于路宽 那么就视为找不到种子
                return std::make_tuple(Vector2i{}, Rangei{});
            }
        }
        
        //如果上次找到种子 就在种子附近寻找窗口
        

        // DEBUG_VALUE(x_range);
        Vector2i ret_pos;
        if(x_range.length()){
            if(ret_pos == Vector2i{0,0}){
                return std::make_tuple(Vector2i(x_range.get_center(), y), x_range);
            }

            if(m_right_align){
                Vector2i new_seed_pos = Vector2i(x_range.end - bounce_x_backward,y);
                return std::make_tuple((last_seed_pos + new_seed_pos) / 2, x_range);
            }else{
                Vector2i new_seed_pos = Vector2i(x_range.start + bounce_x_backward, y);
                return std::make_tuple((last_seed_pos + new_seed_pos) / 2, x_range);
            }
        }

        // DEBUG_DEBUG_PRINT("efault sp");

        return std::make_tuple(Vector2i(), Rangei());
    }

    bool coast_self_insterction(const Coast & coast){
        // auto it = coast.begin();

        if(coast.size() < 2){
            return false;
        }

        auto first = coast.front();

        for(auto it = std::next(coast.begin()); it != coast.end(); ++it){
            if(*it == first) return true;
        }

        return false;
    }

    Vector2i coast_in_window(const Coast & coast, const Rect2i & window){
        if(coast.size() < 1){
            return Vector2i{0, 0};
        }

        for(const auto & point : coast){
            if(not window.has_point(point)) return point;
        }
        return Vector2i{0, 0};
    }

    auto coast_in_window(const Coast & coast, const Vector2i & window_size){
        return coast_in_window(coast, window_size.form_rect());
    }


    // Points coast_get_corners(const Coast & coast, const real_t & threshold, const bool & is_a){
    //     if(coast.size() < 3) return Points{};

    //     Points ret;
    //     for(auto it = std::next(coast.begin()); it != std::prev(coast.end()); it++){
    //         Vector2 lastpoint = *std::prev(it);
    //         Vector2 currpoint = *it;
    //         Vector2 nextpoint = *std::next(it);

    //         auto diff = currpoint - lastpoint;
    //         auto nextdiff = nextpoint - currpoint;
    //         auto value = diff.cos(nextdiff);

    //         if(is_a){
    //             if(diff.y < 0 and value < threshold){
    //                 ret.push_back(currpoint);
    //             }
    //         }else{
    //             if(diff.y > 0 and value < threshold){
    //                 ret.push_back(*it);
    //             }
    //         }
    //     }

    //     return ret;
    // }

    Corners coast_get_corners(const Coast & coast, const real_t & threshold, const CornerType & default_ct){
        // if(corner_type == CornerType::NONE) return Corners{};
        if(coast.size() < 3) return Corners{};

        Corners ret;
        for(auto it = std::next(coast.begin()); it != std::prev(coast.end()); it++){
            Vector2 lastpoint = *std::prev(it);
            Vector2 currpoint = *it;
            Vector2 nextpoint = *std::next(it);

            auto v1 = lastpoint - currpoint;
            auto v2 = nextpoint - currpoint;

            auto v1_l = v1.length();
            auto v2_l = v2.length();
            auto proj = v1.dot(v2) / v1_l / v2_l;
            auto med = ((v1 * v2_l) + (v2 * v1_l)).y; 

            if(proj > threshold){
                CornerType ct = sign(med) == 1 ? CornerType::ACORNER: CornerType::VCORNER;
                if(default_ct == CornerType::ALL || ct == default_ct) ret.push_back({ct, *it});
            }
        }

        return ret;
    }


    // Points coast_get_corners(const Coast & coast, const real_t & threshold, const bool & is_a){
    //     if(coast.size() < 3) return Points{};

    //     Points ret;
    //     for(auto it = std::next(coast.begin()); it != std::prev(coast.end()); it++){
    //         Vector2 lastpoint = *std::prev(it);
    //         Vector2 currpoint = *it;
    //         Vector2 nextpoint = *std::next(it);

    //         auto diff = currpoint - lastpoint;
    //         auto nextdiff = nextpoint - currpoint;
    //         auto value = diff.cos(nextdiff);

    //         if(is_a){
    //             if(diff.y < 0 and value < threshold){
    //                 ret.push_back(currpoint);
    //             }
    //         }else{
    //             if(diff.y > 0 and value < threshold){
    //                 ret.push_back(*it);
    //             }
    //         }
    //     }

    //     return ret;
    // }

    Points coast_get_a_points(const Coast & coast, const real_t & threshold = real_t(-0.2)){
        auto corners = coast_get_corners(coast, threshold, CornerType::ACORNER);
        Points ret;

        for(auto & [_, point]:corners){
            ret.push_back(point);
        }
        
        return ret;
    }

    Points coast_get_v_points(const Coast & coast, const real_t & threshold = real_t(-0.2)){
        auto corners = coast_get_corners(coast, threshold, CornerType::VCORNER);
        Points ret;

        for(auto & [_, point]:corners){
            ret.push_back(point);
        }
        
        return ret;
    }

    Coast fix_coast(const Coast & coast, const Vector2i & window_size){
        if(!coast.size()) return {{}};

        auto first = coast.front();
        auto window = window_size.form_rect();
        if(!window.has_point(first)){
            DEBUG_LOG("first not in window", first);
            return{{}};
        }

        Coast ret;

        for(auto it = std::next(coast.begin()); it != coast.end(); ++it){
            if(not window.has_point(*it)) break;
            if(first == *it) break;
            ret.push_back(*it);
        }
        // DEBUG_TRAP("fix end", ret.size(), ';');
        return ret;
    }

    Rect2i coast_to_rect(const Coast & coast){
        // auto it = coast.begin();
        auto first = coast.front();
        Rect2i ret = {first, Vector2i{}};

        for(auto it = std::next(coast.begin()); it != coast.end(); ++it){
            ret = ret.merge(*it);
        }

        return ret;
    }

    Piles coast_to_ypiles(const Coast & coast){
        Piles ret;
        for(const auto & point : coast){
            if(ret.count(point.x) == 0){
                ret[point.x] = Rangei(point.y, point.y);
            } else {
                ret[point.x].merge(point.y);
            }
        }

        return ret;
    }

    Piles coast_to_xpiles(const Coast & coast){
        Piles ret;
        for(const auto & point : coast){
            if(ret.count(point.y) == 0){
                ret[point.y] = Rangei(point.x, point.x);
            } else {
                ret[point.y].merge(point.x);
            }
        }

        return ret;
    }

    // get_outroad_y_range(const bou)


    Coast get_coast(const ImageReadable<Binary> & src, Vector2i seed_pos, const bool & is_right){
        // CoastFinder finder_dr(src, Seed(seed_pos, is_right ? Direction::R : Direction::L, true));
        // auto ret = finder_dr.find();
        using namespace NVCV2::Shape;

        seed_pos = src.get_window().constrain(Vector2i{seed_pos.x, seed_pos.y});
        Seed seed{seed_pos,  is_right ? Direction::R : Direction::L, !is_right};
        CoastFinder finder{src, seed};
        auto ret = finder.find();
        // if(coast_self_insterction(ret)){
        //     auto rect = coast_to_rect(ret);
        //     if(rect.get_area() < 60){
        //         Seed seed2 {src.get_window().constrain(Vector2i{is_right ? rect.get_x_range().end + 2 : rect.get_x_range().start - 2, seed_pos.y}),  is_right ? Direction::R : Direction::L, !is_right};
        //         CoastFinder finder2 {src, seed2};
        //         DEBUG_PRINT("recoast done");
        //         ret = finder2.find();
        //     }
        // }
    
        // Coast ret;
        // if(is_right) ret.reverse();

        return ret;
    
        // for(auto it = boud_dr.rbegin(); it != boud_dr.rend(); ++it) {
        //     ret.push_back(*it);
        // }

        // for(const auto & item : boud_ur){
        //     ret.push_back(item);
        // }

        // if(is_right) ret.reverse();

        // return ret;
    }

    // Boundary get_coast(const ImageReadable<Grayscale> & src, Vector2i seed_pos, const bool & is_right){
    Piles get_x_piles(const ImageReadable<Binary> & src, Seed seed){

        // DEBUG_PRINT("x_piles", Vector2i(seed));
        Piles ret;
        while(src.has_point(seed) and bool(src[Vector2i(seed)]) == false){
            seed.forward();
            ret[Vector2i(seed).y] = get_h_range(src, Vector2i(seed));
        }

        return ret;
    }


    // Ranges get_outroad_y_ranges(const ImageReadable<Binary> & src, const Rangei & max_road, Seed seed){
    //     Piles x_piles = get_x_piles(src, seed);
    //     // DEBUG_VALUE(x_piles.size());
    //     if(x_piles.size() == 0) return {};

    //     std::vector<int> outroad_y;
    //     // DEBUG_PRINT("xp_size", x_piles.size());
    //     for(auto [y, x_range] : x_piles){
    //         if(!x_range.inside(max_road)) outroad_y.push_back(y);
    //     }

    //     if(!outroad_y.size()){
    //         return {};
    //     }

    //     Ranges ret;
    //     Rangei y_range;
    //     y_range.start = outroad_y.front();
    //     for(auto it = outroad_y.begin();it != std::prev(outroad_y.end()); it++){
    //         if((*std::next(it)) - 1 != *it){
    //             y_range.end = *it;
    //             ret.push_back(y_range);
    //             y_range.start = *std::next(it);
    //         }
    //     }

    //     return ret;
    // }

    // Boundary form_h_boundary(const Coast & coast, const bool & right_side) {
    //     std::map<int, int> x_map;
    //     for(const auto & point : coast) {
    //         if(x_map.count(point.y) == 0) {
    //             x_map[point.y] = point.x;
    //         } else {
    //             x_map[point.y] = right_side ? MAX(x_map[point.y], point.x) : MIN(x_map[point.y], point.x);
    //         }
    //     }

    //     // Copying map's contents to a vector
    //     std::vector<std::pair<int, int>> vec(x_map.begin(), x_map.end());

    //     // Sorting the vector based on keys (y-values) in descending order
    //     std::sort(vec.begin(), vec.end(), [](const auto & a, const auto & b) { return a.first > b.first; });

    //     // Converting sorted vector back to a map
    //     std::map<int, int> sorted_map(vec.begin(), vec.end());

    //     return sorted_map;
    // }

    // Boundary right_boundary(const Coast & coast){
    //     return form_h_boundary(coast, false);
    // }

    // Boundary left_boundary(const Coast & coast){
    //     return form_h_boundary(coast, true);
    // }

    Boundary mean_boundry(const Boundary & left, const Boundary & right){
        std::map<int, int> result;
        std::unordered_map<int, int> hash_map;

        for (const auto& pair : left) {
            hash_map[pair.first] = pair.second;
        }

        for (const auto& pair : right) {
            if (hash_map.find(pair.first) != hash_map.end()) {
                int average = (pair.second + hash_map[pair.first]) / 2;
                result[pair.first] = average;
            }
        }

        return result;

    }

    Coast bound_to_coast(const Boundary & bound){
        Coast coast;
        for(auto [y, x]:bound){
            if(y != 0) coast.push_back(Vector2i(x,y));
        }
        return coast;
    }

    Coasts split_to_coasts(const Coast & line, const real_t & break_angle){
        const real_t MIN_shadow = cos(break_angle);
        Coasts coasts;
        Coast coast = {};

        for(auto it = std::next(line.begin()); it != std::prev(line.end()); ++it){
            auto & lastpoint = *std::prev(it); 
            auto & currpoint = *it;
            auto & nextpoint = *std::next(it);

            auto diff = nextpoint - currpoint;
            auto last_diff = currpoint - lastpoint;
            real_t cos_ang = diff.dot(last_diff) / (diff.length() * last_diff.length());

            if(cos_ang > MIN_shadow){
                coast.push_back(currpoint);
            }else{
                coasts.push_back(coast);
                coast = {};
            }
        }

        coasts.push_back(coast);
        return coasts;
    }

    Coast get_main_coast(const Coast & line, const real_t & break_angle = real_t(0.5)){
        std::map<int, int> length_map;

        auto coasts = split_to_coasts(line, break_angle);
        for(auto it = coasts.begin(); it != coasts.end(); ++it){
            length_map[std::distance(coasts.begin(), it)] = it->size();
        }

        auto MAX_index = 0;
        auto MAX_size = 0;
        for(auto [index, size] : length_map){
            if(size > MAX_size){
                MAX_size = size;
                MAX_index = index;
            }
        }

        // return coasts[0];
        return coasts[MAX_index];
    }



// typedef std::pair<real_t, real_t> Point;

real_t PerpendicularDistance(const Point& pt, const Point& lineStart, const Point& lineEnd)
{
	real_t dx = lineEnd.x - lineStart.x;
	real_t dy = lineEnd.y - lineStart.y;

	//Normalise
	// real_t mag = sqrt(pow(dx, 2) + pow(dy, 2));
    real_t mag = lineEnd.dist_to(lineStart);
	if (mag > 0.0)
	{
		dx /= mag;
		dy /= mag;
	}

	real_t pvx = pt.x - lineStart.x;
	real_t pvy = pt.y - lineStart.y;

	//Get dot product (project pv onto normalized direction)
	real_t pvdot = dx * pvx + dy * pvy;

	//Scale line direction vector
	real_t dsx = pvdot * dx;
	real_t dsy = pvdot * dy;

	//Subtract this from pv
	real_t ax = pvx - dsx;
	real_t ay = pvy - dsy;

	// return sqrt(pow(ax, 2.0) + pow(ay, 2.0));
    return Vector2(ax, ay).length();
}



Points douglas_peucker_vector(const Points& polyLine, const real_t & epsilon){
    // Points& _polyLine
    Points simplifiedPolyLine = {};
	if (polyLine.size() < 2)
	{
		return polyLine;
	}

	// Find the point with the MAXimum distance from line between start and end
	real_t MAXDistance = 0.0;
	int index = 0;
	int end = polyLine.size() - 1;
	for (int i = 1; i < end; i++)
	{
		real_t d = PerpendicularDistance(polyLine[i], polyLine[0], polyLine[end]);
		if (d > MAXDistance)
		{
			index = i;
			MAXDistance = d;
		}
	}

	// If MAX distance is greater than epsilon, recursively simplify
	if (MAXDistance > epsilon)
	{
		// Recursive call
		Points recResults1;
		Points recResults2;
		Points firstLine(polyLine.begin(), polyLine.begin() + index + 1);
		Points lastLine(polyLine.begin() + index, polyLine.end());
		recResults1 = douglas_peucker_vector(firstLine, epsilon);
		recResults2 = douglas_peucker_vector(lastLine, epsilon);

		// Build the result list
		simplifiedPolyLine.assign(recResults1.begin(), recResults1.end() - 1);
		simplifiedPolyLine.insert(simplifiedPolyLine.end(), recResults2.begin(), recResults2.end());

	}
	else
	{
		//Just return start and end points
		simplifiedPolyLine.push_back(polyLine[0]);
		simplifiedPolyLine.push_back(polyLine[end]);
	}

    return simplifiedPolyLine;
}


    Coast douglas_peucker(const Coast & line, const real_t & epsilon) {
        if(line.size() == 0){
            DEBUG_WARN("dp input size 0");
        }
        auto ret = douglas_peucker_vector(Points(line.begin(), line.end()), epsilon);
        if(ret.size() == 0){
            DEBUG_WARN("dp output size 0");
        }else if(ret.size() == 1){
            if(line.front() == ret.front()) ret.push_back(line.back());
            if(line.back() == ret.front()) ret.push_back(line.front());
        }

        return Coast(ret.begin(), ret.end());
    }


    
    //排斥实验
    bool IsRectCross(const Point &p1,const Point &p2,const Point &q1,const Point &q2)
    {
        bool ret = MIN(p1.x,p2.x) <= MAX(q1.x,q2.x)    &&
                    MIN(q1.x,q2.x) <= MAX(p1.x,p2.x) &&
                    MIN(p1.y,p2.y) <= MAX(q1.y,q2.y) &&
                    MIN(q1.y,q2.y) <= MAX(p1.y,p2.y);
        return ret;
    }
 
 
    //跨立判断
    bool is_line_segment_cross(const Point &P1,const Point &P2,const Point & R1,const Point & R2){
        if(
            ((R1.x-P1.x)*(R1.y-R2.y)-(R1.y-P1.y)*( R1.x-R2.x)) * ((R1.x-P2.x)*(R1.y-R2.y)-(R1.y-P2.y)*(R1.x-R2.x)) < 0 ||
            ((P1.x-R1.x)*(P1.y-P2.y)-(P1.y-R1.y)*(P1.x-P2.x)) * ((P1.x-R2.x)*(P1.y-P2.y)-(P1.y-R2.y)*( P1.x-P2.x)) < 0
        ) 
            return true;
        else
        return false;
    }
 

    bool get_cross_point(const Point &p1,const Point &p2,const Point &q1,const Point &q2,Point & out)
    {
        if(IsRectCross(p1,p2,q1,q2))
        {
            if (is_line_segment_cross(p1,p2,q1,q2))
            {
                //求交点
                real_t tmpLeft,tmpRight;
                tmpLeft = (q2.x - q1.x) * (p1.y - p2.y) - (p2.x - p1.x) * (q1.y - q2.y);
                tmpRight = (p1.y - q1.y) * (p2.x - p1.x) * (q2.x - q1.x) + q1.x * (q2.y - q1.y) * (p2.x - p1.x) - p1.x * (p2.y - p1.y) * (q2.x - q1.x);
    
                out.x = int(((real_t)tmpRight/(real_t)tmpLeft));
    
                tmpLeft = (p1.x - p2.x) * (q2.y - q1.y) - (p2.y - p1.y) * (q1.x - q2.x);
                tmpRight = p2.y * (p1.x - p2.x) * (q2.y - q1.y) + (q2.x- p2.x) * (q2.y - q1.y) * (p1.y - p2.y) - q2.y * (q1.x - q2.x) * (p2.y - p1.y); 
                out.y = int(((real_t)tmpRight/(real_t)tmpLeft));
                return true;
            }
        }
        return false;
    }

    void constrain_segment_in_rect(const Rect2i & rect, Vector2i & p1, Vector2i & p2){
        bool has_p1 = rect.has_point(p1);
        bool has_p2 = rect.has_point(p2);
        Point intersection_point;
        Point rect_p1(rect.position.x, rect.position.y);
        Point rect_p2(rect.position.x + rect.size.x, rect.position.y);
        Point rect_p3(rect.position.x + rect.size.x, rect.position.y + rect.size.y);
        Point rect_p4(rect.position.x, rect.position.y + rect.size.y);

        if(has_p1 && has_p2) return;
        if(has_p1 ^ has_p2){
            auto & dst_point = has_p1 ? p2 : p1;
            Point p1v = p1;
            Point p2v = p2;
            bool intersects = get_cross_point(p1v, p2v, rect_p1, rect_p2, intersection_point);

            if (intersects) {
                dst_point = intersection_point;
            } else {
                intersects = get_cross_point(p1v, p2v, rect_p2, rect_p3, intersection_point);
                if (intersects) {
                    dst_point = intersection_point;
                } else {
                    intersects = get_cross_point(p1v, p2v, rect_p3, rect_p4, intersection_point);
                    if (intersects) {
                        dst_point = intersection_point;
                    } else {
                        intersects = get_cross_point(p1v, p2v, rect_p1, rect_p4, intersection_point);
                        if (intersects) {
                            dst_point = intersection_point;
                        }
                    }
                }
            }
        }else{
            Points points;
            Point p2v = p2;
            Point p1v = p1;
            bool intersects = get_cross_point(p1v, p2v, rect_p1, rect_p2, intersection_point);
            if (intersects) {
                points.push_back(intersection_point);
            } else {
                intersects = get_cross_point(p1v, p2v, rect_p2, rect_p3, intersection_point);
                if (intersects) {
                    points.push_back(intersection_point);
                } else {
                    intersects = get_cross_point(p1v, p2v, rect_p3, rect_p4, intersection_point);
                    if (intersects) {
                        points.push_back(intersection_point);
                    } else {
                        intersects = get_cross_point(p1v, p2v, rect_p1, rect_p4, intersection_point);
                        if (intersects) {
                            points.push_back(intersection_point);
                        }
                    }
                }
            }

            if(points.size() > 2) {
                DEBUG_LOG("to much points", points.size());
            }
            p1v = points.front();
            p2v = points.back();

            if((p1v - (Point)p1).length_squared() > (p2v - (Point)p1).length_squared()){
                std::swap(p1v, p2v);
            }

            p1 = p1v;
            p2 = p2v;
        }
    }

    Coast resolve_coast_in_window(const Rect2i & rect, const Coast & coast){
    
        if(coast.size() == 2){
            Coast ret = coast;
            constrain_segment_in_rect(rect, ret.front(), ret.back());
            return ret;
        }

        Coast ret = coast;

        bool entered_window = false;
        // bool exited_window = false;
        auto it = coast.begin();

        while(it != std::prev(coast.end())){
            auto & point = *it;
            auto & nextpoint = *std::next(it);

            if(!entered_window){
                entered_window = rect.has_point(point);
            }

            bool next_entered_window = rect.has_point(nextpoint);
            bool next_exited_window = !rect.has_point(nextpoint);

            if(!entered_window && next_entered_window){
                auto temp_point = point;
                auto temp_nextpoint = nextpoint;
                constrain_segment_in_rect(rect, temp_point, temp_nextpoint);
                ret.push_back(temp_point);
            }

            ret.push_back(point);
            
            if(next_exited_window){
                auto temp_point = point;
                auto temp_nextpoint = nextpoint;

                constrain_segment_in_rect(rect, temp_point, temp_nextpoint);
                ret.push_back(temp_nextpoint);
                break;
            }


            it++;
        }

        return ret;
    }

    Coast shrink_coast(const Coast & line, const real_t & width, const Vector2i & window_size){
        if(line.size() == 1){
            DEBUG_LOG("only one point");
        }else if(line.size() == 2){
            Coast ret;
            Rect2i window = Rect2i(Vector2i(), window_size);

            auto diff = line.front() - line.back();
            auto offset = Vector2i(diff.y, -diff.x).normalized() * width;


            ret.push_back(line.front() + offset);
            ret.push_back(line.back() + offset);
            constrain_segment_in_rect(window, ret.front(), ret.back());
            return ret;
        }
        Coast ret;
        auto it = line.begin();
        while(it != line.end()){
            auto & currpoint = *it;

            Vector2 diff;
            if(it == line.begin()){
                auto & nextpoint = *std::next(it);
                diff = nextpoint - currpoint;
            }else if(it == line.end()){
                auto & lastpoint = *std::prev(it);
                diff = currpoint - lastpoint;
            }else{
                auto & lastpoint = *std::prev(it);
                auto & nextpoint = *std::next(it);
                diff = nextpoint - lastpoint;
            } 
            Vector2i new_point = Vector2{diff.y, -diff.x}.normalized() * width + currpoint;
            
            // {
                // Rect2i window = Rect2i(Vector2i(), window_size);
                // bool addable = window.has_point(new_point);
                // static constexpr real_t toler = 4;

                // const real_t abs_w = ABS(width);
                // const int MIN_ls = int((abs_w - toler) * (abs_w - toler));  
                
                // for(auto & item : line){
                //     if(!addable){
                //         break;
                //     }

                    // if((new_point - item).length_squared() < MIN_ls){
                        // addable = false;
                    // }
                // }

                // if(addable){
                ret.push_back(new_point);
                // }
            // }

            ++it;
        }
        Rect2i window = Rect2i(Vector2i(), window_size);

        resolve_coast_in_window(window, ret);
        return ret;
    }




    // bool is_coast_self_intersecting(const Coast & coastline) {
    //     std::unordered_set<std::pair<int, int>> pointSet = {};

    //     for (const auto& point : coastline) {
    //         std::pair<int, int> pointPair = {point.x, point.y};
    //         if (pointSet.find(pointPair) != pointSet.end()) {
    //             return true;
    //         }

    //         pointSet.insert(pointPair);
    //     }
    //     return false;
    // }
};

#endif