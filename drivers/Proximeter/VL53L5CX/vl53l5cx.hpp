//这个驱动还未完成
//这个驱动还未测试

//参考教程：
// https://blog.csdn.net/qq_24312945/article/details/133848222
// https://blog.csdn.net/qq_24312945/article/details/133722644

//VL53L5X 是意法半导体的一款具有宽视野的飞行时间 (ToF) 8x8多区测距传感器

// VL53L5CX是意法半导体FlightSense产品系列中最先进的飞行时间 (ToF) 多区测距传感器。
// 该产品采用微型可回流焊封装，集成了SPAD阵列、物理红外滤光片和衍射光学元件 (DOE)，
// 确保在各种环境照明条件下，搭配不同类型的盖片材料，均能够实现卓越的测距性能。

// 通过在垂直腔面发射激光器 (VCSEL) 上方使用DOE，将一个方形FoV投影到场景上。反射光随后由接收器透镜聚焦到SPAD阵列上。

// 不同于传统的IR传感器，VL53L5CX利用意法半导体最新一代ToF技术，能够测量目标的绝对距离，
// 而不受目标颜色或反射率的影响。该产品可提供高达400 cm的精确测距，并能以极快的速度 
// (60 Hz) 运行，是目前市场上速度最快的多区微型ToF传感器之一。

// 多区测距最多可覆盖8x8个区域，对角线视野可达65°（可通过软件调整）

// 得益于意法半导体的专利型直方图算法，VL53L5CX能够检测并区分FoV内的多个目标。
// 该算法还能有效抵御60 cm以上的盖片串扰。

#pragma once

#include "details/vl53l5cx_prelude.hpp"
#include "details/vl53l5cx_buffers.hpp"

namespace ymd::drivers{

class VL53L5CX final:public VL53L5CX_Prelude{ 
public:
    enum class ErrorKind:uint8_t { 
        TimeOut = 1,
        McuError = 66,
        InvalidParam = 127,
        InvalidDeviceId,
        InvalidRevisionId,
        Status = 255,
        
    };

    DEF_ERROR_SUMWITH_HALERROR(Error, ErrorKind)

    template<typename T = void>
    using IResult = Result<T, Error>;

    IResult<> init();

    IResult<> send_offset_data(Resolution resolution);


    IResult<> validate();

    /**
     * @brief Mandatory function used to initialize the sensor. This function must
     * be called after a power on, to load the firmware into the VL53L5CX. It takes
     * a few hundred milliseconds.
     * @return (uint8_t) status : 0 if initialization is OK.
     */


    /**
     * @brief This function is used to change the I2C address of the sensor. If
     * multiple VL53L5 sensors are connected to the same I2C line, all other LPn
     * pins needs to be set to Low. The default sensor address is 0x52.
     * @param (uint16_t) i2c_address : New I2C address.
     * @return (uint8_t) status : 0 if new address is OK
     */

    IResult<> set_i2c_address(hal::I2cSlaveAddr<7> i2c_addr);

    /**
     * @brief This function is used to get the current sensor power mode.
     * @param (uint8_t) *p_power_mode : Current power mode. The value of this
     * pointer is equal to 0 if the sensor is in low power,
     * (VL53L5CX_POWER_MODE_SLEEP), or 1 if sensor is in standard mode
     * (VL53L5CX_POWER_MODE_WAKEUP).
     * @return (uint8_t) status : 0 if power mode is OK
     */

    IResult<PowerMode> get_power_mode();

    /**
     * @brief This function is used to set the sensor in Low Power mode, for
     * example if the sensor is not used during a long time. The macro
     * VL53L5CX_POWER_MODE_SLEEP can be used to enable the low power mode. When user
     * want to restart the sensor, he can use macro VL53L5CX_POWER_MODE_WAKEUP.
     * Please ensure that the device is not streaming before calling the function.
     * @param (uint8_t) power_mode : Selected power mode (VL53L5CX_POWER_MODE_SLEEP
     * or VL53L5CX_POWER_MODE_WAKEUP)
     * @return (uint8_t) status : 0 if power mode is OK, or 127 if power mode
     * requested by user is not valid.
     */

