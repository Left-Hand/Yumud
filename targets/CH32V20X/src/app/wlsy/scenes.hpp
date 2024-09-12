#ifndef __SCENCES_HPP__

#define __SCENCES_HPP__

#include "wlsy_inc.hpp"
#include "backend.hpp"
#include "integrator.hpp"
#include "tjc.hpp"


namespace WLSY{

using Waveform = TJC::Waveform;
using WaveWindow = TJC::WaveWindow;
using Label = TJC::Label;


enum class SubSceneIndex:uint8_t{
    MAIN,
    INPUT,
    OUTPUT, 
    EXAM
};

class Scene{


protected:
    BackModule & bm;
public:
    virtual void run() = 0;
    Scene(BackModule & _bm):bm(_bm){;}
};

class RootScene:public Scene{
public:
    RootScene(BackModule & _bm):Scene(_bm){;}
};

class SubScene:public Scene{
public:
    SubScene(BackModule & _bm): Scene(_bm){;}
};




struct PowerInScene:public SubScene{
protected:
    struct Context{
        Label * amps_label;
        Label * volt_label;
        Label * watt_label;

        Label * heat_l_label;
        Label * heat_h_label;

        Waveform * amps_wave;
        Waveform * volt_wave;
        Waveform * watt_wave;
    }context;
public:
    PowerInScene(BackModule & _bm, Context && _context):SubScene(_bm), context(_context){;}

    void run() override{
        auto im_info = bm.getInputModuleInfos();
        context.amps_wave->addData(im_info.amps);
        context.volt_wave->addData(im_info.volt);
        context.watt_wave->addData(im_info.watt);

        context.amps_label->setValue(im_info.amps);
        context.volt_label->setValue(im_info.volt);
        context.watt_label->setValue(im_info.watt);

        context.heat_l_label->setText(String(float(im_info.tmp_l),1) + "℃");
        context.heat_h_label->setText(String(float(im_info.tmp_h),1) + "℃");

    }
};

class PowerOutScene:public SubScene{
protected:
    struct Context{
        Label * speed_label;
        Label * force_label;
        Label * power_label;

        Waveform * speed_wave;
        Waveform * force_wave;
        Waveform * power_wave;
    };
    
    const Context context;
public:
    PowerOutScene(BackModule & _bm, Context && _context):SubScene(_bm), context(_context){;}
    void run() override{
        auto om_info = bm.getOutputModuleInfos();
        context.speed_wave->addData(om_info.speed);
        context.force_wave->addData(om_info.force);
        context.power_wave->addData(om_info.watt);


        context.speed_label->setValue(om_info.speed);
        context.force_label->setValue(om_info.force);
        context.power_label->setValue(om_info.watt);

        // DEBUG_PRINTLN(om_info.speed, om_info.force, om_info.watt);
    }
};

class ExamScene:public SubScene{
protected:

    struct Context{
        Label * spower_label;
        Label * ipower_label;
        Label * opower_label;
        Label * effi_label;
    };

    const Context context;
public:
    ExamScene(BackModule & _bm,Context && _context):SubScene(_bm), context(_context){;}

    void run() override{
        auto info = bm.getAverage();

        context.spower_label->setValue(info.spower);
        context.ipower_label->setValue(info.ipower);
        context.opower_label->setValue(info.opower);
        context.effi_label->setText(String(info.effiency * 100) + "%");
    };
};

class MainScene:public RootScene{
protected:
    enum class ExamStatus:uint8_t{
        IDLE,
        RUNNING,
        STOP
    };

    struct Context{
        PowerInScene * powerin_scene;
        PowerOutScene * powerout_scene;
        ExamScene * exam_scene;
    };

    SubSceneIndex sub_scene_index = SubSceneIndex::MAIN;
    ExamStatus exam_status = ExamStatus::IDLE;

    const Context context;
public:


    MainScene(BackModule & _bm, Context && _context):RootScene(_bm), context(_context){;}

    void changeScene(SubSceneIndex index){
        sub_scene_index = index;
    }

    void run() override{
        switch(sub_scene_index){
            default:
                break;
            case SubSceneIndex::MAIN:
                break;
            case SubSceneIndex::INPUT:
                context.powerin_scene->run();
                break;
            case SubSceneIndex::OUTPUT:
                context.powerout_scene->run();
                break;
            case SubSceneIndex::EXAM:
                context.exam_scene->run();
                break;
        }
    }
};
}

#endif