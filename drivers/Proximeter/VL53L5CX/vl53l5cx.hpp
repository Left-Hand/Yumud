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
#include "primitive/arithmetic/percentage.hpp"

namespace ymd::drivers{

class VL53L5CX final:public VL53L5CX_Prelude{ 
public:

    explicit VL53L5CX(hal::I2cDrv & i2c_drv):
        i2c_drv_(i2c_drv){;}
    explicit VL53L5CX(hal::I2cDrv && i2c_drv):
        i2c_drv_(std::move(i2c_drv)){;}
    explicit VL53L5CX(Some<hal::I2c *> i2c, const hal::I2cSlaveAddr<7> addr = DEFAULT_I2C_ADDR):
        i2c_drv_(i2c, addr){;}

    template<typename T = void>
    using IResult = Result<T, Error>;

    IResult<> init();




    IResult<> validate();


    IResult<> set_i2c_address(hal::I2cSlaveAddr<7> i2c_addr);



    IResult<PowerMode> get_power_mode();



    IResult<> set_power_mode(PowerMode power_mode);

    /**
     * @brief This function starts a ranging session. When the sensor streams, host
     * cannot change settings 'on-the-fly'.
     */

    IResult<> start_ranging();

    /**
     * @brief This function stops the ranging session. It must be used when the
     * sensor streams, after calling vl53l5cx_start_ranging().
     */

    IResult<> stop_ranging();

    //This function checks if a new data is ready by polling I2C. If a new
    //data is ready, a flag will be raised.
    IResult<bool> is_data_ready();

    /**
     * @brief This function gets the ranging data, using the selected output and the
     * resolution.
     * @param (VL53L5CX_Frame) *p_results : VL53L5 results structure.
     */

    IResult<> reflash_ranging_data(VL53L5CX_Frame		*p_results);

    /**
     * @brief This function gets the current resolution (4x4 or 8x8).
     * @param (uint8_t) *p_resolution : Value of this pointer will be equal to 16
     * for 4x4 mode, and 64 for 8x8 mode.
     */

    IResult<Resolution> get_resolution();

    /**
     * @brief This function sets a new resolution (4x4 or 8x8).
     * @param (uint8_t) resolution : Use macro VL53L5CX_RESOLUTION_4X4 or
     * VL53L5CX_RESOLUTION_8X8 to set the resolution.
     */

    IResult<> set_resolution(Resolution resolution);

    /**
     * @brief This function gets the current ranging frequency in Hz. Ranging
     * frequency corresponds to the time between each measurement.
     * @param (uint8_t) *p_frequency_hz: Contains the ranging frequency in Hz.
     */

    IResult<uint8_t> get_ranging_frequency_hz();

    /**
     * @brief This function sets a new ranging frequency in Hz. Ranging frequency
     * corresponds to the measurements frequency. This setting depends of
     * the resolution, so please select your resolution before using this function.
     * @param (uint8_t) frequency_hz : Contains the ranging frequency in Hz.
     * - For 4x4, min and max allowed values are : [1;60]
     * - For 8x8, min and max allowed values are : [1;15]
     * is not correct.
     */

    IResult<> set_ranging_frequency_hz(const uint8_t frequency_hz);

    /**
     * @brief This function gets the current integration time in ms.
     * @param (uint32_t) *p_time_ms: Contains integration time in ms.
     */

    IResult<uint32_t> get_integration_time_ms();

    /**
     * @brief This function sets a new integration time in ms. Integration time must
     * be computed to be lower than the ranging period, for a selected resolution.
     * Please note that this function has no impact on ranging mode continous.
     * @param (uint32_t) time_ms : Contains the integration time in ms. For all
     * resolutions and frequency, the minimum value is 2ms, and the maximum is
     * 1000ms.
     */

    IResult<> set_integration_time_ms(uint32_t integration_time_ms);

    /**
     * @brief This function gets the current sharpener in percent. Sharpener can be
     * changed to blur more or less zones depending of the application.
     * @param (uint32_t) *p_sharpener_percent: Contains the sharpener in percent.
     */