    IResult<> set_power_mode(PowerMode power_mode);

    /**
     * @brief This function starts a ranging session. When the sensor streams, host
     * cannot change settings 'on-the-fly'.
     * @return (uint8_t) status : 0 if start is OK.
     */

    IResult<> start_ranging();

    /**
     * @brief This function stops the ranging session. It must be used when the
     * sensor streams, after calling vl53l5cx_start_ranging().
     * @return (uint8_t) status : 0 if stop is OK
     */

    IResult<> stop_ranging();

    //This function checks if a new data is ready by polling I2C. If a new
    //data is ready, a flag will be raised.
    IResult<bool> is_data_ready();

    /**
     * @brief This function gets the ranging data, using the selected output and the
     * resolution.
     * @param (VL53L5CX_Frame) *p_results : VL53L5 results structure.
     * @return (uint8_t) status : 0 data are successfully get.
     */

    IResult<> get_ranging_data(VL53L5CX_Frame		*p_results);

    /**
     * @brief This function gets the current resolution (4x4 or 8x8).
     * @param (uint8_t) *p_resolution : Value of this pointer will be equal to 16
     * for 4x4 mode, and 64 for 8x8 mode.
     * @return (uint8_t) status : 0 if resolution is OK.
     */

    IResult<Resolution> get_resolution();

    /**
     * @brief This function sets a new resolution (4x4 or 8x8).
     * @param (uint8_t) resolution : Use macro VL53L5CX_RESOLUTION_4X4 or
     * VL53L5CX_RESOLUTION_8X8 to set the resolution.
     * @return (uint8_t) status : 0 if set resolution is OK.
     */

    IResult<> set_resolution(Resolution resolution);

    /**
     * @brief This function gets the current ranging frequency in Hz. Ranging
     * frequency corresponds to the time between each measurement.
     * @param (uint8_t) *p_frequency_hz: Contains the ranging frequency in Hz.
     * @return (uint8_t) status : 0 if ranging frequency is OK.
     */

    IResult<> get_ranging_frequency_hz(
            uint8_t				*p_frequency_hz);

    /**
     * @brief This function sets a new ranging frequency in Hz. Ranging frequency
     * corresponds to the measurements frequency. This setting depends of
     * the resolution, so please select your resolution before using this function.
     * @param (uint8_t) frequency_hz : Contains the ranging frequency in Hz.
     * - For 4x4, min and max allowed values are : [1;60]
     * - For 8x8, min and max allowed values are : [1;15]
     * @return (uint8_t) status : 0 if ranging frequency is OK, or 127 if the value
     * is not correct.
     */

    IResult<> set_ranging_frequency_hz(
            uint8_t				frequency_hz);

    /**
     * @brief This function gets the current integration time in ms.
     * @param (uint32_t) *p_time_ms: Contains integration time in ms.
     * @return (uint8_t) status : 0 if integration time is OK.
     */

    IResult<> get_integration_time_ms(
            uint32_t			*p_time_ms);

    /**
     * @brief This function sets a new integration time in ms. Integration time must
     * be computed to be lower than the ranging period, for a selected resolution.
     * Please note that this function has no impact on ranging mode continous.
     * @param (uint32_t) time_ms : Contains the integration time in ms. For all
     * resolutions and frequency, the minimum value is 2ms, and the maximum is
     * 1000ms.
     * @return (uint8_t) status : 0 if set integration time is OK.
     */

    IResult<> set_integration_time_ms(uint32_t integration_time_ms);

    /**
     * @brief This function gets the current sharpener in percent. Sharpener can be
     * changed to blur more or less zones depending of the application.
     * @param (uint32_t) *p_sharpener_percent: Contains the sharpener in percent.
     * @return (uint8_t) status : 0 if get sharpener is OK.
     */

    IResult<> get_sharpener_percent(
            uint8_t				*p_sharpener_percent);

