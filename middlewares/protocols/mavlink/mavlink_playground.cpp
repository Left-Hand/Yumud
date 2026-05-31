#include <cstdint>

#include "mavlink_packed_code.hpp"
#include "core/string/view/string_view.hpp"
#include "core/string/view/mut_string_view.hpp"
#include "core/utils/Result.hpp"
#include "macro_utils.hpp"
#include <memory>

using namespace ymd;
using namespace ymd::mavlink;




namespace{


template<typename T>
struct _bits_type{
    using type = void;
};

template<typename T>
requires requires {
    T::bits;
}
struct _bits_type<T>{
    using type = decltype(T::bits);
};

template<typename T>
using bits_type_t = typename _bits_type<T>::type;


template<typename T>
struct MavBytesOp;


template<>
struct MavBytesOp<void>{
    static constexpr size_t ELEMENT_SIZE = 0;
};

template<>
struct MavBytesOp<uint8_t>{
    static constexpr size_t ELEMENT_SIZE = 1;
    static constexpr uint8_t load(const uint8_t * ptr){
        return static_cast<uint8_t>(*ptr);
    }

    static constexpr void store(uint8_t * ptr, const uint8_t obj){
        *ptr = static_cast<uint8_t>(obj);
    }
};

template<>
struct MavBytesOp<uint16_t>{
    static constexpr size_t ELEMENT_SIZE = 2;
    static constexpr uint16_t load(const uint8_t * ptr){
        uint16_t sum = 0;
        sum |= ptr[0];
        sum |= ptr[1] << 8;
        return sum;
    }

    static constexpr void store(uint8_t * ptr, const uint16_t obj){
        ptr[0] = static_cast<uint8_t>(obj);
        ptr[1] = static_cast<uint8_t>(obj >> 8);
    }
};

template<>
struct MavBytesOp<uint32_t>{
    static constexpr size_t ELEMENT_SIZE = 4;
    static constexpr uint32_t load(const uint8_t * ptr){
        uint32_t sum = 0;
        sum |= ptr[0];
        sum |= ptr[1] << 8;
        sum |= ptr[2] << 16;
        sum |= ptr[3] << 24;
        return sum;
    }

    static constexpr void store(uint8_t * ptr, const uint32_t obj){
        ptr[0] = static_cast<uint8_t>(obj);
        ptr[1] = static_cast<uint8_t>(obj >> 8);
        ptr[2] = static_cast<uint8_t>(obj >> 16);
        ptr[3] = static_cast<uint8_t>(obj >> 24);
    }
};

template<typename T>
requires(std::is_integral_v<T> and std::is_signed_v<T>)
struct MavBytesOp<T>{
    static constexpr size_t ELEMENT_SIZE = sizeof(T);
    using U = std::make_unsigned_t<T>;
    static constexpr T load(const uint8_t * ptr){
        return std::bit_cast<T>(MavBytesOp<U>::load(ptr));
    }

    static constexpr void store(uint8_t * ptr, const T obj){
        MavBytesOp<U>::store(ptr, std::bit_cast<U>(obj));
    }
};

template<typename T>
requires(not std::is_same_v<bits_type_t<T>, void>)
struct MavBytesOp<T>{
    using U = bits_type_t<T>;
    static constexpr size_t ELEMENT_SIZE = sizeof(U);
    static constexpr T load(const uint8_t * ptr){
        return std::bit_cast<T>(MavBytesOp<U>::load(ptr));
    }

    static constexpr void store(uint8_t * ptr, const T obj){
        MavBytesOp<U>::store(ptr, std::bit_cast<U>(obj));
    }
};

template<typename T>
requires(std::is_enum_v<T>)
struct MavBytesOp<T>{
    using D = std::underlying_type_t<T>;
    static constexpr size_t ELEMENT_SIZE = sizeof(D);

    static constexpr T load(const uint8_t * ptr){
        return std::bit_cast<T>(MavBytesOp<D>::load(ptr));
    }

