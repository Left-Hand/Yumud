#include "finder.hpp"


using namespace NVCV2;

namespace SMC{


    Rangei get_h_range(const ImageReadable<Binary> & src, const Vector2i & pos){
        auto size = src.get_size();
        Rangei current_window = {0,size.x - 1};

        for(auto x = pos.x; x > 0; x--){
            if(bool(src({x, pos.y})) == true){
                current_window.start = x;
                break;
            }
        }

        for(auto x = pos.x; x < size.x-1; x++){
            if(bool(src({x, pos.y})) == true){
                current_window.end = x;
                break;
            }
        }

        return current_window;
    }

    Rangei get_side_range(const ImageReadable<Binary> & src, const int y, const int minimal_length, const AlignMode align_mode){

        sstl::vector<Rangei, 8> windows;
        Rangei current_window = {0,0};

        enum class SearchStatus{
            SEEKING,
            TRAVE,
        }status = SearchStatus::SEEKING;
    
        auto size = src.get_size();
        for(int x = 0; x < size.x - 1; x++){

            switch(status){

            case SearchStatus::SEEKING:
                if(bool(src({x,y})) == true and bool(src({x + 1,y}) == false)){
                    current_window.from = x;
                    status = SearchStatus::TRAVE;
                }
                break;

            case SearchStatus::TRAVE:
                if(bool(src({x,y})) == false and bool(src({x + 1,y}) == true)){
                    current_window.to = x;
                    if(minimal_length < current_window.length()){
                        windows.push_back(current_window);
                    }
                    status = SearchStatus::SEEKING;
                }
                break;
            }
        }

        switch(windows.size()){
            case 0:
                return {0, 0};
            case 1:
                return windows.front();
            default:

                switch(align_mode){

                    case AlignMode::RIGHT:
                        std::sort(windows.begin(), windows.end(), [](const Rangei & p1, const Rangei & p2){return p1.get_center() > p2.get_center();});
                        break;
                    case AlignMode::LEFT:
                        std::sort(windows.begin(), windows.end(), [](const Rangei & p1, const Rangei & p2){return p1.get_center() < p2.get_center();});
                        break;
                    case AlignMode::BOTH:
                        std::sort(windows.begin(), windows.end(), [&size](const Rangei & p1, const Rangei & p2){
                            auto center_cmp = [](const int _p1, const int _p2, const int center) -> bool{
                                return std::abs(_p1 - center) < std::abs(_p2 - center);
                            };

                            auto p1_center = p1.get_center();
                            auto p2_center = p2.get_center();
                            return center_cmp(p1_center, p2_center, size.x/2);
                        });
                        break;
                    default:
                        break;
                }

                return windows.front();
        }
    }

    Vector2i SegmentUtils::vec(const Segment & segment){
        return Vector2i(segment.second.x - segment.first.x, segment.first.y - segment.second.y);//inverse y
    }


    Segment SegmentUtils::shift(const Segment & seg, const Point & offs){
        return {seg.first + offs, seg.second + offs};
    }

    bool PileUtils::invalidity(const Pile & pile, const Rangei & valid_width){
        return pile.second.length() < valid_width.start || pile.second.length() > valid_width.end;
    }

    bool PileUtils::invalidity(const Piles & piles, const Rangei & valid_width){
        for(const auto & pile : piles){
            if(PileUtils::invalidity(pile, valid_width)) return true;
        }
        return false;
    }

    bool CoastUtils::is_self_intersection(const Coast & coast){
        if(coast.size() < 2){
            return false;
        }

        const auto & first = coast.front();

        for(auto it = std::next(coast.begin()); it != coast.end(); ++it){
            if(*it == first) return true;
        }

        return false;
    }

    Vector2i CoastUtils::which_in_window(const Coast & coast, const Rect2i & window){
        if(coast.size() < 1){
            return Vector2i{0, 0};
        }

        for(const auto & point : coast){
            if(not window.has_point(point)) return point;
        }
        return Vector2i{0, 0};
    }

    Point CoastUtils::which_in_window(const Coast & coast, const Vector2i & window_size){
        return CoastUtils::which_in_window(coast, window_size.form_rect());
    }

    Corners CoastUtils::search_corners(const Coast & coast, const CornerType exp_ct, const real_t threshold){
        if(coast.size() < 3) return {};

        Corners ret;
        for(auto it = std::next(coast.begin()); it != std::prev(coast.end()); it++){
            Vector2 lastpoint = *std::prev(it);
            Vector2 currpoint = *it;
            Vector2 nextpoint = *std::next(it);

            auto v1 = lastpoint - currpoint;
            auto v2 = nextpoint - currpoint;

            auto v1_l = v1.length();
            auto v2_l = v2.length();
            auto med = ((v1 * v2_l) + (v2 * v1_l)).y; 

            if(v1.dot(v2) > threshold * v1_l * v2_l){

                switch(int(sign(med))){
                    case 0:
                        ret.push_back(Corner(CornerType::ALL, (*it)));
                        break;
                    case 1:
                        if(exp_ct == CornerType::AC || exp_ct == CornerType::ALL) ret.push_back(Corner{CornerType::AC, (*it)});
                        break;
                    case -1:
                        if(exp_ct == CornerType::VC || exp_ct == CornerType::ALL) ret.push_back(Corner{CornerType::VC, (*it)});
                        break;
                    default:
                        break;
                }
            }
        }

        return ret;
    }