    /**
     * @brief This function sets a new sharpener value in percent. Sharpener can be
     * changed to blur more or less zones depending of the application. Min value is
     * 0 (disabled), and max is 99.
     * @param (uint32_t) sharpener_percent : Value between 0 (disabled) and 99%.
     * @return (uint8_t) status : 0 if set sharpener is OK.
     */

    IResult<> set_sharpener_percent(
            uint8_t				sharpener_percent);

    /**
     * @brief This function gets the current target order (closest or strongest).
     * @param (uint8_t) *p_target_order: Contains the target order.
     * @return (uint8_t) status : 0 if get target order is OK.
     */

    IResult<TargetOrder> get_target_order();

    /**
     * @brief This function sets a new target order. Please use macros
     * VL53L5CX_TARGET_ORDER_STRONGEST and VL53L5CX_TARGET_ORDER_CLOSEST to define
     * the new output order. By default, the sensor is configured with the strongest
     * output.
     * @param (uint8_t) target_order : Required target order.
     * @return (uint8_t) status : 0 if set target order is OK, or 127 if target
     * order is unknown.
     */

    IResult<> set_target_order(TargetOrder target_order);

    /**
     * @brief This function is used to get the ranging mode. Two modes are
     * available using ULD : Continuous and autonomous. The default
     * mode is Autonomous.
     * @param (uint8_t) *p_ranging_mode : current ranging mode
     * @return (uint8_t) status : 0 if get ranging mode is OK.
     */

    IResult<RangingMode> get_ranging_mode();

    /**
     * @brief This function is used to set the ranging mode. Two modes are
     * available using ULD : Continuous and autonomous. The default
     * mode is Autonomous.
     * @param (uint8_t) ranging_mode : Use macros VL53L5CX_RANGING_MODE_CONTINUOUS,
     * VL53L5CX_RANGING_MODE_CONTINUOUS.
     * @return (uint8_t) status : 0 if set ranging mode is OK.
     */

    IResult<> set_ranging_mode(RangingMode ranging_mode);

    /**
     * @brief This function can be used to read 'extra data' from DCI. Using a known
     * index, the function fills the casted structure passed in argument.
     * @param (uint8_t) *data : This field can be a casted structure, or a simple
     * array. Please note that the FW only accept data of 32 bits. So field data can
     * only have a size of 32, 64, 96, 128, bits ....
     * @param (uint32_t) index : Index of required value.
     * @param (uint16_t)*data_size : This field must be the structure or array size
     * (using sizeof() function).
     * @return (uint8_t) status : 0 if OK
     */

    IResult<> dci_read_data(
            uint8_t				*data,
            uint32_t			index,
            uint16_t			data_size);

    /**
     * @brief This function can be used to write 'extra data' to DCI. The data can
     * be simple data, or casted structure.
     * @param (uint8_t) *data : This field can be a casted structure, or a simple
     * array. Please note that the FW only accept data of 32 bits. So field data can
     * only have a size of 32, 64, 96, 128, bits ..
     * @param (uint32_t) index : Index of required value.
     * @param (uint16_t)*data_size : This field must be the structure or array size
     * (using sizeof() function).
     * @return (uint8_t) status : 0 if OK
     */

    IResult<> dci_write_data(
            const uint8_t				*data,
            uint32_t			index,
            uint16_t			data_size);

    /**
     * @brief This function can be used to replace 'extra data' in DCI. The data can
     * be simple data, or casted structure.
     * @param (uint8_t) *data : This field can be a casted structure, or a simple
     * array. Please note that the FW only accept data of 32 bits. So field data can
     * only have a size of 32, 64, 96, 128, bits ..
     * @param (uint32_t) index : Index of required value.
     * @param (uint16_t)*data_size : This field must be the structure or array size
     * (using sizeof() function).
     * @param (uint8_t) *new_data : Contains the new fields.
     * @param (uint16_t) new_data_size : New data size.
     * @param (uint16_t) new_data_pos : New data position into the buffer.
     * @return (uint8_t) status : 0 if OK
     */