    static constexpr void store(uint8_t * ptr, const T obj){
        MavBytesOp<D>::store(ptr, std::bit_cast<D>(obj));
    }
};


template<>
struct MavBytesOp<MavModeFlag>{
    static constexpr size_t ELEMENT_SIZE = 1;
    static constexpr MavModeFlag load(const uint8_t * ptr){
        return static_cast<MavModeFlag>(*ptr);
    }

    static constexpr void store(uint8_t * ptr, const MavModeFlag obj){
        *ptr = static_cast<uint8_t>(obj.bits);
    }
};



template<>
struct MavBytesOp<math::fp32>{
    static constexpr size_t ELEMENT_SIZE = 4;
    static constexpr fp32 load(const uint8_t * ptr){
        return fp32::from_bits(MavBytesOp<uint32_t>::load(ptr));
    }

    static constexpr void store(uint8_t * ptr, const math::fp32 obj){
        MavBytesOp<uint32_t>::store(ptr, obj.to_bits());
    }
};


template<size_t N>
struct MavBytesOp<char[N]>{
    static constexpr size_t ELEMENT_SIZE = N;
    static constexpr void store(uint8_t * ptr,const char * ntstr){
        size_t i = 0;
        for(; i < N; i++){
            ptr[i] = ntstr[i];
            if(ntstr[i] == 0) break;
        }

        for(; i<N; i++){
            ptr[i] = 0;
        }
    }
};


template<typename T>
static constexpr bool is_cstyle_array_v = false;

template<typename T, std::size_t N>
static constexpr bool is_cstyle_array_v<T[N]> = true;

template<typename T, std::size_t N>
static constexpr bool is_cstyle_array_v<const T[N]> = true;



template<typename T>
static constexpr bool is_cstyle_char_array_v = false;

template<std::size_t N>
static constexpr bool is_cstyle_char_array_v<char[N]> = true;

template<std::size_t N>
static constexpr bool is_cstyle_char_array_v<const char[N]> = true;

template<typename T>
struct _cstyle_array_size;

template<typename T, std::size_t N>
struct _cstyle_array_size<T[N]> : std::integral_constant<std::size_t, N> {};

template<typename T, std::size_t N>
struct _cstyle_array_size<const T[N]> : std::integral_constant<std::size_t, N> {};

template<typename T>
static constexpr std::size_t cstyle_array_size_v = 
    _cstyle_array_size<T>::value;



static_assert(is_cstyle_char_array_v<char[8]> == true);
static_assert(is_cstyle_char_array_v<const char[8]> == true);
static_assert(is_cstyle_char_array_v<uint8_t[8]> == false);

template<typename T>
struct MemberBytesProxy{
    using ptr_type = std::conditional_t<std::is_const_v<T>, const uint8_t *, uint8_t *>;
    using U = std::remove_const_t<T>;



    ptr_type ptr;

    constexpr U get() const {
        return MavBytesOp<U>::load(ptr);
    }

    constexpr void set(U obj) requires(not std::is_const_v<T>) {
        return MavBytesOp<U>::store(ptr, obj);
    }
};

template<typename T>
requires(is_cstyle_char_array_v<T>)
struct MemberBytesProxy<T>{
    static_assert(not std::is_pointer_v<T>);
    using ptr_type = std::conditional_t<std::is_const_v<T>, const uint8_t *, uint8_t *>;
    using U = std::remove_const_t<T>;
    static_assert(not std::is_pointer_v<U>);
    static constexpr size_t CAPACITY = cstyle_array_size_v<U>;

    ptr_type ptr;

    StringView get() const {
        return StringView::from_zero_terminated(
            reinterpret_cast<const char *>(ptr),
            CAPACITY
        );
    }

    constexpr size_t length() const {
        size_t i = 0;
        for(; i < CAPACITY; i++){
            if(ptr[i] == 0) break;
        }
        return i;
    }

    consteval size_t capacity() const {
        return CAPACITY;
    }

    constexpr uint8_t operator[](const size_t idx) const {
        if(idx > CAPACITY) __builtin_trap();
        return static_cast<uint8_t>(ptr[idx]);
    }

