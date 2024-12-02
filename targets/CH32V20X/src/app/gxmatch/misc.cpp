#include "misc.hpp"

namespace gxm{



void initDisplayer(ST7789 & tftDisplayer){
    tftDisplayer.init();

    tftDisplayer.setFlipX(false);
    tftDisplayer.setFlipY(true);
    if(true){
        tftDisplayer.setSwapXY(true);
        tftDisplayer.setDisplayOffset({40, 52}); 
    }else{
        tftDisplayer.setSwapXY(false);
        tftDisplayer.setDisplayOffset({52, 40}); 
    }
    tftDisplayer.setFormatRGB(true);
    tftDisplayer.setFlushDirH(false);
    tftDisplayer.setFlushDirV(false);
    tftDisplayer.setInversion(true);

    tftDisplayer.fill(ColorEnum::BLACK);
}


Ray canvas_transform(const Ray & ray){
    scexpr auto meter = int{2};
    scexpr auto size = Vector2{100,100};
    scexpr auto org =  Vector2{12,12};
    scexpr auto area = Rect2i{org,size};
    
    auto x = LERP(real_t(area.x), real_t(area.x + area.w), ray.org.x / meter);
    auto y = LERP(real_t(area.y + area.h), real_t(area.y), ray.org.y / meter);
    return Ray{Vector2{x,y} + Vector2::ones(12), ray.rad};
};


void draw_curve(PainterConcept & painter, const Curve & curve){
    painter.setColor(ColorEnum::BLUE);
    for(auto it = curve.begin(); it != curve.end(); it++){
        auto pos = canvas_transform(Ray(*it)).org;
        painter.drawPixel(pos);
    }
};

void print_curve(OutputStream & logger, const Curve & curve){
    logger << std::setprecision(4);
    for(auto it = curve.begin(); it != curve.end(); it++){
        auto [pos, rad] = Ray(*it);
        delay(1);
        logger.println(pos.x, pos.y, rad);
    }
};


void draw_turtle(PainterConcept & painter, const Ray & ray){
    scexpr real_t len = 7;
    auto [org, rad] = canvas_transform(ray);
    rad = -rad;//flipy
    auto pf = org + Vector2::from_angle(len, rad);
    auto p1 = org + Vector2::from_angle(len, rad + real_t(  PI * 0.8));
    auto p2 = org + Vector2::from_angle(len, rad + real_t(- PI * 0.8));

    // painter.setColor(ColorEnum::RED);
    painter.setColor(RGB888(HSV888(int(t * 64),255,255)));
    painter.drawFilledTriangle(pf, p1, p2);
    // painter.drawPixel(org);

    painter.setColor(ColorEnum::BLACK);
    painter.drawHollowTriangle(pf, p1, p2);
};

void test_servo(RadianServo & servo, std::function<real_t(real_t)> && func){
    while(true){
        auto targ = func(t);
        servo.setRadian(targ);
        // DEBUG_PRINTLN(joint.getRadian(), targ);
        delay(20);
    }
}

void test_joint(JointLR & joint, std::function<real_t(real_t)> && func){
    while(true){
        auto targ = func(t);
        joint.setRadian(targ);
        // DEBUG_PRINTLN(joint.getRadian(), targ);
        delay(20);
    }
}


String getline(InputStream & logger){
    while(true){
        if(logger.available()){
            return logger.readString();
        }
    }
}
}