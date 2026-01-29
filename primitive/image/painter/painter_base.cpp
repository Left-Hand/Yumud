#include "painter_base.hpp"

#include "core/string/view/string_view.hpp"

using namespace ymd;

template<typename T = void>
using IResult = PainterBase::IResult<T>;

IResult<> PainterBase::draw_hollow_rect(const Rect2u & rect){
    // Rect2u regular = rect.abs();
    // if(!src_image -> get_view().intersects(regular)) return;

    // Range2u x_range = regular.x_range();
    // Range2u y_range = regular.y_range();

    // if(y_range.length() > 2){
    //     draw_hri_line(x_range, y_range.from);
    //     draw_hri_line(x_range, y_range.to - 1);
    //     Range2u shrunk_y_range = y_range.grow(-1);
    //     drawVerLine(shrunk_y_range, x_range.from);
    //     drawVerLine(shrunk_y_range, x_range.to - 1);
    // }else{
    //     drawFilledRect(Rect2u(x_range, y_range));
    // }

    if(const auto res = draw_hri_line(rect.x_range(), rect.top_left.y);
        res.is_err()) return res;
    if(const auto res = draw_hri_line(rect.x_range(), rect.top_left.y + rect.size.y - 1);
        res.is_err()) return res;
    if(const auto res = draw_ver_line(rect.y_range(), rect.top_left.x);
        res.is_err()) return res;
    if(const auto res = draw_ver_line(rect.y_range(), rect.top_left.x + rect.size.x - 1);
        res.is_err()) return res;

    return Ok();
}


IResult<> PainterBase::draw_hollow_circle(const Vec2u & pos, const uint radius){
    if(false == 
        Rect2u::from_center_and_halfsize(pos, Vec2u(radius, radius))
        .intersects(this->get_expose_rect().unwrap())
    ) return Ok();

    if(radius == 0) return Ok();
    if(radius < 0) return Err(Error::NegativeRadius);

    int x0 = pos.x;
    int y0 = pos.y;
    int x = radius - 1;
    int y = 0;
    int dx = 1;
    int dy = 1;
    int err=dx - 2 * radius;

    while (x>=y) {
        this->put_pixel_unchecked(Vec2u(x0-x, y0+y));
        this->put_pixel_unchecked(Vec2u(x0+x, y0+y));
        this->put_pixel_unchecked(Vec2u(x0-y, y0+x));
        this->put_pixel_unchecked(Vec2u(x0+y, y0+x));
        this->put_pixel_unchecked(Vec2u(x0-x, y0-y));
        this->put_pixel_unchecked(Vec2u(x0+x, y0-y));
        this->put_pixel_unchecked(Vec2u(x0-y, y0-x));
        this->put_pixel_unchecked(Vec2u(x0+y, y0-x));

        if (err<=0) {
            y++;
            err += dy;
            dy += 2;
        }
        if (err>0) {
            x--;
            dx += 2;
            err += dx-radius * 2;
        }
    }

    return Ok();
}

IResult<> PainterBase::draw_filled_circle(const Vec2u & pos, const uint radius){
    if(not Rect2u::from_center_and_halfsize(pos, Vec2u(radius, radius))
        .is_inside(get_expose_rect().unwrap())) return Ok();
    

    if(radius < 0) return Err(Error::NegativeRadius);
    if(radius == 0) return Ok();

    if(radius == 1){
        put_pixel_unchecked(pos);
        return Ok();
    }
    int x0 = pos.x;
    int y0 = pos.y;
    int x = ABS(radius) - 1;
    int y = 0;
    int dx = 1;
    int dy = 1;
    int err=dx - 2 * ABS(radius);

    while (x>=y) {
        // if(src_image->has_point(x0 - y))
        if(const auto res = draw_hri_line(Vec2u(x0 - x, y0 + y), 2*x);
            res.is_err()) return Err(res.unwrap_err());
        if(const auto res = draw_hri_line(Vec2u(x0 - y, y0 + x), 2*y);
            res.is_err()) return Err(res.unwrap_err());
        if(const auto res = draw_hri_line(Vec2u(x0 - x, y0 - y), 2*x);
            res.is_err()) return Err(res.unwrap_err());
        if(const auto res = draw_hri_line(Vec2u(x0 - y, y0 - x), 2*y);
            res.is_err()) return Err(res.unwrap_err());
    
        if (err<=0) {
            y++;
            err += dy;
            dy += 2;
        }
        if (err>0) {
            x--;
            dx += 2;
            err += dx-ABS(radius * 2);
        }
    }

    return Ok();
}