    Points CoastUtils::a_points(const Coast & coast, const real_t threshold){
        auto corners = CoastUtils::search_corners(coast, CornerType::AC, threshold);
        Points ret;

        for(const auto & [_, point, __]:corners){
            ret.push_back(point);
        }
        
        return ret;
    }

    Points CoastUtils::v_points(const Coast & coast, const real_t threshold){
        auto corners = CoastUtils::search_corners(coast, CornerType::VC, threshold);
        Points ret;

        for(const auto & [_, point, __]:corners){
            ret.push_back(point);
        }
        
        return ret;
    }

    Coast CoastUtils::trim(const Coast & coast, const Vector2i & window_size){
        if(!coast.size()) return {{}};

        const auto & first = coast.front();
        auto window = window_size.form_rect();
        if(!window.has_point(first)){
            DEBUG_LOG("first not in window", first);
            return{{}};
        }

        Coast ret;

        for(auto it = std::next(coast.begin()); it != coast.end(); ++it){
            const auto & point = *it;
            if(point == first){
                //self ins
                break;
            }else if(not window.has_point(point)){
                //out of bound exit
                break;
            }else{
                ret.push_back(point);
            }

        }
        return ret;
    }

    Rect2i CoastUtils::bounding_box(const Coast & coast){
        if(coast.size() == 0){
            DEBUG_WARN("coast is empty");
            return Rect2i{};
        }

        Rect2i ret = {Vector2i(coast.front()), Vector2i{}};

        if(coast.size() < 2){
            return ret;
        }

        for(auto it = std::next(coast.begin()); it != coast.end(); ++it){
            ret = ret.merge(*it);
        }

        return ret;
    }