    constexpr uint8_t & operator[](const size_t idx) requires(not std::is_const_v<T>) {
        if(idx > CAPACITY) __builtin_trap();
        return (ptr[idx]);
    }

    constexpr Result<void, void> set(const StringView str) requires(not std::is_const_v<T>) {
        if(str.length() > CAPACITY) return Err();
        for(size_t i = 0; i < str.length(); i++){
            ptr[i] = static_cast<uint8_t>(str[i]);
        }
    }
};


template<typename R, typename B>
static constexpr auto make_member_bytes_proxy(B ptr){
    static_assert(not std::is_pointer_v<R>);
    using T = std::conditional_t<std::is_const_v<std::remove_pointer_t<B>>, 
        const R, R>;
    return MemberBytesProxy<T>(ptr);
}



template<typename T>
requires(is_cstyle_array_v<T> and (not is_cstyle_char_array_v<T>))
struct MemberBytesProxy<T>{
    using ptr_type = std::conditional_t<std::is_const_v<T>, const uint8_t *, uint8_t *>;
    using U = std::remove_const_t<T>;

    static constexpr size_t CAPACITY = cstyle_array_size_v<U>;

    using element_type = std::remove_all_extents_t<U>;
    using maymut_element_type = std::conditional_t<std::is_const_v<T>, const element_type, element_type>;

    static constexpr size_t ELEMENT_SIZE = MavBytesOp<element_type>::ELEMENT_SIZE;

    ptr_type ptr;

    consteval size_t capacity() const {
        return CAPACITY;
    }

    consteval size_t size() const {
        return CAPACITY;
    }

    template<typename Self>
    constexpr auto operator[](this Self && self, const size_t idx) {
        if(idx > CAPACITY) __builtin_trap();
        
        return make_member_bytes_proxy<maymut_element_type>(&self.ptr[idx * ELEMENT_SIZE]);
    }
};

template <size_t N, typename Tup>
struct _nth_offset{
    static constexpr size_t value = 
        _nth_offset<N - 1, Tup>::value + MavBytesOp<std::tuple_element_t<N - 1, Tup>>::ELEMENT_SIZE;
};


template<typename Tup>
struct _nth_offset<0, Tup>{
    static constexpr size_t value = 0;
};


#define _MAVREF_ENUM(_method_tuple_) \
_TRAIT_METHOD_PICK_NAME(_method_tuple_),


#define _MAVREF_TYPETUPLE(_method_tuple_) \
_TRAIT_METHOD_PICK_RET(_method_tuple_),

#define _MAVREF_MPROXYS(_method_tuple_) \
template<typename Self>\
constexpr auto _TRAIT_METHOD_PICK_NAME(_method_tuple_) (this Self && self){\
    constexpr size_t OFFSET = nth_offset_v<static_cast<size_t>(ElementRank::_TRAIT_METHOD_PICK_NAME(_method_tuple_))>;\
    return make_member_bytes_proxy<_TRAIT_METHOD_PICK_RET(_method_tuple_)>(&self.data[OFFSET]);\
}


#define _MAVREF_STRUCT_BASE(_NAME, NUM_MSG_ID, ...) \
template<typename Storage>\
struct [[nodiscard]] _NAME##_Facade final{\
    static constexpr MavMessageId MSG_ID = MavMessageId{NUM_MSG_ID};\
    using members_tuple = std::tuple<MACRO_MAP(_MAVREF_TYPETUPLE, __VA_ARGS__) void>;\
    enum class ElementRank:int{MACRO_MAP(_MAVREF_ENUM, __VA_ARGS__)};\
    template<size_t N> static constexpr size_t nth_offset_v = _nth_offset<N, members_tuple>::value;\
    Storage data;\
    MACRO_MAP(_MAVREF_MPROXYS, __VA_ARGS__) \
};

#define MAVREF_DEFMSG(_NAME, NUM_MSG_ID, ...) _MAVREF_STRUCT_BASE(_NAME, NUM_MSG_ID, __VA_ARGS__)



#if 1


using math::fp32;

