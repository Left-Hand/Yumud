#include "painter_concept.hpp"

#include "sys/string/string.hpp"
#include "sys/string/string_view.hpp"

using namespace yumud;

void PainterConcept::drawHollowRect(const Rect2i & rect){
    // Rect2i regular = rect.abs();
    // if(!src_image -> get_view().intersects(regular)) return;

    // Rangei x_range = regular.get_x_range();
    // Rangei y_range = regular.get_y_range();

    // if(y_range.length() > 2){
    //     drawHriLine(x_range, y_range.from);
    //     drawHriLine(x_range, y_range.to - 1);
    //     Rangei shrunk_y_range = y_range.grow(-1);
    //     drawVerLine(shrunk_y_range, x_range.from);
    //     drawVerLine(shrunk_y_range, x_range.to - 1);
    // }else{
    //     drawFilledRect(Rect2i(x_range, y_range));
    // }
    auto p1 = rect.position;
    auto p2 = rect.position + Vector2i{rect.size.x, 0};
    auto p3 = rect.position + rect.size;
    auto p4 = rect.position + Vector2i{0, rect.size.y};
    drawLine(p1,p2);
    drawLine(p2,p3);
    drawLine(p4,p3);
    drawLine(p1,p4);
}
void PainterConcept::drawHollowCircle(const Vector2i & pos, const uint radius){
    if((!(Rect2i::from_center(pos, Vector2i(radius, radius)).intersects(this->getClipWindow()))) || radius <= 0) return;

    int x0 = pos.x;
    int y0 = pos.y;
    int x = radius - 1;
    int y = 0;
    int dx = 1;
    int dy = 1;
    int err=dx - 2 * radius;

    while (x>=y) {
        this->drawPixel(Vector2i(x0-x, y0+y));
        this->drawPixel(Vector2i(x0+x, y0+y));
        this->drawPixel(Vector2i(x0-y, y0+x));
        this->drawPixel(Vector2i(x0+y, y0+x));
        this->drawPixel(Vector2i(x0-x, y0-y));
        this->drawPixel(Vector2i(x0+x, y0-y));
        this->drawPixel(Vector2i(x0-y, y0-x));
        this->drawPixel(Vector2i(x0+y, y0-x));

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
}

void PainterConcept::drawFilledCircle(const Vector2i & pos, const uint radius){
    if((!(Rect2i::from_center(pos, Vector2i(radius, radius)).inside(Rect2i(Vector2i(), getClipWindow().size)))) || radius == 0) return;
    if(radius == 1){
        return drawPixel(pos);
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
        drawHriLine(Vector2i(x0 - x, y0 + y), 2*x);
        drawHriLine(Vector2i(x0 - y, y0 + x), 2*y);
        drawHriLine(Vector2i(x0 - x, y0 - y), 2*x);
        drawHriLine(Vector2i(x0 - y, y0 - x), 2*y);

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
}



void PainterConcept::drawString(const Vector2i & pos, const String & str){
    drawStr(pos, str.c_str(), str.length());
}

void PainterConcept::drawString(const Vector2i & pos, const char * str){
    drawStr(pos, str, strlen(str));
}

void PainterConcept::drawString(const Vector2i & pos, const StringView & str){
    drawStr(pos, str.data(), str.length());
}

void PainterConcept::drawHollowEllipse(const Vector2i & pos, const Vector2i & r) {
    int rx = r.x;
    int ry = r.y;
    if (rx == ry) return drawHollowCircle(pos, rx);
    if (rx<2 || ry<2|| !this->getClipWindow().intersects(Rect2i::from_center(pos, r))) return;

    int x0 = pos.x;
    int y0 = pos.y;
    int x, y;
    int rx2 = rx * rx;
    int ry2 = ry * ry;
    int fx2 = 4 * rx2;
    int fy2 = 4 * ry2;
    int s;

    for (x = 0, y = ry, s = 2*ry2+rx2*(1-2*ry); ry2*x <= rx2*y; x++) {
        drawPixel(Vector2i(x0 + x, y0 + y));
        drawPixel(Vector2i(x0 - x, y0 + y));
        drawPixel(Vector2i(x0 - x, y0 - y));
        drawPixel(Vector2i(x0 + x, y0 - y));
        if (s >= 0) {
            s += fx2 * (1 - y);
            y--;
        }
        s += ry2 * ((4 * x) + 6);
    }

    for (x = rx, y = 0, s = 2*rx2+ry2*(1-2*rx); rx2*y <= ry2*x; y++) {
        drawPixel(Vector2i(x0 + x, y0 + y));
        drawPixel(Vector2i(x0 - x, y0 + y));
        drawPixel(Vector2i(x0 - x, y0 - y));
        drawPixel(Vector2i(x0 + x, y0 - y));
        if (s >= 0)
        {
            s += fy2 * (1 - x);
            x--;
        }
        s += rx2 * ((4 * y) + 6);
    }
}

void PainterConcept::drawFilledEllipse(const Vector2i & pos, const Vector2i & r){
    int rx = r.x;
    int ry = r.y;
    if (rx == ry) return drawHollowCircle(pos, rx);
    if (rx<2 || ry<2|| !this->getClipWindow().intersects(Rect2i::from_center(pos, r))) return;

    int x0 = pos.x;
    int y0 = pos.y;

    int x, y;
    int rx2 = rx * rx;
    int ry2 = ry * ry;
    int fx2 = 4 * rx2;
    int fy2 = 4 * ry2;
    int s;

    for (x = 0, y = ry, s = 2*ry2+rx2*(1-2*ry); ry2*x <= rx2*y; x++) {
        Rangei x_range = Rangei(x0 - x, x0 + x + 1);
        drawHriLine(x_range, y0 - y);
        drawHriLine(x_range, y0 + y);

        if (s >= 0) {
            s += fx2 * (1 - y);
            y--;
        }
        s += ry2 * ((4 * x) + 6);
    }

    for (x = rx, y = 0, s = 2*rx2+ry2*(1-2*rx); rx2*y <= ry2*x; y++) {
        Rangei x_range = Rangei(x0 - x, x0 + x + 1);
        drawHriLine(x_range, y0 - y);
        drawHriLine(x_range, y0 + y);

        if (s >= 0) {
            s += fy2 * (1 - x);
            x--;
        }
        s += rx2 * ((4 * y) + 6);
    }
}


void PainterConcept::drawPolyline(const Vector2i * points, const size_t count){
    for(size_t i = 0; i < count-1; i++) drawLine(points[i], points[i + 1]);
}


void PainterConcept::drawPolygon(const Vector2i * points, const size_t count){
    drawPolyline(points, count);
    drawLine(points[0], points[count - 1]);
}

void PainterConcept::drawHollowTriangle(const Vector2i & p0,const Vector2i & p1,const Vector2i & p2){
    drawLine(p0, p1);
    drawLine(p1, p2);
    drawLine(p0, p2);
}

void PainterConcept::drawFilledTriangle(const Vector2i & p0,const Vector2i & p1,const Vector2i & p2){
    int a, b, y, last;
    int x0 = p0.x;
    int y0 = p0.y;
    int x1 = p1.x;
    int y1 = p1.y;
    int x2 = p2.x;
    int y2 = p2.y;

    if (y0 > y1) {
        SWAP(y0, y1);
        SWAP(x0, x1);
    }
    if (y1 > y2) {
        SWAP(y2, y1);
        SWAP(x2, x1);
    }
    if (y0 > y1) {
        SWAP(y0, y1);
        SWAP(x0, x1);
    }

    if (y0 == y2) { // Handle awkward all-on-same-line case as its own thing
        a = b = x0;
        if (x1 < a)      a = x1;
        else if (x1 > b) b = x1;
        if (x2 < a)      a = x2;
        else if (x2 > b) b = x2;
        drawHriLine(Rangei(a, b + 1), y);
        return;
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

    for (y = y0; y <= last; y++) {
        a   = x0 + sa / dy01;
        b   = x0 + sb / dy02;
        sa += dx01;
        sb += dx02;

        if (a > b) SWAP(a, b);
        drawHriLine(Rangei(a, b + 1), y);
    }

    sa = dx12 * (y - y1);
    sb = dx02 * (y - y0);
    for (; y <= y2; y++) {
        a   = x1 + sa / dy12;
        b   = x0 + sb / dy02;
        sa += dx12;
        sb += dx02;

        if (a > b) SWAP(a, b);
        drawHriLine(Rangei(a, b + 1), y);
    }
}

void PainterConcept::drawRoi(const Rect2i & rect){
    drawHollowRect(rect);
    Vector2i center = rect.get_center();
    drawHriLine(center+Vector2i(-2,0), 5);
    drawVerLine(center+Vector2i(0,-2), 5);
}

void PainterConcept::drawPolyline(const std::initializer_list<Vector2i> & points){
    drawPolyline(points.begin(), points.size());
}

void PainterConcept::drawPolygon(const std::initializer_list<Vector2i> & points){
    drawPolygon(points.begin(), points.size());
}
