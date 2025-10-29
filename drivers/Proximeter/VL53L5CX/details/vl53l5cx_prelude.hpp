#pragma once

/*******************************************************************************
* Copyright (c) 2020, STMicroelectronics - All Rights Reserved
*
* This file is part of the VL53L5CX Ultra Lite Driver and is dual licensed,
* either 'STMicroelectronics Proprietary license'
* or 'BSD 3-clause "New" or "Revised" License' , at your option.
*
********************************************************************************
*
* 'STMicroelectronics Proprietary license'
*
********************************************************************************
*
* License terms: STMicroelectronics Proprietary in accordance with licensing
* terms at www.st.com/sla0081
*
* STMicroelectronics confidential
* Reproduction and Communication of this document is strictly prohibited unless
* specifically authorized in writing by STMicroelectronics.
*
*
********************************************************************************
*
* Alternatively, the VL53L5CX Ultra Lite Driver may be distributed under the
* terms of 'BSD 3-clause "New" or "Revised" License', in which case the
* following provisions apply instead of the ones mentioned above :
*
********************************************************************************
*
* License terms: BSD 3-clause "New" or "Revised" License.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
* 1. Redistributions of source code must retain the above copyright notice, this
* list of conditions and the following disclaimer.
*
* 2. Redistributions in binary form must reproduce the above copyright notice,
* this list of conditions and the following disclaimer in the documentation
* and/or other materials provided with the distribution.
*
* 3. Neither the name of the copyright holder nor the names of its contributors
* may be used to endorse or promote products derived from this software
* without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
* FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
* SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
* CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
* OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
*
*******************************************************************************/

#include "core/math/real.hpp"
#include "core/utils/Result.hpp"
#include "core/utils/Errno.hpp"

#include "hal/bus/i2c/i2cdrv.hpp"

