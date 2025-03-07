#include "misc.hpp"
#include "sys/clock/clock.h"
#include "sys/core/system.hpp"
#include "sys/clock/time.hpp"


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
    painter.setColor(RGB888(HSV888(int(time() * 64),255,255)));
    painter.drawFilledTriangle(pf, p1, p2);
    // painter.drawPixel(org);

    painter.setColor(ColorEnum::BLACK);
    painter.drawHollowTriangle(pf, p1, p2);
};

void test_servo(RadianServo & servo, std::function<real_t(real_t)> && func){
    while(true){
        auto targ = func(time());
        servo.setRadian(targ);
        // DEBUG_PRINTLN(joint.getRadian(), targ);
        delay(20);
    }
}

void test_joint(JointLR & joint, std::function<real_t(real_t)> && func){
    while(true){
        auto targ = func(time());
        joint.setRadian(targ);
        // DEBUG_PRINTLN(joint.getRadian(), targ);
        delay(20);
    }
}


PMW3901 create_pmw(){
    auto & pmw_spi = spi1;

    pmw_spi.init(9_MHz);
    pmw_spi.bindCsPin(portD[5], 0);
    return {pmw_spi, 0};
    // PMW3901 pmw{pmw_spi, 0};
    // pmw.init();
    // return pmw;
}

void init_pmw(PMW3901 & pmw){
    pmw.init();
}


ST7789 create_displayer(){
    auto & lcd_cs = portD[6];
    auto & lcd_dc = portD[7];
    auto & dev_rst = portB[7];


    #ifdef CH32V30X
    auto & lcd_spi = spi2;
    #else
    auto & lcd_spi = spi1;
    #endif
    
    lcd_spi.init(144_MHz, CommMethod::Blocking, CommMethod::None);
    lcd_spi.bindCsPin(lcd_cs, 0);

    return {{{lcd_spi, 0}, lcd_dc, dev_rst}, {240, 135}};
}


void init_displayer(ST7789 & displayer){
    displayer.init();

    // displayer.setFlipX(false);
    // displayer.setFlipY(true);

    displayer.setFlipX(true);
    displayer.setFlipY(false);

    if(true){
        displayer.setSwapXY(true);
        displayer.setDisplayOffset({40, 53}); 
    }else{
        displayer.setSwapXY(false);
        displayer.setDisplayOffset({53, 40}); 
    }

    displayer.setFormatRGB(true);
    displayer.setFlushDirH(false);
    displayer.setFlushDirV(false);
    displayer.setInversion(true);

    displayer.fill(ColorEnum::BLACK);
}


String getline(InputStream & logger){
    while(true){
        if(logger.available()){
            return logger.readString();
        }
    }
}

void bind_tick_200hz(std::function<void(void)> && func){
    auto & timer = timer3;
    timer.init(200);

    timer.bindCb(TimerIT::Update, std::move(func));
    timer.enableIt(TimerIT::Update, {1,0});
    DEBUG_PRINTLN("tick200 binded");
};


void bind_tick_800hz(std::function<void(void)> && func){
    auto & timer = timer3;
    timer.init(800);

    timer.bindCb(TimerIT::Update, std::move(func));
    timer.enableIt(TimerIT::Update, {1,0});
    DEBUG_PRINTLN("tick800 binded");
};


void bind_tick_50hz(std::function<void(void)> && func){
    // if(true){//绑定50hz舵机更新回调函数
    auto & timer = timer2;
    timer.init(50);

    timer.bindCb(TimerIT::Update, std::move(func));
    timer.enableIt(TimerIT::Update, {1,0});
    DEBUG_PRINTLN("tick50 binded");
    // }
}

void bind_tick_1khz(std::function<void(void)> && func){
    // if(true){//绑定滴答时钟
    bindSystickCb(std::move(func));
    DEBUG_PRINTLN("tick1k binded");
    // }
}

}