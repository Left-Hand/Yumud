//这个驱动已经完成
//这个驱动已经测试

//FT6336是一款电容触摸屏芯片

#pragma once

#include "core/io/regs.hpp"
#include "core/utils/Result.hpp"
#include "core/utils/Errno.hpp"

#include "hal/bus/i2c/i2cdrv.hpp"

#include "types/vectors/vector2.hpp"

namespace ymd::drivers{

struct FT6336_Prelude{
public:
    static constexpr auto DEFAULT_I2C_ADDR = hal::I2cSlaveAddr<7>::from_u7(0x38);
    static constexpr auto MAX_I2C_BAUDRATE = 200000;
    static constexpr uint8_t PANEL_ID = 0x11;
    static constexpr size_t FT6336_MAX_POINTS_COUNT = 2;
    
    enum class Error_Kind:uint8_t{
        Unspecified,
        InvalidGestureId,
        PointsCountGreatThan2
    };

    DEF_FRIEND_DERIVE_DEBUG(Error_Kind)
    DEF_ERROR_SUMWITH_HALERROR(Error, Error_Kind)

    template<typename T = void>
    using IResult = Result<T, Error>;

    using RegAddress = uint8_t;



    static constexpr size_t CTP_MAX_TOUCH = 2;
    enum class DeviceMode:uint8_t{
        Working = 0b000,
        Factory = 0b100,
    } ;

    typedef enum {
        pollingMode = 0,
        triggerMode = 1,
    } G_MODE_Enum;


    typedef enum {
        keep_active_mode = 0,
        switch_to_monitor_mode = 1,
    } CTRL_MODE_Enum;

    enum class TouchStatus:uint8_t{
        Touch = 0,
        Stream,
        Release,
    } ;

    using Point = Vector2<uint16_t>;

    // struct PointWithStatus{
    //     TouchStatus status;
    //     uint16_t x;
    //     uint16_t y;
    // };


    struct Status_bit{
        uint8_t ptNum                    : 4;
        uint8_t tpDown                   : 1;
        uint8_t tpPress                  : 1;
        uint8_t res                      : 2;
    } ;


    struct GestureId{
        enum class Kind:uint8_t{
            Up = 0x10,
            Right = 0x14,
            Down = 0x18,
            Left = 0x1C,
            ZoomIn = 0x48,
            ZoomOut = 0x49,
        };

        DEF_FRIEND_DERIVE_DEBUG(Kind)

        using enum Kind;

        constexpr GestureId(Kind kind):kind_(std::bit_cast<uint8_t>(kind)) {;}
        constexpr GestureId(_None_t):kind_(0) {;}

        static constexpr IResult<GestureId> from_u8(const uint8_t raw){
            switch(raw){
                case k_None: return Ok(GestureId(None));
                case std::bit_cast<uint8_t>(Kind::Up):        return Ok(GestureId(Kind::Up));
                case std::bit_cast<uint8_t>(Kind::Right):     return Ok(GestureId(Kind::Right));
                case std::bit_cast<uint8_t>(Kind::Down):      return Ok(GestureId(Kind::Down));
                case std::bit_cast<uint8_t>(Kind::Left):      return Ok(GestureId(Kind::Left));
                case std::bit_cast<uint8_t>(Kind::ZoomIn):        return Ok(GestureId(Kind::ZoomIn));
                case std::bit_cast<uint8_t>(Kind::ZoomOut):       return Ok(GestureId(Kind::ZoomOut));
                default:    return Err(Error::InvalidGestureId);
            }
        }

        constexpr bool is_some() const { return kind_ != k_None; }
        constexpr bool is_none() const { return kind_ == k_None; }

        constexpr Kind unwrap() const { 
            if(is_none()) __builtin_trap();
            return std::bit_cast<Kind>(kind_);
        }

        friend OutputStream & operator<<(OutputStream & os, GestureId id){
            if(id.is_some()){
                return os << id.unwrap();
            }else{
                return os << "None";
            }
        }
    private:
        static constexpr uint8_t k_None = 0x00;
        uint8_t kind_;
    };

    struct Points{
        constexpr explicit Points():
            points_({Point{0,0}, Point{0,0}}),
            touch_count_(0)
            {;}

        constexpr explicit Points(Point p1):
            points_({p1, Point{0,0}}),
            touch_count_(1)
            {;}