namespace ymd::drivers{ 

struct VL53L5CX_Prelude{
/**
 * @brief Current driver version.
 */

static constexpr auto VL53L5CX_API_REVISION			= "VL53L5CX_1.3.0";
static constexpr uint8_t VL53L5CX_NB_TARGET_PER_ZONE = 1U;

/**
 * @brief Default I2C address of VL53L5CX sensor. Can be changed using function
 * vl53l5cx_set_i2c_address() function is called.
 */

static constexpr uint16_t VL53L5CX_DEFAULT_I2C_ADDRESS = 0x52;

/**
 * @brief Macro VL53L5CX_RESOLUTION_4X4 or VL53L5CX_RESOLUTION_8X8 allows
 * setting sensor in 4x4 mode or 8x8 mode, using function
 * vl53l5cx_set_resolution().
 */

static constexpr uint8_t VL53L5CX_RESOLUTION_4X4 = 16U;
static constexpr uint8_t VL53L5CX_RESOLUTION_8X8 = 64U;

enum class Resolution:uint8_t{
	_4x4 = VL53L5CX_RESOLUTION_4X4,
	_8x8 = VL53L5CX_RESOLUTION_8X8
};

/**
 * @brief Macro VL53L5CX_TARGET_ORDER_STRONGEST or VL53L5CX_TARGET_ORDER_CLOSEST
 *	are used to select the target order for data output.
 */

static constexpr uint8_t VL53L5CX_TARGET_ORDER_CLOSEST = 1U;
static constexpr uint8_t VL53L5CX_TARGET_ORDER_STRONGEST = 2U;

/**
 * @brief Macro VL53L5CX_RANGING_MODE_CONTINUOUS and
 * VL53L5CX_RANGING_MODE_AUTONOMOUS are used to change the ranging mode.
 * Autonomous mode can be used to set a precise integration time, whereas
 * continuous is always maximum.
 */

static constexpr uint8_t VL53L5CX_RANGING_MODE_CONTINUOUS = 1U;
static constexpr uint8_t VL53L5CX_RANGING_MODE_AUTONOMOUS = 3U;

/**
 * @brief The default power mode is VL53L5CX_POWER_MODE_WAKEUP. User can choose
 * the mode VL53L5CX_POWER_MODE_SLEEP to save power consumption is the device
 * is not used. The low power mode retains the firmware and the configuration.
 * Both modes can be changed using function vl53l5cx_set_power_mode().
 */

static constexpr uint8_t VL53L5CX_POWER_MODE_SLEEP = 0U;
static constexpr uint8_t VL53L5CX_POWER_MODE_WAKEUP = 1U;

/**
 * @brief Macro VL53L5CX_STATUS_OK indicates that VL53L5 sensor has no error.
 * Macro VL53L5CX_STATUS_ERROR indicates that something is wrong (value,
 * I2C access, ...). Macro VL53L5CX_MCU_ERROR is used to indicate a MCU issue.
 */

static constexpr uint8_t VL53L5CX_STATUS_OK = 0U;
static constexpr uint8_t VL53L5CX_STATUS_TIMEOUT_ERROR = 1U;
static constexpr uint8_t VL53L5CX_MCU_ERROR = 66U;
static constexpr uint8_t VL53L5CX_STATUS_INVALID_PARAM = 127U;
static constexpr uint8_t VL53L5CX_STATUS_ERROR = 255U;

/**
 * @brief Definitions for Range results block headers
 */

// #if VL53L5CX_NB_TARGET_PER_ZONE == 1

static constexpr uint32_t VL53L5CX_START_BH = 0x0000000DU;
static constexpr uint32_t VL53L5CX_METADATA_BH = 0x54B400C0U;
static constexpr uint32_t VL53L5CX_COMMONDATA_BH = 0x54C00040U;
static constexpr uint32_t VL53L5CX_AMBIENT_RATE_BH = 0x54D00104U;
static constexpr uint32_t VL53L5CX_SPAD_COUNT_BH = 0x55D00404U;
static constexpr uint32_t VL53L5CX_NB_TARGET_DETECTED_BH = 0xCF7C0401U;
static constexpr uint32_t VL53L5CX_SIGNAL_RATE_BH = 0xCFBC0404U;
static constexpr uint32_t VL53L5CX_RANGE_SIGMA_MM_BH = 0xD2BC0402U;
static constexpr uint32_t VL53L5CX_DISTANCE_BH = 0xD33C0402U;
static constexpr uint32_t VL53L5CX_REFLECTANCE_BH = 0xD43C0401U;
static constexpr uint32_t VL53L5CX_TARGET_STATUS_BH = 0xD47C0401U;
static constexpr uint32_t VL53L5CX_MOTION_DETECT_BH = 0xCC5008C0U;

static constexpr uint16_t VL53L5CX_METADATA_IDX = 0x54B4U;
static constexpr uint16_t VL53L5CX_SPAD_COUNT_IDX = 0x55D0U;
static constexpr uint16_t VL53L5CX_AMBIENT_RATE_IDX = 0x54D0U;
static constexpr uint16_t VL53L5CX_NB_TARGET_DETECTED_IDX = 0xCF7CU;
static constexpr uint16_t VL53L5CX_SIGNAL_RATE_IDX = 0xCFBCU;
static constexpr uint16_t VL53L5CX_RANGE_SIGMA_MM_IDX = 0xD2BCU;
static constexpr uint16_t VL53L5CX_DISTANCE_IDX = 0xD33CU;
static constexpr uint16_t VL53L5CX_REFLECTANCE_EST_PC_IDX = 0xD43CU;
static constexpr uint16_t VL53L5CX_TARGET_STATUS_IDX = 0xD47CU;
static constexpr uint16_t VL53L5CX_MOTION_DETEC_IDX = 0xCC50U;

#if 0
static constexpr uint32_t VL53L5CX_START_BH = 0x0000000DU;
static constexpr uint32_t VL53L5CX_METADATA_BH = 0x54B400C0U;
static constexpr uint32_t VL53L5CX_COMMONDATA_BH = 0x54C00040U;
static constexpr uint32_t VL53L5CX_AMBIENT_RATE_BH = 0x54D00104U;
static constexpr uint32_t VL53L5CX_NB_TARGET_DETECTED_BH = 0x57D00401U;
static constexpr uint32_t VL53L5CX_SPAD_COUNT_BH = 0x55D00404U;
static constexpr uint32_t VL53L5CX_SIGNAL_RATE_BH = 0x58900404U;
static constexpr uint32_t VL53L5CX_RANGE_SIGMA_MM_BH = 0x64900402U;
static constexpr uint32_t VL53L5CX_DISTANCE_BH = 0x66900402U;
static constexpr uint32_t VL53L5CX_REFLECTANCE_BH = 0x6A900401U;
static constexpr uint32_t VL53L5CX_TARGET_STATUS_BH = 0x6B900401U;
static constexpr uint32_t VL53L5CX_MOTION_DETECT_BH = 0xCC5008C0U;

static constexpr uint16_t VL53L5CX_METADATA_IDX = 0x54B4U;
static constexpr uint16_t VL53L5CX_SPAD_COUNT_IDX = 0x55D0U;
static constexpr uint16_t VL53L5CX_AMBIENT_RATE_IDX = 0x54D0U;
static constexpr uint16_t VL53L5CX_NB_TARGET_DETECTED_IDX = 0x57D0U;
static constexpr uint16_t VL53L5CX_SIGNAL_RATE_IDX = 0x5890U;
static constexpr uint16_t VL53L5CX_RANGE_SIGMA_MM_IDX = 0x6490U;
static constexpr uint16_t VL53L5CX_DISTANCE_IDX = 0x6690U;
static constexpr uint16_t VL53L5CX_REFLECTANCE_EST_PC_IDX = 0x6A90U;
static constexpr uint16_t VL53L5CX_TARGET_STATUS_IDX = 0x6B90U;
static constexpr uint16_t VL53L5CX_MOTION_DETEC_IDX = 0xCC50U;
#endif


/**
 * @brief Inner Macro for API. Not for user, only for development.
 */

static constexpr uint16_t VL53L5CX_NVM_DATA_SIZE = 492U;
static constexpr uint16_t VL53L5CX_CONFIGURATION_SIZE = 972U;
static constexpr uint16_t VL53L5CX_OFFSET_BUFFER_SIZE = 488U;
static constexpr uint16_t VL53L5CX_XTALK_BUFFER_SIZE = 776U;

static constexpr uint16_t VL53L5CX_DCI_ZONE_CONFIG = 0x5450U;
static constexpr uint16_t VL53L5CX_DCI_FREQ_HZ = 0x5458U;
static constexpr uint16_t VL53L5CX_DCI_INT_TIME = 0x545CU;
static constexpr uint16_t VL53L5CX_DCI_FW_NB_TARGET = 0x5478;
static constexpr uint16_t VL53L5CX_DCI_RANGING_MODE = 0xAD30U;
static constexpr uint16_t VL53L5CX_DCI_DSS_CONFIG = 0xAD38U;
static constexpr uint16_t VL53L5CX_DCI_TARGET_ORDER = 0xAE64U;
static constexpr uint16_t VL53L5CX_DCI_SHARPENER = 0xAED8U;
static constexpr uint16_t VL53L5CX_DCI_MOTION_DETECTOR_CFG = 0xBFACU;
static constexpr uint16_t VL53L5CX_DCI_SINGLE_RANGE = 0xCD5CU;
static constexpr uint16_t VL53L5CX_DCI_OUTPUT_CONFIG = 0xCD60U;
static constexpr uint16_t VL53L5CX_DCI_OUTPUT_ENABLES = 0xCD68U;
static constexpr uint16_t VL53L5CX_DCI_OUTPUT_LIST = 0xCD78U;
static constexpr uint16_t VL53L5CX_DCI_PIPE_CONTROL = 0xCF78U;

static constexpr uint16_t VL53L5CX_UI_CMD_STATUS = 0x2C00U;
static constexpr uint16_t VL53L5CX_UI_CMD_START = 0x2C04U;
static constexpr uint16_t VL53L5CX_UI_CMD_END = 0x2FFFU;

/**
 * @brief Inner values for API. Max buffer size depends of the selected output.
 */

#ifndef VL53L5CX_DISABLE_AMBIENT_PER_SPAD
static constexpr size_t  L5CX_AMB_SIZE	= 260U;
#else
static constexpr size_t  L5CX_AMB_SIZE	= 0U;
#endif

#ifndef VL53L5CX_DISABLE_NB_SPADS_ENABLED
static constexpr size_t  L5CX_SPAD_SIZE	= 260U;
#else
static constexpr size_t  L5CX_SPAD_SIZE	= 0U;
#endif

#ifndef VL53L5CX_DISABLE_NB_TARGET_DETECTED
static constexpr size_t  L5CX_NTAR_SIZE	= 68U;
#else
static constexpr size_t  L5CX_NTAR_SIZE	= 0U;
#endif

#ifndef VL53L5CX_DISABLE_SIGNAL_PER_SPAD
static constexpr size_t  L5CX_SPS_SIZE = ((256U * VL53L5CX_NB_TARGET_PER_ZONE) + 4U);
#else
static constexpr size_t  L5CX_SPS_SIZE	= 0U;
#endif

#ifndef VL53L5CX_DISABLE_RANGE_SIGMA_MM
static constexpr size_t  L5CX_SIGR_SIZE = ((128U * VL53L5CX_NB_TARGET_PER_ZONE) + 4U);
#else
static constexpr size_t  L5CX_SIGR_SIZE	= 0U;
#endif

#ifndef VL53L5CX_DISABLE_DISTANCE_MM
static constexpr size_t  L5CX_DIST_SIZE = ((128U * VL53L5CX_NB_TARGET_PER_ZONE) + 4U);
#else
static constexpr size_t  L5CX_DIST_SIZE	= 0U;
#endif

#ifndef VL53L5CX_DISABLE_REFLECTANCE_PERCENT
static constexpr size_t  L5CX_RFLEST_SIZE = ((64U *VL53L5CX_NB_TARGET_PER_ZONE) + 4U);
#else
static constexpr size_t  L5CX_RFLEST_SIZE	= 0U;
#endif

#ifndef VL53L5CX_DISABLE_TARGET_STATUS
static constexpr size_t  L5CX_STA_SIZE = ((64U  *VL53L5CX_NB_TARGET_PER_ZONE) + 4U);
#else
static constexpr size_t  L5CX_STA_SIZE	= 0U;
#endif

#ifndef VL53L5CX_DISABLE_MOTION_INDICATOR
static constexpr size_t  L5CX_MOT_SIZE	= 144U;
#else
static constexpr size_t  L5CX_MOT_SIZE	= 0U;
#endif

/**
 * @brief Macro VL53L5CX_MAX_RESULTS_SIZE indicates the maximum size used by
 * output through I2C. Value 40 corresponds to headers + meta-data + common-data
 * and 8 corresponds to the footer.
 */

static constexpr size_t VL53L5CX_MAX_RESULTS_SIZE = ( 40U 
	+ L5CX_AMB_SIZE + L5CX_SPAD_SIZE + L5CX_NTAR_SIZE + L5CX_SPS_SIZE
	+ L5CX_SIGR_SIZE + L5CX_DIST_SIZE + L5CX_RFLEST_SIZE + L5CX_STA_SIZE
	+ L5CX_MOT_SIZE + 8U);

/**
 * @brief Macro VL53L5CX_TEMPORARY_BUFFER_SIZE can be used to know the size of
 * the temporary buffer. The minimum size is 1024, and the maximum depends of
 * the output configuration.
 */

static constexpr size_t VL53L5CX_TEMPORARY_BUFFER_SIZE = MAX(1024U, VL53L5CX_MAX_RESULTS_SIZE);

/**
 * @brief Structure VL53L5CX_Configuration contains the sensor configuration.
 * User MUST not manually change these field, except for the sensor address.
 */

/**
 * @brief Structure VL53L5CX_Frame contains the ranging results of
 * VL53L5CX. If user wants more than 1 target per zone, the results can be split
 * into 2 sub-groups :
 * - Per zone results. These results are common to all targets (ambient_per_spad
 * , nb_target_detected and nb_spads_enabled).
 * - Per target results : These results are different relative to the detected
 * target (signal_per_spad, range_sigma_mm, distance_mm, reflectance,
 * target_status).
 */

struct VL53L5CX_Frame{
	/* Internal sensor silicon temperature */
	int8_t silicon_temp_degc;