    IResult<Percentage<uint8_t>> get_sharpener_percent();

    /**
     * @brief This function sets a new sharpener value in percent. Sharpener can be
     * changed to blur more or less zones depending of the application. Min value is
     * 0 (disabled), and max is 99.
     * @param (uint32_t) sharpener_percent : Value between 0 (disabled) and 99%.
     */

    IResult<> set_sharpener_percent(Percentage<uint8_t> sharpener_percent);

    /**
     * @brief This function gets the current target order (closest or strongest).
     * @param (uint8_t) *p_target_order: Contains the target order.
     */

    IResult<TargetOrder> get_target_order();

    /**
     * @brief This function sets a new target order. Please use macros
     * VL53L5CX_TARGET_ORDER_STRONGEST and VL53L5CX_TARGET_ORDER_CLOSEST to define
     * the new output order. By default, the sensor is configured with the strongest
     * output.
     * @param (uint8_t) target_order : Required target order.
     * order is unknown.
     */

    IResult<> set_target_order(TargetOrder target_order);

    /**
     * @brief This function is used to get the ranging mode. Two modes are
     * available using ULD : Continuous and autonomous. The default
     * mode is Autonomous.
     * @param (uint8_t) *p_ranging_mode : current ranging mode
     */

    IResult<RangingMode> get_ranging_mode();

    /**
     * @brief This function is used to set the ranging mode. Two modes are
     * available using ULD : Continuous and autonomous. The default
     * mode is Autonomous.
     * @param (uint8_t) ranging_mode : Use macros VL53L5CX_RANGING_MODE_CONTINUOUS,
     * VL53L5CX_RANGING_MODE_CONTINUOUS.
     */

    IResult<> set_ranging_mode(RangingMode ranging_mode);



    IResult<Enable> get_detection_thresholds_enable();

    /**
     * @brief This function allows enable the detection thresholds.
     * @param (uint8_t) enabled : Set to 1 to enable, or 0 to disable thresholds.
     */

    IResult<> set_detection_thresholds_enable(Enable en);

    /**
     * @brief This function allows getting the detection thresholds.
     * @param (VL53L5CX_DetectionThresholds) *p_thresholds : Array of 64 thresholds.
     */

    IResult<> get_detection_thresholds(std::span<VL53L5CX_DetectionThresholds, VL53L5CX_NB_THRESHOLDS> p_thresholds);

    /**
     * @brief This function allows programming the detection thresholds.
     * @param (VL53L5CX_DetectionThresholds) *p_thresholds :  Array of 64 thresholds.
     */

    IResult<> set_detection_thresholds(std::span<const VL53L5CX_DetectionThresholds, VL53L5CX_NB_THRESHOLDS> p_thresholds);


    // @brief This function is used to initialized the motion indicator. By default,
    // indicator is programmed to monitor movements between 400mm and 1500mm.

    IResult<> motion_indicator_init(
        VL53L5CX_Motion_Config & motion_config,
        Resolution resolution);

    /**
     * @brief This function can be used to change the working distance of motion
     * indicator. By default, indicator is programmed to monitor movements between
     * 400mm and 1500mm.
     * @param (VL53L5CX_Motion_Config) *p_motion_config : Structure
     * containing the initialized motion configuration.
     * @param (uint16_t) distance_min_mm : Minimum distance for indicator (min value 
     * 400mm, max 4000mm).
     * @param (uint16_t) distance_max_mm : Maximum distance for indicator (min value 
     * 400mm, max 4000mm).
     * VL53L5CX_RESOLUTION_4X4 or VL53L5CX_RESOLUTION_8X8.
     */

    IResult<> motion_indicator_set_distance_motion(
        VL53L5CX_Motion_Config & motion_config,
        uint16_t			distance_min_mm,
        uint16_t			distance_max_mm);

    /**
     * @brief This function is used to update the internal motion indicator map.
     * @param (VL53L5CX_Motion_Config) *p_motion_config : Structure
     * containing the initialized motion configuration.
     * @param (uint8_t) resolution : Wanted SCI resolution, defined by macros
     * VL53L5CX_RESOLUTION_4X4 or VL53L5CX_RESOLUTION_8X8.
     */