        constexpr explicit Points(Point p1, Point p2):
            points_({p1, p2}),
            touch_count_(2)
            {;}


        constexpr uint8_t count() const{
            return touch_count_;
        }

        constexpr Option<Point> get(const size_t nth) const{
            if(nth >= touch_count_){
                return None;
            }
            return Some(points_[nth]);
        }

        // 将当前点映射到下一帧的点，寻找各自最近的位置重新映射
        constexpr Points map_to_next(const Points& next) const {
            auto manhattan_dist = [](const Point& p1, const Point& p2) {
                return std::abs(p1.x - p2.x) + std::abs(p1.y - p2.y);
            };

            // 无点情况处理
            if (touch_count_ == 0 || next.touch_count_ == 0) {
                return next;
            }

            // 获取最近邻映射
            auto find_nearest = [&](const Point& target, const Points& candidates) {
                size_t min_idx = 0;
                uint16_t min_dist = manhattan_dist(target, candidates.points_[0]);
                
                for (size_t i = 1; i < candidates.touch_count_; ++i) {
                    const auto dist = manhattan_dist(target, candidates.points_[i]);
                    if (dist < min_dist) {
                        min_dist = dist;
                        min_idx = i;
                    }
                }
                return candidates.points_[min_idx];
            };

            // 单点映射处理
            if (touch_count_ == 1) {
                if (next.touch_count_ == 1) {
                    return Points(next.points_[0]);
                }
                // 单点→双点：尝试恢复第二个点
                const Point nearest = find_nearest(points_[0], next);
                const Point second = (next.points_[0] == nearest) ? next.points_[1] : next.points_[0];
                return Points(nearest, second);
            }

            // 双点映射处理
            if (touch_count_ == 2) {
                if (next.touch_count_ == 1) {
                    // 双点→单点：两个点都映射到同一点
                    return Points(next.points_[0], next.points_[0]);
                }
                
                // 双点→双点：最优匹配
                const auto dist11 = manhattan_dist(points_[0], next.points_[0]);
                const auto dist12 = manhattan_dist(points_[0], next.points_[1]);
                const auto dist21 = manhattan_dist(points_[1], next.points_[0]);
                const auto dist22 = manhattan_dist(points_[1], next.points_[1]);

                return (dist11 + dist22 <= dist12 + dist21)
                    ? Points(next.points_[0], next.points_[1])
                    : Points(next.points_[1], next.points_[0]);
            }

            return next;
        }

        constexpr std::span<const Point> iter() const{
            return std::span(points_.data(), touch_count_);
        }
    private:
        std::array<Point,FT6336_MAX_POINTS_COUNT> points_;
        uint8_t touch_count_;
    };

    // static_assert(sizeof(Points) == 0x08);
};


struct FT6336_Regs:public FT6336_Prelude{

    struct R8_DevMode:public Reg8<>{
        static constexpr RegAddress ADDRESS = 0x00;
        uint8_t :4;
        DeviceMode device_mode:3;
        uint8_t :1;
    }DEF_R8(devmode)

    struct R8_GestId:public Reg8<>{
        static constexpr RegAddress ADDRESS = 0x01;
        uint8_t gesture_id;
    }DEF_R8(gestid)

    struct R8_TdStatus:public Reg8<>{
        static constexpr RegAddress ADDRESS = 0x02;
        uint8_t touch_cnt:4;
        uint8_t :4;
    }DEF_R8(td_status)

    struct R8_P1xh:public Reg8<>{
        static constexpr RegAddress ADDRESS = 0x03;
        uint8_t p1xh:4;
        uint8_t :2;
        uint8_t p1ev:2;
    }DEF_R8(p1xh)

    struct R8_P1xl:public Reg8<>{
        static constexpr RegAddress ADDRESS = 0x04;
        uint8_t p1xl;
    }DEF_R8(p1xl)

    struct R8_P1yh:public Reg8<>{
        static constexpr RegAddress ADDRESS = 0x05;
        uint8_t p1yh:4;
        uint8_t :2;
        uint8_t p1id:2;
    }DEF_R8(p1yh)

    struct R8_P1yl:public Reg8<>{
        static constexpr RegAddress ADDRESS = 0x06;
        uint8_t p1yl;
    }DEF_R8(p1yl)

    struct R8_P1Weight:public Reg8<>{
        static constexpr RegAddress ADDRESS = 0x07;
        uint8_t p1weight;
    }DEF_R8(p1weight)