	/* Ambient noise in kcps/spads */
#ifndef VL53L5CX_DISABLE_AMBIENT_PER_SPAD
	uint32_t ambient_per_spad[VL53L5CX_RESOLUTION_8X8];
#endif

	/* Number of valid target detected for 1 zone */
#ifndef VL53L5CX_DISABLE_NB_TARGET_DETECTED
	uint8_t nb_target_detected[VL53L5CX_RESOLUTION_8X8];
#endif

	/* Number of spads enabled for this ranging */
#ifndef VL53L5CX_DISABLE_NB_SPADS_ENABLED
	uint32_t nb_spads_enabled[VL53L5CX_RESOLUTION_8X8];
#endif

	/* Signal returned to the sensor in kcps/spads */
#ifndef VL53L5CX_DISABLE_SIGNAL_PER_SPAD
	uint32_t signal_per_spad[(VL53L5CX_RESOLUTION_8X8
					*VL53L5CX_NB_TARGET_PER_ZONE)];
#endif

	/* Sigma of the current distance in mm */
#ifndef VL53L5CX_DISABLE_RANGE_SIGMA_MM
	uint16_t range_sigma_mm[(VL53L5CX_RESOLUTION_8X8
					*VL53L5CX_NB_TARGET_PER_ZONE)];
#endif