IResult<> PainterBase::draw_hollow_ellipse(const Vec2u & pos, const Vec2u & r) {
    int rx = r.x;
    int ry = r.y;
    if (rx == ry) return draw_hollow_circle(pos, rx);
    if (!this->get_expose_rect().unwrap().
        intersects(Rect2u::from_center_and_halfsize(pos, r))) return Ok();
    if (rx<2 || ry<2 ) return Ok();

    int x0 = pos.x;
    int y0 = pos.y;
    int x, y;
    int rx2 = rx * rx;
    int ry2 = ry * ry;
    int fx2 = 4 * rx2;
    int fy2 = 4 * ry2;
    int s;

    for (x = 0, y = ry, s = 2*ry2+rx2*(1-2*ry); ry2*x <= rx2*y; x++) {
        put_pixel_unchecked(Vec2u(x0 + x, y0 + y));
        put_pixel_unchecked(Vec2u(x0 - x, y0 + y));
        put_pixel_unchecked(Vec2u(x0 - x, y0 - y));
        put_pixel_unchecked(Vec2u(x0 + x, y0 - y));
        if (s >= 0) {
            s += fx2 * (1 - y);
            y--;
        }
        s += ry2 * ((4 * x) + 6);
    }

    for (x = rx, y = 0, s = 2*rx2+ry2*(1-2*rx); rx2*y <= ry2*x; y++) {
        put_pixel_unchecked(Vec2u(x0 + x, y0 + y));
        put_pixel_unchecked(Vec2u(x0 - x, y0 + y));
        put_pixel_unchecked(Vec2u(x0 - x, y0 - y));
        put_pixel_unchecked(Vec2u(x0 + x, y0 - y));
        if (s >= 0)
        {
            s += fy2 * (1 - x);
            x--;
        }
        s += rx2 * ((4 * y) + 6);
    }

    return Ok();
}

IResult<> PainterBase::draw_filled_ellipse(const Vec2u & pos, const Vec2u & r){
    int rx = r.x;
    int ry = r.y;
    if (rx == ry) return draw_hollow_circle(pos, rx);
    if (rx<2 || ry<2|| 
        (
            false == this->get_expose_rect()
            .unwrap()
            .intersects(Rect2u::from_center_and_halfsize(pos, r))
        )
    ) 
        return Ok();

    int x0 = pos.x;
    int y0 = pos.y;

    int x, y;
    int rx2 = rx * rx;
    int ry2 = ry * ry;
    int fx2 = 4 * rx2;
    int fy2 = 4 * ry2;
    int s;

    for (x = 0, y = ry, s = 2*ry2+rx2*(1-2*ry); ry2*x <= rx2*y; x++) {
        Range2u x_range = Range2u(x0 - x, x0 + x + 1);
        if(const auto res = draw_hri_line(x_range, y0 - y); res.is_err()) return res;
        if(const auto res = draw_hri_line(x_range, y0 + y); res.is_err()) return res;

        if (s >= 0) {
            s += fx2 * (1 - y);
            y--;
        }
        s += ry2 * ((4 * x) + 6);
    }

    for (x = rx, y = 0, s = 2*rx2+ry2*(1-2*rx); rx2*y <= ry2*x; y++) {
        Range2u x_range = Range2u(x0 - x, x0 + x + 1);
        if(const auto res = draw_hri_line(x_range, y0 - y); res.is_err()) return res;
        if(const auto res = draw_hri_line(x_range, y0 + y); res.is_err()) return res;

        if (s >= 0) {
            s += fy2 * (1 - x);
            x--;
        }
        s += rx2 * ((4 * y) + 6);
    }

    return Ok();
}


IResult<> PainterBase::draw_polyline(const std::span<const Vec2u> points){
    const auto count = points.size();
    for(size_t i = 0; i < count-1; i++){
        const auto res = draw_line(points[i], points[i + 1]);
        if(res.is_err()) return res;
    }
    return Ok();
}