    IResult<> dci_replace_data(
            uint8_t				*data,
            uint32_t			index,
            uint16_t			data_size,
            const uint8_t				*new_data,
            uint16_t			new_data_size,
            uint16_t			new_data_pos);


    IResult<Enable> get_detection_thresholds_enable();

    /**
     * @brief This function allows enable the detection thresholds.
     * @param (uint8_t) enabled : Set to 1 to enable, or 0 to disable thresholds.
     * @return (uint8_t) status : 0 if programming is OK
     */

    IResult<> set_detection_thresholds_enable(Enable en);

    /**
     * @brief This function allows getting the detection thresholds.
     * @param (VL53L5CX_DetectionThresholds) *p_thresholds : Array of 64 thresholds.
     * @return (uint8_t) status : 0 if programming is OK
     */

    IResult<> get_detection_thresholds(
        const std::span<VL53L5CX_DetectionThresholds, VL53L5CX_NB_THRESHOLDS> p_thresholds);

    /**
     * @brief This function allows programming the detection thresholds.
     * @param (VL53L5CX_DetectionThresholds) *p_thresholds :  Array of 64 thresholds.
     * @return (uint8_t) status : 0 if programming is OK
     */

    IResult<> set_detection_thresholds(
        const std::span<const VL53L5CX_DetectionThresholds, VL53L5CX_NB_THRESHOLDS> p_thresholds);



    /**
     * @brief This function is used to initialized the motion indicator. By default,
     * indicator is programmed to monitor movements between 400mm and 1500mm.
     * @param (VL53L5CX_Motion_Configuration) *p_motion_config : Structure
     * containing the initialized motion configuration.
     * @param (uint8_t) resolution : Wanted resolution, defined by macros
     * VL53L5CX_RESOLUTION_4X4 or VL53L5CX_RESOLUTION_8X8.
     * @return (uint8_t) status : 0 if OK, or 127 is the resolution is unknown.
     */

    IResult<> motion_indicator_init(
        VL53L5CX_Motion_Configuration & motion_config,
        Resolution resolution);

    /**
     * @brief This function can be used to change the working distance of motion
     * indicator. By default, indicator is programmed to monitor movements between
     * 400mm and 1500mm.
     * @param (VL53L5CX_Motion_Configuration) *p_motion_config : Structure
     * containing the initialized motion configuration.
     * @param (uint16_t) distance_min_mm : Minimum distance for indicator (min value 
     * 400mm, max 4000mm).
     * @param (uint16_t) distance_max_mm : Maximum distance for indicator (min value 
     * 400mm, max 4000mm).
     * VL53L5CX_RESOLUTION_4X4 or VL53L5CX_RESOLUTION_8X8.
     * @return (uint8_t) status : 0 if OK, or 127 if an argument is invalid.
     */

    IResult<> motion_indicator_set_distance_motion(
        VL53L5CX_Motion_Configuration & motion_config,
        uint16_t			distance_min_mm,
        uint16_t			distance_max_mm);

    /**
     * @brief This function is used to update the internal motion indicator map.
     * @param (VL53L5CX_Motion_Configuration) *p_motion_config : Structure
     * containing the initialized motion configuration.
     * @param (uint8_t) resolution : Wanted SCI resolution, defined by macros
     * VL53L5CX_RESOLUTION_4X4 or VL53L5CX_RESOLUTION_8X8.
     * @return (uint8_t) status : 0 if OK, or 127 is the resolution is unknown.
     */

    IResult<> motion_indicator_set_resolution(
        VL53L5CX_Motion_Configuration & motion_config,
        Resolution resolution
    );


    /**
     * @brief This function starts the VL53L5CX sensor in order to calibrate Xtalk.
     * This calibration is recommended is user wants to use a coverglass.
     * @param (uint16_t) reflectance_percent : Target reflectance in percent. This
     * value is include between 1 and 99%. For a better efficiency, ST recommends a
     * 3% target reflectance.
     * @param (uint8_t) nb_samples : Nb of samples used for calibration. A higher
     * number of samples means a higher accuracy, but it increases the calibration
     * time. Minimum is 1 and maximum is 16.
     * @param (uint16_t) distance_mm : Target distance in mm. The minimum allowed
     * distance is 600mm, and maximum is 3000mm. The target must stay in Full FOV,
     * so short distance are easier for calibration.
     * @return (uint8_t) status : 0 if calibration OK, 127 if an argument has an
     * incorrect value, or 255 is something failed.
     */

