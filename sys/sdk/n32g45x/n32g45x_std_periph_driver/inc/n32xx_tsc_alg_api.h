/*****************************************************************************
 * Copyright (c) 2019, Nations Technologies Inc.
 *
 * All rights reserved.
 * ****************************************************************************
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the disclaimer below.
 *
 * Nations' name may not be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * DISCLAIMER: THIS SOFTWARE IS PROVIDED BY NATIONS "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * DISCLAIMED. IN NO EVENT SHALL NATIONS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * ****************************************************************************/

/**
 * @brief �����㷨ͷ�ļ�.
 * �������:(����TIMER->����ص�API->��ʼ��->����)
 * @file n32xx_tsc_alg_api.h
 * @author Nations
 * @version v1.0.1
 * @copyright Copyright (c) 2019, Nations Technologies Inc. All rights reserved.
 */
#ifndef __N32XX_TSC_ALG_API__
#define __N32XX_TSC_ALG_API__

#ifdef __cplusplus
extern "C" {
#endif // defined __cplusplus

#define TSC_ALG_VERSION_NUMBER "Ver1.0.4" //�㷨��汾��

#define TSC_ALG_HANDLER_STOP2_DATA_SIZE (144)  //������STOP2ģʽ�±��津�ػ��ѹ�����ص�TSC���ݡ�
#define TSC_ALG_HANDLER_PERIOD_PER_CHN  (400)  //�����㷨��ͨ���Ĵ����������ӡ�
#define TSC_ALG_WAKEUP_TIMES            (1000) //�й�TSC���ѹ��ܵ�ʱ�����ã���Ҫ�����޸�
#define TSC_ALG_DEBUG_BUF_SIZE          (260)  //�������ģʽ�µ�BUF��С
#define TSC_ALG_REF_SIZE_PER_CHN        (430)  //����ÿͨ���Ĳο���С��ʵ�ʴ�С��tsc_alg_need_sramsize()����Ϊ׼

/**
 * @brief ���崥������
 */
typedef enum tsc_alg_type_e
{
    TSC_ALG_BUTTON_TYPE = 0, ///< tsc application of simple button
    TSC_ALG_TYPE_MAX         ///<
} tsc_alg_type;

/**
 * @brief �����¼�
 */
typedef enum tsc_press_key_event_e
{
    TSC_PRESS_KEY_NORMAL = 0, //�����̰��¼�
    TSC_PRESS_KEY_MAX         ///<
} tsc_press_key_event;

/**
 * @brief �������Ͷ���
 */
typedef enum tsc_ret_code_e
{
    TSC_SUCCESS = 0,               ///< �ɹ�
    TSC_NOT_INIT_ERR,              ///< ģ��δ��ʼ������
    TSC_NOT_REG_CHN_ERR,           ///< ģ��ע����Ч�Ĵ���ͨ������
    TSC_NOT_ACCORD_LIB_ERR,        ///< �㷨��汾����
    TSC_POINTER_NULL_ERR,          ///< ָ��Ϊ�մ���
    TSC_PARAM_ZERO_ERR,            ///< ��������
    TSC_REPEAT_REG_ERR,            ///< �ظ�ע�����
    TSC_CHN_NUM_ERR,               ///< ���ʼ����ͨ��������һ�´���
    TSC_REG_CHANNEL_ENOUGH_ERR,    ///< ע���ͨ���Ŵ���򳬳�ϵͳ��ͨ����
    TSC_REG_TIMX_ERR,              ///< ע���Timer��Դ����
    TSC_REG_DMA_ERR,               ///< ע���DMA��Դ����
    TSC_SOURCE_NOT_ENOUGH_ERR,     ///< ��Դ�������
    TSC_NOT_SUPPORT_ERR,           ///< δʵ�ֻ������֧�ִ���
    TSC_LEVEL_CFG_ERR,             ///< ͨ�������������ô���
    TSC_AUTO_CALIB_TIMER_ERR,      ///< �Զ�У׼ʱ��С��2��ͨ����������ʱ��.
    TSC_DISTURB_ERR,               ///< ���Ŵ���.
    TSC_CHN_RAM_NOT_ENOUGH_ERR,    ///< �ṩ��TSCͨ��RAMΪNULL��ռ䲻�����
    TSC_STOP2_NULL_OR_INVALID_ERR, ///< �ṩ��Stop2Data���ݿռ�ΪNULL����16K retention�����ڡ�
    TSC_DEBUG_BUF_ENOUGH_ERR       ///< �ṩ�ĵ��Ի���ռ䲻�����
} tsc_ret_code;

/**
 * @brief ���ذ��������ȼ�
 * �����ȼ�ԽС:��Ӧ�ٶ�Խ�죬��˲��ĸ���ҲԽ����
 * �����ȼ�Խ��:��Ӧ�ٶ����������˲���������Խǿ
 */
typedef enum tsc_hld_lev_e
{
    TSC_HOLD_LEV1  = 1,  // HOLD�ȼ�1(5ms)
    TSC_HOLD_LEV2  = 2,  // HOLD�ȼ�2(7ms)
    TSC_HOLD_LEV3  = 3,  // HOLD�ȼ�3(11ms)
    TSC_HOLD_LEV4  = 4,  // HOLD�ȼ�4(17ms)
    TSC_HOLD_LEV5  = 5,  // HOLD�ȼ�5(25ms)
    TSC_HOLD_LEV6  = 6,  // HOLD�ȼ�6(35ms)
    TSC_HOLD_LEV7  = 7,  // HOLD�ȼ�7(47ms)
    TSC_HOLD_LEV8  = 8,  // HOLD�ȼ�8(61ms)
    TSC_HOLD_LEV9  = 9,  // HOLD�ȼ�9(77ms)
    TSC_HOLD_LEV10 = 10, // HOLD�ȼ�10(95ms)
    TSC_HOLD_LEV11 = 11, // HOLD�ȼ�11(115ms)
    TSC_HOLD_LEV12 = 12, // HOLD�ȼ�12(137ms)
    TSC_HOLD_LEV13 = 13, // HOLD�ȼ�13(161ms)
    TSC_HOLD_LEV14 = 14, // HOLD�ȼ�14(187ms)
    TSC_HOLD_LEV15 = 15, // HOLD�ȼ�15(215ms)
    TSC_HOLD_LEV16 = 16, // HOLD�ȼ�16(245ms)
    TSC_HOLD_LEV17 = 17, // HOLD�ȼ�17(277ms)
    TSC_HOLD_LEV18 = 18, // HOLD�ȼ�18(311ms)
    TSC_HOLD_LEV19 = 19, // HOLD�ȼ�19(347ms)
    TSC_HOLD_LEV20 = 20, // HOLD�ȼ�20(385ms)
    TSC_HOLD_MAX         ///< ��Ч
} tsc_hld_lev;

/**
 * @brief �ڵ͹���ģʽ�£��滷���仯�����»������ޡ�
 * �������һ���仯�����ӡ�
 * С�ڴ˱仯�����ӵ�delta����Ϊ����Ч�仯����ͨ���˽ӿڸ���TSC�������ޣ�
 * ���ڴ˱仯�����ӵ�delta����Ϊ����Ч�仯����˽ӿں���֮�������»������ޡ�
 * �仯������Խ�����ʾ�仯��Խ��һ������ΪLEV15��
 */
typedef enum tsc_delta_limit_lev_e
{
    TSC_DELTA_LIMIT_LEV1  = 1,  //
    TSC_DELTA_LIMIT_LEV2  = 2,  //
    TSC_DELTA_LIMIT_LEV3  = 3,  //
    TSC_DELTA_LIMIT_LEV4  = 4,  //
    TSC_DELTA_LIMIT_LEV5  = 5,  //
    TSC_DELTA_LIMIT_LEV6  = 6,  //
    TSC_DELTA_LIMIT_LEV7  = 7,  //
    TSC_DELTA_LIMIT_LEV8  = 8,  //
    TSC_DELTA_LIMIT_LEV9  = 9,  //
    TSC_DELTA_LIMIT_LEV10 = 10, //
    TSC_DELTA_LIMIT_LEV11 = 11, //
    TSC_DELTA_LIMIT_LEV12 = 12, //
    TSC_DELTA_LIMIT_LEV13 = 13, //
    TSC_DELTA_LIMIT_LEV14 = 14, //
    TSC_DELTA_LIMIT_LEV15 = 15, //
    TSC_DELTA_LIMIT_LEV16 = 16, //
    TSC_DELTA_LIMIT_LEV17 = 17, //
    TSC_DELTA_LIMIT_LEV18 = 18, //
    TSC_DELTA_LIMIT_LEV19 = 19, //
    TSC_DELTA_LIMIT_LEV20 = 20, //
    TSC_DELTA_LIMIT_MAX         ///< ��Ч
} tsc_delta_limit_lev;

/**
 * @brief �����ŵȼ�
 * �����ŵȼ�,�ȼ�Խ�߿�����Խǿ����Ҳ�԰弶����Ҫ��Խ�Ͽ�.
 */
typedef enum tsc_resist_disturb_lev_e
{
    TSC_RESIST_DIS_LEV0 = 0, //Ĭ�ϵȼ������ⲿ����һ�㡣֧��PCBA&�ǿ���������
    TSC_RESIST_DIS_LEV1 = 1, //��ǿ�ȼ������ⲿ������ǿ���ǿ��������������á�
    TSC_RESIST_DIS_LEV2 = 2, //�ݱ�����
    TSC_RESIST_DIS_MAX       ///< ��Ч
} tsc_resist_disturb_lev;

/**
 * @brief TSC����ͨ����ʼ����ֵ����
 */
typedef struct TSC_AlgInitThreValue_t
{
    uint16_t hold_level;     /* �������������ȼ� */
    uint16_t rate_of_change; /* ��ͨ�������仯��(����ѹ��Ϊ70,ѹ��Ϊ77����仯��Ϊ(77-70)/70 = 0.1��%10(ע��:�ʵ�����Ϊ8%)��Ĭ��Ϊ5,��仯��%5 */
    uint32_t chn;            /* ͨ�� */
} TSC_AlgInitThreValue;

/**
 * @brief TSC��ʼ�����ò���
 */
typedef struct TSC_AlgInitTypeDef_t
{
    TIM_Module* TIMx;                      /* �����㷨ʹ�õ�TIMER��Դ(��֧��TIMER2) */
    DMA_ChannelType* DMAyChx;              /* �����㷨ʹ�õ�DMA��Դ(��֧��DMA1_CH5) */
    uint32_t DMARemapEnable;               /* �Ƿ�ʹ��DMA ȫ��REMAP����(��DMA1������ͨ����ʹ��REMAP���ܣ���˴�������Ϊ1) */
    TSC_AlgInitThreValue* pTScChannelList; /* �ɴ���ͨ������б�����顣Ŀǰ��֧��1����(��ͨ��λ������,�����TSCͨ�����һ���б�)�� */
    uint32_t AutoCalibrateTimer;           /* �����и���������µ��Զ�У׼ʱ��(�޸���������ʱ����У׼),һ������1000ms����,���65535����λms����ֵ������ڰ�������ʱ���2�����ϣ������ʼ������ */
    uint32_t ResistDisturbLev;             /* �����ŵȼ�(tsc_resist_disturb_lev),�ȼ�Խ�߿�����Խǿ����Ҳ�԰弶װ�价��Ҫ��Խ��. */
    uint8_t* pTscSramAddr;                 /* Ӧ�ó����ṩ��TSC������Ĵ���ͨ��RAM�ռ��ַ*/
    uint32_t TscSramSize;                  /* Ӧ�ó����ṩ��TSC������Ĵ���ͨ��RAM�ռ��С.��λ(bytes) */
    uint16_t* LogBuf;                      /* ���ڵ���ģʽ�µ�buf����,�ǵ���ģʽ����Ϊ0 */
    uint16_t LogBufSize;                   /* ÿͨ����СΪu16 * 256.��λ(bytes) */
    uint8_t* Stop2Data;                    /* ������STOP2ģʽ�±��津�ػ��ѹ�����ص�TSC����BUF�� */
    uint16_t Stop2DataSize;                /* ������STOP2ģʽ�±��津�ػ��ѹ�����ص�TSC����BUF��С����λ(bytes) */
} TSC_AlgInitTypeDef;

/**
 * @brief �����㷨ʵʱ����������(�������TIMER�жϺ�����)
 * @TIMER��ʱ���ڲο��������ӣ���ʱ�����ڲο�DEMO����.
 * @param void
 * @return void
 */
void tsc_alg_analyze_handler(void);

/**
 * @brief �͹���У׼
 * @param uint32_t delta_limit_level �仯����ֵ�ȼ�tsc_delta_limit_lev
 * @uint32_t hse_or_hsi 0:HSI,  1:HSE;
 * @return
 * - `TSC_SUCCESS�� ��ʾ�����ɹ�
 * - ����ֵ��ʾ����
 * - ע�⣺������STOP2�͹���ģʽ�£���ʱУ׼��
 */
int32_t tsc_alg_set_powerdown_calibrate(tsc_delta_limit_lev delta_limit_level, uint32_t hse_or_hsi);

/**
 * @brief �͹���ģʽ��,����Ƿ񱻸��Ż���
 * @param void
 * @return 0:�������ѣ�1:���Ż���
 */
int32_t tsc_alg_wakeup_disturb_check(uint32_t* wakeup_src);

/**
 * @brief ��ȡ�㷨�汾
 * @param void
 * @return void
 */
char* tsc_alg_get_version(void);

/**
 * @brief �����㷨ϵͳ�δ�,Ĭ��1ms
 * @param void
 * @return void
 */
void tsc_alg_tick_count(void);

/**
 * @brief ��ȡTSC�����㷨��Ҫ��SRAM��С
 * uint32_t      chn_totals; // ʹ�õ�TSC����ͨ����
 * @return
 * - 0:     ��ʾʧ��
 * - ��0:  ��ʾ�ɹ�
 */
uint32_t tsc_alg_need_sramsize(uint32_t chn_totals);

/**
 * @brief �����㷨��ʼ��
 * @param tsc_init_parameter *ptsc_init_parameter �����㷨��ʼ���ṹ���ַ.
 * @param void
 * @return
 * - `TSC_SUCCESS�� ��ʾ�����ɹ�
 * - ����ֵ��ʾ����
 */
int32_t tsc_alg_init(TSC_AlgInitTypeDef* TSC_AlgInitStruct);

/**
 * @brief �������ؿ�ʼ����
 * @param void
 * @return
 * - `TSC_SUCCESS�� ��ʾ�����ɹ�
 * - ����ֵ��ʾ����
 */
int32_t tsc_alg_start(void);

/**
 * @brief ����TSC����͹���(���ڵ͹��Ļ���ģʽ)
 * @param uint32_t TScChannelList ��������������Ϊ0��ʾʹ����ע�������ͨ��
 * @return
 * - `TSC_SUCCESS�� ��ʾ�����ɹ�
 * - ����ֵ��ʾ����
 * - ע�⣺STOP2�͹���ģʽʹ�ã�����ģʽ�²���ע��
 */
int32_t tsc_alg_set_powerdown(uint32_t TscChannelList);

////////////////////////////////////////////////////////////

/*****************�ϲ�Ӧ���ṩ�İ����ص�������*********
 * @brief ע�ᰴť�͡������͡�ת���ʹ��صĻص�����
 * @param tsc_touch_type type �����Ĵ�������(��ֻ֧�ְ�����)
 * @param uint32_t event 0:���������¼���
 * @param uint32_t chn ��ʾ����ͨ���ţ�
 * @param uint32_t value ����״̬��1ѹ�£�0�ɿ���
 * @return
 * - `TSC_SUCCESS�� ��ʾ�����ɹ�
 * - ����ֵ��ʾ����
 * ע��:�˻ص����������ж��е��ã���˾������ٻص������Ĵ���ʱ�䡣
 ********************************************************/
int32_t tsc_alg_isr_callback(tsc_alg_type type, uint32_t event, uint32_t chn, uint32_t value);

/**
 * @brief �������������PC�Ľӿڣ��Ա���PC�˹��߹۲죬�趨����Ĵ�����ֵ
 * @param uint32_t chn ����ͨ��
 * @return uint8_t data �ô���ͨ������
 */
void tsc_alg_debug_output(uint32_t chn, uint8_t data);

#ifdef __cplusplus
}
#endif // defined __cplusplus

#endif //__N32XX_TSC_ALG_API__