MAVREF_DEFMSG(
    Heartbeat, 0,
    MAVREF_PROXY(type,              MavType),
    MAVREF_PROXY(autopilot,         MavAutopilot),
    MAVREF_PROXY(base_mode,         MavModeFlag),
    MAVREF_PROXY(custom_mode,       uint32_t),
    MAVREF_PROXY(system_status,     MavState )
)


// SYS_STATUS (1)
// 系统核心状态（传感器、电池、负载、通信）。
MAVREF_DEFMSG(
    SysStatus, 1,

    // 传感器在位掩码（0=不存在，1=存在）
    MAVREF_PROXY(onboard_control_sensors_present, MavSysStatusSensor),
    
    // 传感器使能掩码（0=关闭，1=开启）
    MAVREF_PROXY(onboard_control_sensors_enabled, MavSysStatusSensor),

    // 传感器健康掩码（0=故障，1=正常）
    MAVREF_PROXY(onboard_control_sensors_health, MavSysStatusSensor),

    // 主循环最大占用率（0–1000，建议<1000）
    MAVREF_PROXY(load, LoadPercents),

    // 电池电压
    MAVREF_PROXY(voltage_battery, VoltageCode),

    // 电池电流
    MAVREF_PROXY(current_battery, CurrentCode),

    // 剩余电量百分比
    MAVREF_PROXY(battery_remaining, int8_t),

    // 通信丢包率（UART/I2C/SPI/CAN）
    MAVREF_PROXY(drop_rate_comm, uint16_t),

    // 通信错误数
    MAVREF_PROXY(errors_comm, uint16_t),

    // 飞控自定义错误码1
    MAVREF_PROXY(errors_count, uint16_t[4]),

    // 扩展传感器在位掩码
    MAVREF_PROXY(onboard_control_sensors_present_extended, uint32_t),

    // 扩展传感器使能掩码
    MAVREF_PROXY(onboard_control_sensors_enabled_extended, uint32_t),

    // 扩展传感器健康掩码
    MAVREF_PROXY(onboard_control_sensors_health_extended, uint32_t)

)

MAVREF_DEFMSG(
    ParamSet, 23,
    MAVREF_PROXY(target_system,         uint8_t),
    MAVREF_PROXY(target_component,      uint8_t),
    MAVREF_PROXY(param_id,              char[16]),
    MAVREF_PROXY(param_value,           fp32),
    MAVREF_PROXY(param_type,            MavParamType)
)

#else
template<typename Storage>
struct [[nodiscard]] Heartbeat_Facade final{
    static constexpr MavMessageId MSG_ID = MavMessageId{0};

    using members_tuple = std::tuple<MavType, MavAutopilot, MavModeFlag, uint32_t>;

    template<size_t N>
    static constexpr size_t nth_offset_v = _nth_offset<N, members_tuple>::value;

    Storage data;

    enum class ElementRank:int{
        type,
        autopilot,
        base_mode,
        custom_mode,
        system_status
    };
    

    template<typename Self>
    constexpr auto type(this Self && self){
        constexpr size_t OFFSET = nth_offset_v<static_cast<size_t>(ElementRank::type)>;
        return make_member_bytes_proxy<MavType>(&self.data[OFFSET]);
    }

    template<typename Self>
    constexpr auto autopilot(this Self && self){
        constexpr size_t OFFSET = nth_offset_v<static_cast<size_t>(ElementRank::autopilot)>;
        return make_member_bytes_proxy<MavAutopilot>(&self.data[OFFSET]);
    }
    
    template<typename Self>
    constexpr auto base_mode(this Self && self){
        constexpr size_t OFFSET = nth_offset_v<static_cast<size_t>(ElementRank::base_mode)>;
        return make_member_bytes_proxy<MavModeFlag>(&self.data[OFFSET]);
    }

    template<typename Self>
    constexpr auto custom_mode(this Self && self){
        constexpr size_t OFFSET = nth_offset_v<static_cast<size_t>(ElementRank::custom_mode)>;
        return make_member_bytes_proxy<uint32_t>(&self.data[OFFSET]);
    }