    IResult<> calibrate_xtalk(
        uint16_t			reflectance_percent,
        uint8_t				nb_samples,
        uint16_t			distance_mm);

    /**
     * @brief This function gets the Xtalk buffer. The buffer is available after
     * using the function vl53l5cx_calibrate_xtalk().
     * @param (VL53L5CX_Configuration) *p_dev : VL53L5 configuration structure.
     * @param (uint8_t) *p_xtalk_data : Buffer with a size defined by
     * macro VL53L5CX_XTALK_SIZE.
     * @return (uint8_t) status : 0 if buffer reading OK
     */

    IResult<> get_caldata_xtalk(
        uint8_t				*p_xtalk_data);

    /**
     * @brief This function sets the Xtalk buffer. This function can be used to
     * override default Xtalk buffer.
     * @param (VL53L5CX_Configuration) *p_dev : VL53L5 configuration structure.
     * @param (uint8_t) *p_xtalk_data : Buffer with a size defined by
     * macro VL53L5CX_XTALK_SIZE.
     * @return (uint8_t) status : 0 if buffer OK
     */

    IResult<> set_caldata_xtalk(
        uint8_t				*p_xtalk_data);

    /**
     * @brief This function gets the Xtalk margin. This margin is used to increase
     * the Xtalk threshold. It can also be used to avoid false positives after the
     * Xtalk calibration. The default value is 50 kcps/spads.
     * @param (uint32_t) *p_xtalk_margin : Xtalk margin in kcps/spads.
     * @return (uint8_t) status : 0 if reading OK
     */

    IResult<> get_xtalk_margin(
        uint32_t			*p_xtalk_margin);

    /**
     * @brief This function sets the Xtalk margin. This margin is used to increase
     * the Xtalk threshold. It can also be used to avoid false positives after the
     * Xtalk calibration. The default value is 50 kcps/spads.
     * @param (uint32_t) xtalk_margin : New Xtalk margin in kcps/spads. Min value is
     * 0 kcps/spads, and max is 10.000 kcps/spads
     * @return (uint8_t) status : 0 if set margin is OK, or 127 is the margin is
     * invalid.
     */

    IResult<> set_xtalk_margin(
        uint32_t			xtalk_margin);

    IResult<> program_output_config();

    IResult<> send_xtalk_data(Resolution resolution);
        
private:
	/* Results stream_count_, value auto-incremented at each range */
	uint8_t		        stream_count_;
	/* Size of data read though I2C */
	uint32_t	        data_read_size_;

	/* Offset buffer */
	uint8_t		        offset_data[VL53L5CX_OFFSET_BUFFER_SIZE];
	/* Xtalk buffer */
	uint8_t		        xtalk_data[VL53L5CX_XTALK_BUFFER_SIZE];
	/* Temporary buffer used for internal driver processing */
    uint8_t	        temp_buffer[VL53L5CX_TEMPORARY_BUFFER_SIZE];

    IResult<> read_byte(const uint16_t addr, uint8_t *data);

    IResult<> write_byte(const uint16_t addr, uint8_t data);

    IResult<> read_burst(const uint16_t addr, uint8_t *data, uint16_t size);

    IResult<> write_burst(const uint16_t addr, const uint8_t *data, uint16_t size);

    IResult<> poll_for_answer(
		uint8_t					size,
		uint8_t					pos,
		uint16_t				address,
		uint8_t					mask,
		uint8_t					expected_value
    );

    IResult<> _poll_for_answer(
		uint16_t				address,
		uint8_t					expected_value
    );

    IResult<> poll_for_mcu_boot();

    static constexpr ErrorKind map_status_to_error(uint8_t status){
        return std::bit_cast<ErrorKind>(status);
    }
};
}