    struct R8_P1Misc:public Reg8<>{
        static constexpr RegAddress ADDRESS = 0x08;
        uint8_t touch_area:4;
        uint8_t :4;
    }DEF_R8(p1misc)

    struct R8_P2xh:public Reg8<>{
        static constexpr RegAddress ADDRESS = 0x09;
        uint8_t p2xh:4;
        uint8_t :2;
        uint8_t p2ev:2;
    }DEF_R8(p2xh)

    struct R8_P2xl:public Reg8<>{
        static constexpr RegAddress ADDRESS = 0x0a;
        uint8_t p2xl;
    }DEF_R8(p2xl)

    struct R8_P2yh:public Reg8<>{
        static constexpr RegAddress ADDRESS = 0x0b;
        uint8_t p2yh:4;
        uint8_t :2;
        uint8_t p2id:2;
    }DEF_R8(p2yh)

    struct R8_P2yl:public Reg8<>{
        static constexpr RegAddress ADDRESS = 0x0c;
        uint8_t p2yl;
    }DEF_R8(p2yl)

    struct R8_P2Weight:public Reg8<>{
        static constexpr RegAddress ADDRESS = 0x0d;
        uint8_t p2weight;
    }DEF_R8(p2weight)

    struct R8_P2Misc:public Reg8<>{
        static constexpr RegAddress ADDRESS = 0x0e;
        uint8_t touch_area:4;
        uint8_t :4;
    }DEF_R8(p2misc)

    struct R8_ThGroup:public Reg8<>{
        static constexpr RegAddress ADDRESS = 0x80;
        uint8_t touch_threshold;
    }DEF_R8(th_group)

    struct R8_ThDiff:public Reg8<>{
        static constexpr RegAddress ADDRESS = 0x85;
        uint8_t touch_threshold;
    }DEF_R8(th_diff)

    struct R8_Ctrl:public Reg8<>{
        static constexpr RegAddress ADDRESS = 0x86;
        uint8_t enable_touch_detect:1;
        uint8_t :7;
    }DEF_R8(ctrl)

    struct R8_TimeEnterMonnitor:public Reg8<>{
        static constexpr RegAddress ADDRESS = 0x87;
        uint8_t time_enter_monitor;
    }DEF_R8(time_enter)

    struct R8_PeriodActive:public Reg8<>{
        static constexpr RegAddress ADDRESS = 0x88;
        uint8_t report_rate_when_active;
    }DEF_R8(period_active)

    struct R8_PeriodMonitor:public Reg8<>{
        static constexpr RegAddress ADDRESS = 0x89;
        uint8_t report_rate_when_monitor;
    }DEF_R8(period_monitor)

    struct R8_RadianValue:public Reg8<>{
        static constexpr RegAddress ADDRESS = 0x91;
        uint8_t minimal_allowed_angle;
    }DEF_R8(radian_value)

    struct R8_OffsetLeftReight:public Reg8<>{
        static constexpr RegAddress ADDRESS = 0x92;
        uint8_t maxmimul_offset;
    }DEF_R8(offset_left)
};

class FT6336U:public FT6336_Prelude{
public:
    explicit FT6336U(const hal::I2cDrv & i2c_drv):
        i2c_drv_(i2c_drv){;}
    explicit FT6336U(hal::I2cDrv && i2c_drv):
        i2c_drv_(std::move(i2c_drv)){;}
    explicit FT6336U(
        Some<hal::I2c *> i2c, 
        const hal::I2cSlaveAddr<7> addr = DEFAULT_I2C_ADDR
    ):
        i2c_drv_(hal::I2cDrv{i2c, addr}){;}

    [[nodiscard]] IResult<> init();


    [[nodiscard]] IResult<GestureId>    get_gesture_id();
    [[nodiscard]] IResult<DeviceMode>   get_device_mode();
    [[nodiscard]] IResult<>             set_device_mode(DeviceMode);
    [[nodiscard]] IResult<uint8_t>      get_touch_count(); 
    [[nodiscard]] IResult<Points>       get_touch_points(); 
    [[nodiscard]] IResult<Vector2<uint16_t>> get_touch1_position();
    [[nodiscard]] IResult<Vector2<uint16_t>> get_touch2_position();
    [[nodiscard]] IResult<uint8_t>      get_touch1_event();
    [[nodiscard]] IResult<uint8_t>      get_touch1_id();
    [[nodiscard]] IResult<uint8_t>      get_touch1_weight();
    [[nodiscard]] IResult<uint8_t>      get_touch1_misc();
    [[nodiscard]] IResult<uint8_t>      get_touch2_event();
    [[nodiscard]] IResult<uint8_t>      get_touch2_id();
    [[nodiscard]] IResult<uint8_t>      get_touch2_weight();
    [[nodiscard]] IResult<uint8_t>      get_touch2_misc();