	/* Measured distance in mm */
#ifndef VL53L5CX_DISABLE_DISTANCE_MM
	int16_t distance_mm[(VL53L5CX_RESOLUTION_8X8
					*VL53L5CX_NB_TARGET_PER_ZONE)];
#endif

	/* Estimated reflectance in percent */
#ifndef VL53L5CX_DISABLE_REFLECTANCE_PERCENT
	uint8_t reflectance[(VL53L5CX_RESOLUTION_8X8
					*VL53L5CX_NB_TARGET_PER_ZONE)];
#endif

	/* Status indicating the measurement validity (5 & 9 means ranging OK)*/
#ifndef VL53L5CX_DISABLE_TARGET_STATUS
	uint8_t target_status[(VL53L5CX_RESOLUTION_8X8
					*VL53L5CX_NB_TARGET_PER_ZONE)];
#endif

	/* Motion detector results */
#ifndef VL53L5CX_DISABLE_MOTION_INDICATOR
	struct MotionIndicator
	{
		uint32_t global_indicator_1;
		uint32_t global_indicator_2;
		uint8_t	 status;
		uint8_t	 nb_of_detected_aggregates;
		uint8_t	 nb_of_aggregates;
		uint8_t	 spare;
		uint32_t motion[32];
	};
	MotionIndicator motion_indicator;
#endif

};
struct VL53L5CX_Motion_Configuration{
	int32_t  ref_bin_offset;
	uint32_t detection_threshold;
	uint32_t extra_noise_sigma;
	uint32_t null_den_clip_value;
	uint8_t  mem_update_mode;
	uint8_t  mem_update_choice;
	uint8_t  sum_span;
	uint8_t  feature_length;
	uint8_t  nb_of_aggregates;
	uint8_t  nb_of_temporal_accumulations;
	uint8_t  min_nb_for_global_detection;
	uint8_t  global_indicator_format_1;
	uint8_t  global_indicator_format_2;
	uint8_t  spare_1;
	uint8_t  spare_2;
	uint8_t  spare_3;
	int8_t 	 map_id[64];
	uint8_t  indicator_format_1[32];
	uint8_t  indicator_format_2[32];
};

struct Block_header {
	uint32_t type : 4;
	uint32_t size : 12;
	uint32_t idx : 16;