IResult<> PainterBase::draw_polygon(const std::span<const Vec2u> points){
    const auto count = points.size();
    if(const auto res = draw_polyline(points);
        res.is_err()) return Err(res.unwrap_err());
    if(const auto res = draw_line(points[0], points[count - 1]);
        res.is_err()) return Err(res.unwrap_err());
    return Ok();
}

IResult<> PainterBase::draw_hollow_triangle(const Vec2u & p0,const Vec2u & p1,const Vec2u & p2){
    if(const auto res = draw_line(p0, p1); res.is_err()) return res;
    if(const auto res = draw_line(p1, p2); res.is_err()) return res;
    if(const auto res = draw_line(p0, p2); res.is_err()) return res;
    return Ok();
}

IResult<> PainterBase::draw_filled_triangle(const Vec2u & p0,const Vec2u & p1,const Vec2u & p2){
    int a, b, last;
    int x0 = p0.x;
    int y0 = p0.y;
    int x1 = p1.x;
    int y1 = p1.y;
    int x2 = p2.x;
    int y2 = p2.y;

    if (y0 > y1) {
        std::swap(y0, y1);
        std::swap(x0, x1);
    }
    if (y1 > y2) {
        std::swap(y2, y1);
        std::swap(x2, x1);
    }
    if (y0 > y1) {
        std::swap(y0, y1);
        std::swap(x0, x1);
    }

    if (y0 == y2) { // Handle awkward all-on-same-line case as its own thing
        a = b = x0;
        if (x1 < a)      a = x1;
        else if (x1 > b) b = x1;
        if (x2 < a)      a = x2;
        else if (x2 > b) b = x2;
        if(const auto res = draw_hri_line(Range2u(a, b + 1), y0); res.is_err()) return res;
        return Ok();
    }

    int dx01 = x1 - x0;
    int dy01 = y1 - y0;
    int dx02 = x2 - x0;
    int dy02 = y2 - y0;
    int dx12 = x2 - x1;
    int dy12 = y2 - y1;
    int sa   = 0;
    int sb   = 0;

    if (y1 == y2) last = y1;  // Include y1 scanline
    else         last = y1 - 1; // Skip it

    int y;
    for (y = y0; y <= last; y++) {
        a   = x0 + sa / dy01;
        b   = x0 + sb / dy02;
        sa += dx01;
        sb += dx02;

        if (a > b) SWAP(a, b);
        if(const auto res = draw_hri_line(Range2u(a, b + 1), y); res.is_err()) return res;
    }

    sa = dx12 * (y - y1);
    sb = dx02 * (y - y0);
    for (; y <= y2; y++) {
        a   = x1 + sa / dy12;
        b   = x0 + sb / dy02;
        sa += dx12;
        sb += dx02;

        if (a > b) SWAP(a, b);
        if(const auto res = draw_hri_line(Range2u(a, b + 1), y); res.is_err()) return res;
    }

    return Ok();
}

IResult<> PainterBase::draw_roi(const Rect2u & rect){
    Vec2u center = rect.center();
    if(const auto res = draw_hollow_rect(rect); res.is_err()) return res;
    if(const auto res = draw_hri_line(center+Vec2u(-2,0), 5); res.is_err()) return res;
    if(const auto res = draw_ver_line(center+Vec2u(0,-2), 5); res.is_err()) return res;

    return Ok();
}


IResult<> PainterBase::draw_hri_line(const Vec2u & pos,const int l){
    const auto ins = Rect2u(pos, Vec2u(l, 1));
    if(ins.area() == 0) return Err(Error::AreaNotExist);
    return draw_filled_rect(ins);
}
IResult<> PainterBase::draw_ver_line(const Vec2u & pos,const int l){
    const auto ins = Rect2u(pos, Vec2u(1, l));
    if(ins.area() == 0) return Err(Error::AreaNotExist);
    return draw_filled_rect(ins);
}
IResult<> PainterBase::draw_ver_line(const Range2u & y_range, const int x){
    const auto y_range_regular = y_range.abs();
    return draw_ver_line(Vec2u(x, y_range_regular.start), y_range_regular.length());
}

IResult<> PainterBase::draw_hri_line(const Range2u & x_range, const int y){
    const auto x_range_regular = x_range.abs();
    return draw_hri_line(Vec2u(x_range_regular.start, y), x_range_regular.length());
}