    IResult<> motion_indicator_set_resolution(
        VL53L5CX_Motion_Config & motion_config,
        Resolution resolution
    );



    // @brief This function starts the VL53L5CX sensor in order to calibrate Xtalk.
    // This calibration is recommended is user wants to use a coverglass.
    // @param (uint16_t) reflectance_percent : Target reflectance in percent. This
    // value is include between 1 and 99%. For a better efficiency, ST recommends a
    // 3% target reflectance.
    // @param (uint8_t) nb_samples : Nb of samples used for calibration. A higher
    // number of samples means a higher accuracy, but it increases the calibration
    // time. Minimum is 1 and maximum is 16.
    // @param (uint16_t) distance_mm : Target distance in mm. The minimum allowed
    // distance is 600mm, and maximum is 3000mm. The target must stay in Full FOV,
    // so short distance are easier for calibration.

    IResult<> calibrate_xtalk(
        Percentage<uint8_t> reflectance_percent,
        uint8_t				nb_samples,
        uint16_t			distance_mm);

    //@brief This function gets the Xtalk buffer. The buffer is available after
    //using the function vl53l5cx_calibrate_xtalk().

    IResult<> get_caldata_xtalk(std::span<uint8_t, VL53L5CX_XTALK_BUFFER_SIZE> p_xtalk_data);

    
    // @brief This function sets the Xtalk buffer. This function can be used to
    // override default Xtalk buffer.

    IResult<> set_caldata_xtalk(std::span<const uint8_t,VL53L5CX_XTALK_BUFFER_SIZE> p_xtalk_data);

    /**
     * @brief This function gets the Xtalk margin. This margin is used to increase
     * the Xtalk threshold. It can also be used to avoid false positives after the
     * Xtalk calibration. The default value is 50 kcps/spads.
     * @param (uint32_t) *p_xtalk_margin : Xtalk margin in kcps/spads.
     */

    IResult<uint32_t> get_xtalk_margin();

    /**
     * @brief This function sets the Xtalk margin. This margin is used to increase
     * the Xtalk threshold. It can also be used to avoid false positives after the
     * Xtalk calibration. The default value is 50 kcps/spads.
     * @param (uint32_t) xtalk_margin : New Xtalk margin in kcps/spads. Min value is
     * 0 kcps/spads, and max is 10.000 kcps/spads
     * invalid.
     */

    IResult<> set_xtalk_margin(uint32_t			xtalk_margin);

    IResult<> program_output_config();


        
private:
    hal::I2cDrv i2c_drv_;
	/* Results stream_count_, value auto-incremented at each range */
	uint8_t		        stream_count_ = 0;
	/* Size of data read though I2C */
	size_t	        data_read_size_ = 0;

	/* Offset buffer */
	alignas(4) uint8_t		        offset_data[VL53L5CX_OFFSET_BUFFER_SIZE];
	/* Xtalk buffer */
	alignas(4) uint8_t		        xtalk_data[VL53L5CX_XTALK_BUFFER_SIZE];
	/* Temporary buffer used for internal driver processing */
    alignas(4) uint8_t	        temp_buffer[VL53L5CX_TEMPORARY_BUFFER_SIZE];

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

    /**
     * @brief This function can be used to read 'extra data' from DCI. Using a known
     * index, the function fills the casted structure passed in argument.
     * @param (uint8_t) *data : This field can be a casted structure, or a simple
     * array. Please note that the FW only accept data of 32 bits. So field data can
     * only have a size of 32, 64, 96, 128, bits ....
     * @param (uint32_t) index : Index of required value.
     * @param (uint16_t)*data_size : This field must be the structure or array size
     * (using sizeof() function).
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
     */

    IResult<> dci_replace_data(
            uint8_t				*data,
            uint32_t			index,
            uint16_t			data_size,
            const uint8_t				*new_data,
            uint16_t			new_data_size,
            uint16_t			new_data_pos);

    IResult<> send_xtalk_data(Resolution resolution);
    IResult<> send_offset_data(Resolution resolution);
};
}