	[[nodiscard]] std::span<const uint8_t, 4> as_bytes() const {
		return std::span<const uint8_t, 4>(reinterpret_cast<const uint8_t*>(this), 4);
	} 

	[[nodiscard]] std::span<uint8_t, 4> as_mut_bytes() {
		return std::span<uint8_t, 4>(reinterpret_cast<uint8_t*>(this), 4);
	} 
};


/**
 * @brief Macro VL53L5CX_NB_THRESHOLDS indicates the number of checkers. This
 * value cannot be changed.
 */

static constexpr uint8_t VL53L5CX_NB_THRESHOLDS = 64U;

/**
 * @brief Inner Macro for API. Not for user, only for development.
 */

static constexpr uint16_t VL53L5CX_DCI_DET_THRESH_CONFIG = 0x5488U;
static constexpr uint16_t VL53L5CX_DCI_DET_THRESH_GLOBAL_CONFIG = 0xB6E0U;
static constexpr uint16_t VL53L5CX_DCI_DET_THRESH_START = 0xB6E8U;
static constexpr uint16_t VL53L5CX_DCI_DET_THRESH_VALID_STATUS = 0xB9F0U;

/**
 * @brief Macro VL53L5CX_LAST_THRESHOLD is used to indicate the end of checkers
 * programming.
 */

static constexpr uint8_t VL53L5CX_LAST_THRESHOLD = 128U;

/**
 * @brief The following macro are used to define the 'param_type' of a checker.
 * They indicate what is the measurement to catch.
 */

static constexpr uint8_t VL53L5CX_DISTANCE_MM = 1U;
static constexpr uint8_t VL53L5CX_SIGNAL_PER_SPAD_KCPS = 2U;
static constexpr uint8_t VL53L5CX_RANGE_SIGMA_MM = 4U;
static constexpr uint8_t VL53L5CX_AMBIENT_PER_SPAD_KCPS = 8U;
static constexpr uint8_t VL53L5CX_NB_TARGET_DETECTED = 9U;
static constexpr uint8_t VL53L5CX_TARGET_STATUS = 12U;
static constexpr uint8_t VL53L5CX_NB_SPADS_ENABLED = 13U;
static constexpr uint8_t VL53L5CX_MOTION_INDICATOR = 19U;

/**
 * @brief The following macro are used to define the 'type' of a checker.
 * They indicate the window of measurements, defined by low and a high
 * thresholds.
 */

static constexpr uint8_t VL53L5CX_IN_WINDOW = 0U;
static constexpr uint8_t VL53L5CX_OUT_OF_WINDOW = 1U;
static constexpr uint8_t VL53L5CX_LESS_THAN_EQUAL_MIN_CHECKER = 2U;
static constexpr uint8_t VL53L5CX_GREATER_THAN_MAX_CHECKER = 3U;
static constexpr uint8_t VL53L5CX_EQUAL_MIN_CHECKER = 4U;
static constexpr uint8_t VL53L5CX_NOT_EQUAL_MIN_CHECKER = 5U;

/**
 * @brief The following macro are used to define multiple checkers in the same
 * zone, using operators. Please note that the first checker MUST always be a OR
 * operation.
 */

static constexpr uint8_t VL53L5CX_OPERATION_NONE = 0U;
static constexpr uint8_t VL53L5CX_OPERATION_OR = 0U;
static constexpr uint8_t VL53L5CX_OPERATION_AND = 2U;

static constexpr uint8_t VL53L5CX_FW_NBTAR_XTALK = (VL53L5CX_NB_TARGET_PER_ZONE == 1) ? 
	2 : VL53L5CX_NB_TARGET_PER_ZONE;

static constexpr size_t  VL53L5CX_FW_NBTAR_RANGING	= 
    (VL53L5CX_NB_TARGET_PER_ZONE == 1) ? 2 : VL53L5CX_NB_TARGET_PER_ZONE;
/**
 * @brief Structure VL53L5CX_DetectionThresholds contains a single threshold.
 * This structure  is never used alone, it must be used as an array of 64
 * thresholds (defined by macro VL53L5CX_NB_THRESHOLDS).
 */

struct VL53L5CX_DetectionThresholds{

	/* Low threshold */
	int32_t 	param_low_thresh;
	/* High threshold */
	int32_t 	param_high_thresh;
	/* Measurement to catch (VL53L5CX_MEDIAN_RANGE_MM,...)*/
	uint8_t 	measurement;
	/* Windows type (VL53L5CX_IN_WINDOW, VL53L5CX_OUT_WINDOW, ...) */
	uint8_t 	type;
	/* Zone id. Please read VL53L5 user manual to find the zone id.Set macro
	 * VL53L5CX_LAST_THRESHOLD to indicates the end of checkers */
	uint8_t 	zone_num;
	/* Mathematics operation (AND/OR). The first threshold is always OR.*/
	uint8_t		mathematic_operation;
};

};

}