    // Mode Parameter Register
    [[nodiscard]] IResult<uint8_t>  get_touch_threshold();
    [[nodiscard]] IResult<uint8_t>  get_filter_coefficient();
    [[nodiscard]] IResult<uint8_t>  get_ctrl_mode();
    [[nodiscard]] IResult<>         set_ctrl_mode(CTRL_MODE_Enum mode);
    [[nodiscard]] IResult<uint8_t>  get_time_period_enter_monitor();
    [[nodiscard]] IResult<uint8_t>  get_active_rate();
    [[nodiscard]] IResult<uint8_t>  get_monitor_rate();

    // Gestrue Parameter Register
    [[nodiscard]] IResult<uint8_t>  get_radian_value();
    [[nodiscard]] IResult<>         set_radian_value(uint8_t val);
    [[nodiscard]] IResult<uint8_t>  get_offset_left_right();
    [[nodiscard]] IResult<>         set_offset_left_right(uint8_t val);
    [[nodiscard]] IResult<uint8_t>  get_offset_up_down();
    [[nodiscard]] IResult<>         set_offset_up_down(uint8_t val);
    [[nodiscard]] IResult<uint8_t>  get_distance_left_right();
    [[nodiscard]] IResult<>         set_distance_left_right(uint8_t val);
    [[nodiscard]] IResult<uint8_t>  get_distance_up_down();
    [[nodiscard]] IResult<>         set_distance_up_down(uint8_t val);
    [[nodiscard]] IResult<uint8_t>  get_distance_zoom();
    [[nodiscard]] IResult<>         set_distance_zoom(uint8_t val);

    // System Information
    [[nodiscard]] IResult<uint16_t> get_library_version();
    [[nodiscard]] IResult<uint8_t>  get_chip_id();
    [[nodiscard]] IResult<uint8_t>  get_g_mode();
    [[nodiscard]] IResult<>         set_g_mode(G_MODE_Enum mode);
    [[nodiscard]] IResult<uint8_t>  get_pwrmode();
    [[nodiscard]] IResult<uint8_t>  get_firmware_id();
    [[nodiscard]] IResult<uint8_t>  get_focaltech_id();
    [[nodiscard]] IResult<uint8_t>  get_release_code_id();
    [[nodiscard]] IResult<uint8_t>  get_state();

private:
    hal::I2cDrv i2c_drv_;
    FT6336_Regs regs_;
    Points points_ = Points{};

    [[nodiscard]] IResult<> write_reg(const uint8_t addr, const uint8_t data);

    template<typename T>
    [[nodiscard]] IResult<> write_reg(const RegCopy<T> & reg){
        return write_reg(T::ADDRESS, reg.as_val());
    }

    [[nodiscard]] IResult<uint8_t> read_reg(const uint8_t addr);

    template<typename T>
    [[nodiscard]] IResult<> read_reg(T & reg){
        const auto res = read_reg(T::ADDRESS);
        if(res.is_err()) return Err(res.unwrap_err());
        reg.as_ref() = res.unwrap();
        return Ok();
    }

    [[nodiscard]] IResult<> read_burst(const uint8_t addr, std::span<uint8_t> pbuf){
        if(const auto res = i2c_drv_.read_burst(addr, pbuf);
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }

    [[nodiscard]] IResult<uint16_t> read_u12(const uint8_t addr){
        uint16_t ret;
        if(const auto res = read_burst_u12(addr, std::span(&ret, 1));
            res.is_err()) return Err(res.unwrap_err());
        return Ok(ret);
    }

    [[nodiscard]] IResult<> read_burst_u12(const uint8_t addr, std::span<uint16_t> pbuf){
        if(const auto res = i2c_drv_.read_burst(addr, pbuf, MSB);
            res.is_err()) return Err(res.unwrap_err());
        for(auto & item : pbuf){
            item = item & 0x0fff;
        }
        return Ok();
    }
};

}