    template<typename Self>
    constexpr auto system_status(this Self && self){
        constexpr size_t OFFSET = nth_offset_v<static_cast<size_t>(ElementRank::system_status)>;
        return make_member_bytes_proxy<MavState>(&self.data[OFFSET]);
    }
};
#endif
[[maybe_unused]] static void test_pg(){
    {
        static constexpr std::array<uint8_t, 8> buf = {
            static_cast<uint8_t>(MavType::Adsb),
            static_cast<uint8_t>(MavAutopilot::Argentum),
            0x11,
            0x12,0x34,0x56,0x78,
            static_cast<uint8_t>(MavState::Active)
        };


        static constexpr auto it = Heartbeat_Facade<std::span<const uint8_t, 8>>
            {std::span<const uint8_t, 8>(buf)};

        static_assert(it.type().get() == MavType::Adsb);
        static_assert(it.autopilot().get() == MavAutopilot::Argentum);
        static_assert(it.base_mode().get().armed().get() == true);
        static_assert(it.base_mode().get().guided_enabled().get() == true);
        static_assert(it.custom_mode().get() == 0x78563412);
        static_assert(it.system_status().get() == MavState::Active);
    }

    {

        // static constexpr auto it = ParamSet_Facade<std::span<const uint8_t, 23>>
        //     {std::span<const uint8_t, 23>(buf)};

        static constexpr auto it = []{
            ParamSet_Facade<std::array<uint8_t, 23>> temp{};
            temp.target_system().set(0x14);
            temp.target_component().set(0x51);
            temp.param_value().set(1.73782444e+34f);
            temp.param_id()[0] = 'H';
            temp.param_id()[1] = 'e';
            temp.param_id()[2] = 'l';
            temp.param_id()[3] = 'l';
            temp.param_id()[4] = 'o';
            temp.param_id()[5] = '\0';
            return temp;
        }();

        static_assert(it.target_system().get() == 0x14);
        static_assert(it.target_component().get() == 0x51);
        static_assert((float)it.param_value().get() == 1.73782444e+34f);
        static_assert(it.param_id().length() == 5);
        static_assert(it.param_id()[0] == 'H');
        static_assert(it.param_id()[4] == 'o');
    }


    {
        static constexpr auto it = []{
            SysStatus_Facade<std::array<uint8_t, 80>> temp{};
            temp.onboard_control_sensors_present().set([]{
                auto s = MavSysStatusSensor{0};
                s.sensor_3d_gyro().set(1);
                s.sensor_gps().set(1);
                return s;
            }());


            temp.load().set(LoadPercents{42});

            temp.errors_count()[0].set(0x12);
            temp.errors_count()[1].set(0x34);
            temp.errors_count()[2].set(0x56);
            temp.errors_count()[3].set(0x78);
            return temp;
        }();

        constexpr auto onboard_control_sensors_present = it.onboard_control_sensors_present().get();


        static_assert(onboard_control_sensors_present.sensor_3d_gyro().get() == true);
        static_assert(onboard_control_sensors_present.sensor_absolute_pressure().get() == false);
        static_assert(onboard_control_sensors_present.sensor_gps().get() == true);

        static_assert(it.load().get().bits == 42);
        static_assert(it.errors_count()[0].get() == 0x12);
        static_assert(it.errors_count()[1].get() == 0x34);
        static_assert(it.errors_count()[2].get() == 0x56);
        static_assert(it.errors_count()[3].get() == 0x78);
    }

    {
        static constexpr auto it = []{
            Heartbeat_Facade<std::array<uint8_t, 8>> temp{};
            temp.type().set(MavType::Adsb);
            temp.autopilot().set(MavAutopilot::Argentum);
            temp.custom_mode().set(0x78563412);
            temp.system_status().set(MavState::Active);
            return temp;
        }();

        static_assert(it.type().get() == MavType::Adsb);
        static_assert(it.autopilot().get() == MavAutopilot::Argentum);
        static_assert(it.custom_mode().get() == 0x78563412);
        static_assert(it.system_status().get() == MavState::Active);
    }
}
#if 0

#endif

}