    Piles CoastUtils::ypiles(const Coast & coast){
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

    Piles CoastUtils::xpiles(const Coast & coast){
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


    Coast CoastUtils::form(const ImageReadable<Binary> & src, const Vector2i & _seed_pos, const LR is_right){

        using namespace NVCV2::Shape;

        auto seed_pos = src.get_window().constrain(_seed_pos);
        Seed seed{seed_pos,  is_right ? Direction::R : Direction::L, !is_right};
        CoastFinder finder{src, seed};
        auto ret = finder.find();


        return ret;
    

    }

    // Boundry CoastUtils::form(const ImageReadable<Grayscale> & src, Vector2i seed_pos, const bool & is_right){
    Piles get_x_piles(const ImageReadable<Binary> & src, const Point & seed_pos){

        Piles ret;
        Seed seed{seed_pos};
        while(src.has_point(seed_pos) and bool(src[Vector2i(seed_pos)]) == false){
            seed.forward();
            ret[Vector2i(seed).y] = get_h_range(src, Vector2i(seed));
        }

        return ret;
    }


    Boundry BoundryUtils::mean(const Boundry & left, const Boundry & right){
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

    Coast BoundryUtils::to_coast(const Boundry & bound){
        Coast coast;
        for(auto [y, x]:bound){
            if(y != 0) coast.push_back(Vector2i(x,y));
        }
        return coast;
    }

    Coasts split_to_coasts(const Coast & line, const real_t break_angle){
        const real_t MIN_shadow = cos(break_angle);
        Coasts coasts;
        Coast coast = {};

        if(line.size() < 3){
            return {line};
        }

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

    Coast get_main_coast(const Coast & line, const real_t break_angle){
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



Points douglas_peucker_vector(const Points& polyLine, const real_t epsilon){
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


    Coast CoastUtils::douglas_peucker(const Coast & line, const real_t epsilon) {
        if(line.size() == 0){
            DEBUG_WARN("dp input size 0");
            return {};
        }
        auto ret = douglas_peucker_vector(Points(line.begin(), line.end()), epsilon);

        if(ret.size() == 0){
            DEBUG_WARN("dp output size 0");
            return {};
        }else if(ret.size() == 1){
            //如果只有一个点 那么添加首尾
            if(line.front() == ret.front()) ret.push_back(line.back());
            if(line.back() == ret.front()) ret.push_back(line.front());
        }

        return Coast(ret.begin(), ret.end());
    }

    static bool IsRectCross(const Point &p1,const Point &p2,const Point &q1,const Point &q2)
    {
        bool ret = MIN(p1.x,p2.x) <= MAX(q1.x,q2.x)    &&
                    MIN(q1.x,q2.x) <= MAX(p1.x,p2.x) &&
                    MIN(p1.y,p2.y) <= MAX(q1.y,q2.y) &&
                    MIN(q1.y,q2.y) <= MAX(p1.y,p2.y);
        return ret;
    }
 
 
    //跨立判断
    static bool is_line_segment_cross(const Point &P1,const Point &P2,const Point & R1,const Point & R2){
        if(
            ((R1.x-P1.x)*(R1.y-R2.y)-(R1.y-P1.y)*( R1.x-R2.x)) * ((R1.x-P2.x)*(R1.y-R2.y)-(R1.y-P2.y)*(R1.x-R2.x)) < 0 ||
            ((P1.x-R1.x)*(P1.y-P2.y)-(P1.y-R1.y)*(P1.x-P2.x)) * ((P1.x-R2.x)*(P1.y-P2.y)-(P1.y-R2.y)*( P1.x-P2.x)) < 0
        ) 
            return true;
        else
        return false;
    }
 

    static bool get_cross_point(const Point &p1,const Point &p2,const Point &q1,const Point &q2,Point & out)
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

    Segment constrain(const Segment & seg, const Rect2i & rect){
        auto [p1, p2] = seg;
        bool has_p1 = rect.has_point(p1);
        bool has_p2 = rect.has_point(p2);
        Point intersection_point;
        Point rect_p1(rect.position.x, rect.position.y);
        Point rect_p2(rect.position.x + rect.size.x, rect.position.y);
        Point rect_p3(rect.position.x + rect.size.x, rect.position.y + rect.size.y);
        Point rect_p4(rect.position.x, rect.position.y + rect.size.y);

        switch((has_p1) + (has_p2)){
            case 2:return seg;
            case 1:{
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
                return {p1v, p2v};
            }
            default:
            case 0:{
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

                return {p1v,p2v};
            }
        }
    }

    Coast CoastUtils::constrain(const Coast & coast, const Rect2i & rect){
    
        if(coast.size() == 2){
            Segment seg{coast.front(), coast.back()};
            SegmentUtils::constrain(seg, rect);
            return Coast{seg.first, seg.second};
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
                Segment seg{point, nextpoint};
                SegmentUtils::constrain(seg, rect);
                ret.push_back(seg.first);
            }

            ret.push_back(point);
            
            if(next_exited_window){
                Segment seg{point, nextpoint};
                SegmentUtils::constrain(seg, rect);
                ret.push_back(seg.second);
                break;
            }


            it++;
        }

        return ret;
    }

    Coast CoastUtils::shrink(const Coast & line, const real_t width, const Vector2i & window_size){
        if(line.size() == 1){
            DEBUG_WARN("only one point");
        }else if(line.size() == 2){
            Rect2i window = Rect2i(Vector2i(), window_size);

            auto diff = line.front() - line.back();
            auto offset = Vector2i(diff.y, -diff.x).normalized() * width;

            Segment seg{line.front() + offset, line.back() + offset};
            SegmentUtils::constrain(seg, window);
            return {seg.first, seg.second};
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
                ret.push_back(new_point);
            ++it;
        }

        return ret;
    }

    Circle calculate_cicular(const Vector2 & px1, const Vector2 & px2, const Vector2 & px3)
    {
        real_t x1, y1, x2, y2, x3, y3;
        real_t a, b, c, g, e, f;
        x1 = px1.x;
        y1 = px1.y;
        x2 = px2.x;
        y2 = px2.y;
        x3 = px3.x;
        y3 = px3.y;
        e = 2 * (x2 - x1);
        f = 2 * (y2 - y1);
        g = x2*x2 - x1*x1 + y2*y2 - y1*y1;
        a = 2 * (x3 - x2);
        b = 2 * (y3 - y2);
        c = x3*x3 - x2*x2 + y3*y3 - y2*y2;
        real_t X = (g*b - c*f) / (e*b - a*f);
        real_t Y = (a*g - c*e) / (a*f - b*e);
        real_t R = sqrt((X-x1)*(X-x1)+(Y-y1)*(Y-y1));

        return {{X, Y}, R};
    }



    Circle calculate_cicular(const Coast & coast, const int begin_index, const int end_index){
        int S = CLAMP(begin_index, 0, coast.size() - 1);
        int End = CLAMP(end_index, 0, coast.size() - 1);
        int N = End- S;

        if (N < 3) {
            return {Vector2{0, 0}, std::numeric_limits<real_t>::infinity()};
        }

        Vector2 center;
        real_t r;
        
        real_t sumX = 0.0; 
        real_t sumY = 0.0;
        real_t sumX2 = 0.0;
        real_t sumY2 = 0.0;
        real_t sumX3 = 0.0;
        real_t sumY3 = 0.0;
        real_t sumXY = 0.0;
        real_t sumXY2 = 0.0;
        real_t sumX2Y = 0.0;

        for (int pId = S; pId < End; ++pId) {
            sumX += coast[pId].x;
            sumY += coast[pId].y;

            real_t x2 = coast[pId].x * coast[pId].x;
            real_t y2 = coast[pId].y * coast[pId].y;
            sumX2 += x2;
            sumY2 += y2;

            sumX3 += x2 * coast[pId].x;
            sumY3 += y2 * coast[pId].y;
            sumXY += coast[pId].x * coast[pId].y;
            sumXY2 += coast[pId].x * y2;
            sumX2Y += x2 * coast[pId].y;
        }



        real_t C, D, E, G, H;
        real_t a, b, c;

        C = N * sumX2 - sumX * sumX;
        D = N * sumXY - sumX * sumY;
        E = N * sumX3 + N * sumXY2 - (sumX2 + sumY2) * sumX;
        G = N * sumY2 - sumY * sumY;
        H = N * sumX2Y + N * sumY3 - (sumX2 + sumY2) * sumY;

        a = (H * D - E * G) / (C * G - D * D);
        b = (H * C - E * D) / (D * D - G * C);
        c = -(a * sumX + b * sumY + sumX2 + sumY2) / N;

        center.x = -a / 2.0;
        center.y = -b / 2.0;
        r = sqrt(a * a + b * b - 4 * c) / 2.0;

        real_t err = 0.0;
        real_t e;
        real_t r2 = r * r;

        for(int pId = S; pId < End; ++pId){
            Vector2 point = coast[pId];
            e = (point - center).length_squared() - r2;
            if (e > err) {
                err = e;
            }
        }

        return {center, r};
    }

    const Corner * CornerUtils::find_corner(const Corners & corners, const size_t from_index, const CornerType ct){
        if(ct == CornerType::NONE) return nullptr;
        if(corners.size() < from_index) return nullptr;

        for(size_t i = from_index; i < corners.size();++i){
            const auto * it = &corners[i];
            switch(ct){
                case CornerType::AC:
                case CornerType::VC:
                    if(it->type == ct){
                        return it;
                    }
                    break;
                case CornerType::ALL:
                    if(it->type != CornerType::NONE){
                        return (const Corner *)it;
                    }
                    break;
                default:
                    break;
            }
        }

        return nullptr;
    }

    const Corner * CornerUtils::find_a(const Corners & corners, const size_t from_index){
        return find_corner(corners, from_index, CornerType::AC);
    }
    const Corner * CornerUtils::find_v(const Corners & corners, const size_t from_index){
        return find_corner(corners, from_index, CornerType::VC);
    }

    static size_t cnt_of_corners(const Corners &corners, const size_t from_index, const CornerType ct){
        if(ct == CornerType::NONE) return 0;
        if(corners.size() < from_index) return 0;

        size_t cnt;
        for(size_t i = from_index; i < corners.size();++i){
            const auto * it = &corners[i];
            switch(ct){
                case CornerType::AC:
                case CornerType::VC:
                    if(it->type == ct){
                        cnt++;
                    }
                    break;
                case CornerType::ALL:
                    if(it->type != CornerType::NONE){
                        cnt++;
                    }
                    break;
                default:
                    break;
            }
        }

        return cnt;
    }

    size_t cnt_a(const Corners & corners, const size_t from_index = 0){
        return cnt_of_corners(corners, from_index, CornerType::AC);
    }

    size_t cnt_v(const Corners & corners, const size_t from_index = 0){
        return cnt_of_corners(corners, from_index, CornerType::VC);
    }

    bool CoastUtils::is_single(const Coast & coast, const LR side, const int fall_back){
        if(coast.size() < 2) return true;

        int expected_sign = 0;
        int inital_x = coast[0].x;

        switch (side){
        case LR::LEFT:
            expected_sign = -1;
            break;
        
        case LR::RIGHT:
            expected_sign = 1;
            break;

        default:
            break;
        }

        for(auto it = coast.begin(); it != std::prev(coast.end()); ++it){
            int dx = std::next(it)->x - it->x;
            int sgn = sign(dx);
            if((sgn != expected_sign) && (std::abs(dx) > fall_back)){
                return false;
            }
        }
        return true;
    }

    
    int CoastUtils::sigle_sign(const Coast & coast){
        if(coast.size() < 2) return 0;

        int initial_sgn = sign((coast[1] - coast[0]).x);
        if(coast.size() < 3) return initial_sgn;
        
        for(auto it = std::next(coast.begin()); it != std::prev(coast.end()); ++it){
            int sgn = sign(std::next(it)->x - it->x);
            if(sgn != initial_sgn) return 0; 
        }
        return initial_sgn;
    }
};