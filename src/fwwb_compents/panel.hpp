#ifndef __FWWB_PANEL_HPP__

#define __FWWB_PANEL_HPP__

#include "simple_demodem.hpp"
#include "src/device/CommonIO/Led/rgbLed.hpp"
#include "src/system.hpp"

namespace FWWB{

using Sys::t;

class PanelTarget{
protected:
// public:
    SimpleDeModem & demodem;
    std::function<void(void)> cb;
    const uint8_t species;
    const uint8_t index;
    volatile uint8_t combo_cnt = 0;
    static constexpr uint8_t combo_thr = 3;
    volatile bool shotted = false;
    volatile bool outline = false;

    void update(const uint8_t & code){
        if(!shotted){
            if(code == species){
                combo_cnt++;
                if(combo_cnt >= combo_thr){
                    shotted = true;
                    combo_cnt = 0;
                }
            }else{
                if(code == 0){
                    outline = true;
                }
                combo_cnt = 0;
            }
        }
    }
public:
    PanelTarget(SimpleDeModem & _demodem, const uint8_t & _species, const uint8_t & _index):demodem(_demodem), species(_species), index(_index){;}

    void init(){
        demodem.init();
        demodem.bindCb([this](){
            this->update(demodem.getCode());
        });
    }

    bool isShotted(){
        volatile bool _shotted = shotted;
        shotted = false;
        return _shotted;
    }

    bool isOutline(){
        return outline;
    }

    void bindCb(const std::function<void(void)>& _cb){
        cb = _cb;
    }
};

class PanelLed{
public:
    enum class Method:uint8_t{
        Sine = 0,
        Saw,
    };
protected:
    RgbLedConcept<true> & led;

    using Color = Color_t<real_t>;

    Color color_a;
    Color color_b;
    uint16_t period;

    Method method;
public:
    PanelLed(RgbLedConcept<true> & _led) : led(_led){;}

    void init(){
        led.init();
    }

    void setPeriod(const uint16_t & _period){
        period = _period;
    }

    void setTranstit(const Color & _color_a, const Color & _color_b, const Method & _method){
        color_a = _color_a;
        color_b = _color_b;
        method = _method;
    }

    void run(){
        real_t ratio;
        real_t _t = t / (real_t(period) / 1000);
        switch(method){
        case Method::Saw:
            ratio = frac(_t);
            break;
        case Method::Sine:
            ratio = abs(2 * frac(_t) - 1);
        }

        Color color_mux = color_a.linear_interpolate(color_b, ratio);
        led = color_mux;
    }

    PanelLed & operator = (const Color & color){
        led = color;
        return *this;
    }
};

class PanelUnit{
protected:
    enum class StateMachine:uint8_t{
        NONE, IDLE, SHOTTED, RECOVER, DIED
    };
    StateMachine sm = StateMachine::NONE;

    using Color = Color_t<real_t>;
public:
    PanelTarget & target;
    PanelLed & led;

    int32_t shot_tick = -10000;
    static constexpr uint32_t besta_time = 1600;
    static constexpr uint32_t transtit_time = 2400;
    static constexpr uint8_t max_hp = 2;
    bool shot = false;

    Color from_hp_to_color(const uint8_t & _hp){
        // return Color::from_hsv(real_t(MAX(_hp, 0)) / max_hp * real_t(0.6666), real_t(1), real_t(1));
        switch(_hp){
        case 0:
            return Color(1,0,0);
        case 1:
            return Color(0,1,0);
        case 2:
            return Color(0,0,1);
        default:
            return Color();
        }
    }

    void sw(const StateMachine & _sm){
        switch(_sm){
        case StateMachine::IDLE:
            led.setPeriod(1200);
            {
                Color hp_color = from_hp_to_color(hp);
                led.setTranstit(hp_color, Color(), PanelLed::Method::Sine);
            }
            break;
        case StateMachine::RECOVER:
            led.setPeriod(400);
            goto saw_transtit;
        case StateMachine::SHOTTED:
            led.setPeriod(200);

        saw_transtit:
            led.setTranstit(from_hp_to_color(hp + 1), from_hp_to_color(hp), PanelLed::Method::Saw);
            sm = StateMachine::SHOTTED;
            break;
        case StateMachine::DIED:
            {
                Color hp_color = from_hp_to_color(0);
                led.setTranstit(hp_color, hp_color, PanelLed::Method::Sine);
            }
        default:
            break;
        }

        sm = _sm;
    }
public:
    uint8_t hp = max_hp;

    PanelUnit(PanelTarget & _target, PanelLed & _led) : target(_target), led(_led){;}

    void init(){
        target.init();
        led.init();
    }

    void run(){
        switch(sm){
        case StateMachine::NONE:
            sw(StateMachine::IDLE);
        case StateMachine::IDLE:
            if(target.isOutline()){
                hp = 0;
                sw(StateMachine::DIED);
            }else if(target.isShotted()){
                hp = MAX(0, hp - 1);
                if(hp > 0) sw(StateMachine::SHOTTED);
                else sw(StateMachine::DIED);
                shot = true;
                shot_tick = millis();
            }
            break;
        case StateMachine::SHOTTED:
            if(millis() - shot_tick > besta_time){
                sw(StateMachine::RECOVER);
            }
            break;
        case StateMachine::RECOVER:
            if(millis() - shot_tick > transtit_time + besta_time){
                target.isShotted();
                sw(StateMachine::IDLE);
            }
        case StateMachine::DIED:
            break;
        }

        led.run();
    }

    bool isShotted(){
        auto _shot = shot;
        shot = false;
        return _shot;
    }

    bool isOutline() const{
        return target.isOutline();
    }

    bool isDied() const{
        return sm == StateMachine::DIED;
    }
};
};
#endif