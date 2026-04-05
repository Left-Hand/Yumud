#pragma once

#include "sr2631z3_prelude.hpp"

namespace ymd::drivers::sr2631z3{


namespace msgs{

// 卫星信息项(GSV专用)
struct [[nodiscard]] SatelliteInfo final{
    Option<int> sv_id;     // 卫星编号
    Option<int> elevation; // 仰角(0~90°)
    Option<int> azimuth;   // 方位角(0~359°)
    Option<int> snr;       // 载噪比(0~99dBHz)
};

// 1. GGA 时间、位置、定位类型
struct [[nodiscard]] Gga final{
    using Self = Gga;

    // UTC时间
    Option<UtcTime> utc_time;          

    // 纬度值
    Option<Lat> lat;                   

    // 纬度方向(N/S)
    Option<uLat> ulat;         

    // 经度值
    Option<Lon> lon;                   

    // 经度方向(E/W)
    Option<uLon> ulon;         

    // 定位质量标志(0-6)
    Fs fix_status;            

    // 参与定位卫星数
    Option<size_t> num_satellites;       

    // 水平精度因子
    // Option<Hdop> hdop;                 

    // 海拔高度(米)
    Option<uint32_t> msl_alt;            

    // 高度单位(固定M)
    char msl_unit;        

    // 高程异常(米)
    Option<uint32_t> geo_sep;            

    // 高程异常单位(固定M)
    char geo_sep_unit;    

    // 差分数据龄期
    Option<uint32_t> diff_age;           

    // 差分参考站ID
    Option<uint32_t> diff_station_id;          


    // static constexpr Self from_uninitialized(){
    //     return Self{
    //         .utc_time = None,
    //         .lat = None,
    //         .ulat = None,
    //         .lon = None,
    //         .ulon = None
    //     };
    // }
};

// 2. GLL 经纬度、定位时间、状态
struct [[nodiscard]] Gll final{
    using Self = Gll;

    // 纬度值
    Option<Lat> lat;                   

    // 纬度方向(N/S)
    Option<uLat> ulat;         

    // 经度值
    Option<Lon> lon;                   

    // 经度方向(E/W)
    Option<uLon> ulon;         

    // 定位UTC时间
    Option<UtcTime> utc_time;          

    // 数据有效性(A=有效/V=无效)
    Option<char> valid;                

    // 定位模式(A/D/E/M/N)
    Option<char> mode;                 

};


struct Smode{
    char chr;
};

using Pdop = float;
using Hdop = float;
using Vdop = float;

// 3. GSA 卫星状态、DOP值
struct [[nodiscard]] Gsa final{
    using Self = Gsa;

    // 模式切换(A=自动/M=手动)
    Option<Smode> switch_mode;          

    // 定位状态(1=无效/2=2D/3=3D)
    Option<int> fix_mode;              

    // 参与定位卫星编号(最多12颗)
    Option<int> sv_ids[12];            

    // 位置精度因子
    Option<Pdop> pdop;                 

    // 水平精度因子
    // Option<Hdop> hdop;                 

    // 垂直精度因子
    Option<Vdop> vdop;                 

    // NMEA4.11+系统ID
    Option<int> system_id;             

};

// 4. GSV 可见卫星信息
struct [[nodiscard]] Gsv final{
    using Self = Gsv;

    // 本批次总语句数
    Option<int> total_msg;             

    // 当前语句编号
    Option<int> current_msg;           

    // 可见卫星总数
    Option<int> total_sv;              

    // 卫星信息(每条最多4颗)
    Option<SatelliteInfo> sv_info[4];  

    // GNSS信号ID
    Option<int> signal_id;             

};

// 5. RMC 推荐最小定位信息
struct [[nodiscard]] Rmc final{
    using Self = Rmc;

    // UTC时间
    Option<UtcTime> utc_time;          

    // 状态(A=有效/V=无效)
    Option<char> status;               

    // 纬度值
    Option<Lat> lat;                   

    // 纬度方向(N/S)
    Option<uLat> ulat;         

    // 经度值
    Option<Lon> lon;                   

    // 经度方向(E/W)
    Option<uLat> lon_dir;         

    // 对地速度(节)
    // Option<Speed> speed_knot;          

    // 对地真航向(度)
    // Option<Angle> true_course;         

    // 日期(ddmmyy)
    // Option<Date> date;                 

    // 磁偏角(预留空)
    // Option<Angle> mag_var;             

    // 磁偏角方向(预留空)
    Option<uLat> mag_var_dir;     

    // 定位模式(A/D/E/M/N)
    Option<char> mode;                 

    // 导航状态(固定V)
    Option<char> nav_status;           

};

// 6. VTG 地面速度、航向
struct [[nodiscard]] Vtg final{
    using Self = Vtg;

    // 真北航向(度)
    // Option<Angle> true_course;         

    // 真北标志(固定T)
    Option<char> true_flag;            

    // 磁北航向(度)
    // Option<Angle> mag_course;          

    // 磁北标志(固定M)
    Option<char> mag_flag;             

    // 速度(节)
    // Option<Speed> speed_knot;          

    // 节单位(固定N)
    Option<char> knot_unit;            

    // 速度(公里/小时)
    // Option<Speed> speed_kmh;           

    // 公里/小时单位(固定K)
    Option<char> kmh_unit;             

    // 定位模式(A/D/E/M/N)
    Option<char> mode;                 

};

// 7. ZDA 时间、日期
struct [[nodiscard]] Zda final{
    using Self = Zda;

    // UTC时间
    Option<UtcTime> utc_time;          

    // 日(01-31)
    Option<int> day;                   

    // 月(01-12)
    Option<int> month;                 

    // 年(4位)
    Option<int> year;                  

    // 时区小时(固定00)
    Option<int> tz_hour;               

    // 时区分钟(固定00)
    Option<int> tz_minute;             

};

// 8. TXT 天线状态检测
struct [[nodiscard]] Txt final{
    using Self = Txt;

    // 总语句数(固定01)
    Option<int> total_msg;             

    // 当前语句号(固定01)
    Option<int> current_msg;           

    // 文本标识符(固定01)
    Option<int> text_id;               

    // 天线状态文本(OPEN/OK/SHORT)
    Option<StringView> status_text;   

};


}


}