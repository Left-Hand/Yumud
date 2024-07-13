#include "../sys/core/system.hpp"
#include "../sys/kernel/clock.h"
#include "../types/real.hpp"
#include "../hal/bus/bus_inc.h"
#include "../hal/timer/instance/timer_hw.hpp"
#include "../hal/adc/adcs/adc1.hpp"

#define NOP_DELAY(N) asm volatile(".rept " #N "\n\t nop \n\t .endr \n\t":::)

struct MotorPosition{
    real_t lapPositionHome = real_t(0);
    real_t lapPosition = real_t(0);
    real_t lapPositionLast = real_t(0);

    int16_t accTurns = 0;
    real_t accPosition = real_t(0);
    real_t accPositionLast = real_t(0);

    real_t vel = real_t(0);
    real_t accPositionAgo = real_t(0);
    real_t elecRad = real_t(0);

}motorPosition;

enum class CaliProgress:uint8_t{
    checkMotor,
    forwardStop,
    forwardAccel,
    forwardPrepare,
    forwardInit,
    forwardRunning,
    backwardStop,
    backwardPrepare,
    backwardInit,
    backwardRunning,
    locate,
    relocate,
    done
}caliProgress;
int16_t Calibrattion_Val;

using Sys::t;
// constexpr int pwmFreq = 67000;
// constexpr auto adc_sample_cycles = ADC_SampleTime_28Cycles5;
// constexpr float sample_ticks = -14;
// constexpr float dutyScale = 0.55f;
// constexpr float mk = 0.17;

// constexpr int pwmFreq = 67000;
// constexpr auto adc_sample_cycles = ADC_SampleTime_28Cycles5;
// constexpr float sample_ticks = -28;
// constexpr float dutyScale = 0.6f;
// constexpr float mk = 0.42f;

// constexpr int pwmFreq = 63000;
// constexpr auto adc_sample_cycles = ADC_SampleTime_28Cycles5;
// constexpr float sample_ticks = -24;
// constexpr float dutyScale = 0.3f;
// constexpr float mk = 0.38f;

// constexpr int pwmFreq = 32000;
// constexpr auto adc_sample_cycles = ADC_SampleTime_28Cycles5;
// constexpr float sample_ticks = -15;
// constexpr float dutyScale = 0.3f;
// constexpr float mk = 0.3f;

constexpr int pwmFreq = 73000;
constexpr auto adc_sample_cycles = ADC_SampleTime_13Cycles5;
constexpr float sample_ticks = -10.5;
constexpr float dutyScale = 0.5f;

constexpr int focFreq = 10000;
constexpr float focDelta = (1.0f / float(focFreq));
constexpr int psc = 1;
constexpr int pwmArr = 72000000 / psc / pwmFreq - 1;
constexpr int focArr = 72000000 / psc / focFreq - 1;
constexpr int poles = 7;
constexpr bool rsv = true;

constexpr uint8_t over_sample_times = 8;
constexpr uint8_t over_sample_mask = over_sample_times - 1;

constexpr double sample_time = sample_ticks / (144000000.0 / 8.0);
constexpr double window_time =  2.0 / 1000000.0;
constexpr double arr_time = 1.0 / float(pwmFreq);

constexpr int offset_cvr = (sample_time / arr_time * pwmArr / 2);
constexpr int window_cvr = (window_time / arr_time * pwmArr);
constexpr float shunt_res = (0.03);
constexpr float shunt_magnification = 100 * over_sample_times;

real_t cur = real_t(0);
real_t uDuty, vDuty, wDuty;
real_t dDuty = real_t(0);
real_t qDuty = real_t(0);
real_t qCurrTarget, dCurrTarget;
real_t uCurr, vCurr, wCurr, alphaCurr, betaCurr,dCurr, qCurr;
real_t target = real_t(0);

real_t sixtant_theta;
real_t iDuty;
real_t modu_rad;
int modu_sect;

volatile uint8_t trig_sect;
volatile uint16_t trigStamps[4] = {0};
volatile int16_t currData[4] = {0};
volatile uint8_t trigProg = 0;

real_t adjMap[poles * 6];
real_t adjMapF[poles * 6];
real_t adjMapB[poles * 6];
real_t caliCurrent = real_t(0);
real_t limitCurrent = real_t(0);
real_t openLoopCurrent = real_t(0);

bool setCali = false;
bool isCali = false;

extern "C"{
// void TIM1_UP_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void TIM2_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void ADC1_2_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
}

int fsector(const real_t & x, const real_t & step, int sectors){
    return int(std::fmod((x / step),  real_t(sectors)));
}

void focMain();
void caliMain();

void GPIO_Init(){
    GPIO_InitTypeDef GPIO_InitStructure = {0};

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, ENABLE);
    
    //PA8 -> U
    //PA9 -> V
    //PA10 -> W
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    //PB3 SCLK
    //PB4 MISO
    //PB5 MOSI
    //PA15 CS
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3 | GPIO_Pin_5;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_15;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    //PA0 AIN
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);
    GPIO_PinRemapConfig(GPIO_Remap_SPI1, ENABLE);
}

void ADC1_Init(void)
{
    ADC_InitTypeDef  ADC_InitStructure = {0};

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
    RCC_ADCCLKConfig(RCC_PCLK2_Div8);

    ADC_DeInit(ADC1);
    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
    ADC_InitStructure.ADC_ScanConvMode = DISABLE;
    ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
    // ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T1_CC3;
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
    ADC_InitStructure.ADC_NbrOfChannel = 1;
    // ADC_InitStructure.ADC_Pga = ADC_Pga_4;
    ADC_Init(ADC1, &ADC_InitStructure);

    ADC_RegularChannelConfig(ADC1, ADC_Channel_TempSensor, 1, adc_sample_cycles);
    
    ADC_InjectedSequencerLengthConfig(ADC1, 1);
    ADC_InjectedChannelConfig(ADC1,ADC_Channel_0,1,adc_sample_cycles);

    ADC_ExternalTrigInjectedConvConfig(ADC1, ADC_ExternalTrigInjecConv_T1_CC4);
    // ADC_ExternalTrigInjectedConvEdgeConfig(ADC1,adcexternaltriginjeced);
    ADC_ExternalTrigInjectedConvCmd(ADC1, ENABLE);

    ADC_DMACmd(ADC1, DISABLE);
    
    // ADC_ClearITPendingBit(ADC1, ADC_IT_EOC);
    ADC_ClearITPendingBit(ADC1, ADC_IT_JEOC);
    // ADC_ITConfig(ADC1,ADC_IT_EOC, ENABLE);
    ADC_ITConfig(ADC1,ADC_IT_JEOC,ENABLE);//ENABLE INJECTED INTERRUPT
    
    // ADC_AutoInjectedConvCmd(ADC1, ENABLE);

    ADC_Cmd(ADC1, ENABLE);

    ADC_BufferCmd(ADC1, DISABLE); //disable buffer
    ADC_ResetCalibration(ADC1);
    while(ADC_GetResetCalibrationStatus(ADC1));
    ADC_StartCalibration(ADC1);
    while(ADC_GetCalibrationStatus(ADC1));
    Calibrattion_Val = Get_CalibrationValue(ADC1);

    ADC_BufferCmd(ADC1, ENABLE); //enable buffer

}

void ADC_IT_Init(){
    NVIC_InitTypeDef NVIC_InitStructure = {0};
    NVIC_InitStructure.NVIC_IRQChannel = ADC1_2_IRQn;  
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;//��ռ���ȼ�0��
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;  	   //�����ȼ�0��
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure); 
}

void TIM1_Init()
{
    TIM_OCInitTypeDef TIM_OCInitStructure={0};
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure={0};
 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE );
    TIM_InternalClockConfig(TIM1);

    TIM_TimeBaseInitStructure.TIM_Period = pwmArr;
    TIM_TimeBaseInitStructure.TIM_Prescaler = psc;
    TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_CenterAligned1;
    TIM_TimeBaseInit( TIM1, &TIM_TimeBaseInitStructure);    //���ú��ļ���������ģʽ
 
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;

    TIM_OC1Init( TIM1, &TIM_OCInitStructure );
    TIM_OC2Init( TIM1, &TIM_OCInitStructure );
    TIM_OC3Init( TIM1, &TIM_OCInitStructure );
    TIM_OC4Init( TIM1, &TIM_OCInitStructure );
 
    TIM_OC1PreloadConfig( TIM1, TIM_OCPreload_Enable);
    TIM_OC2PreloadConfig( TIM1, TIM_OCPreload_Enable);
    TIM_OC3PreloadConfig( TIM1, TIM_OCPreload_Enable);
    TIM_OC4PreloadConfig( TIM1, TIM_OCPreload_Disable);
 
    TIM_ARRPreloadConfig( TIM1, ENABLE );           //�������ļ������Զ���װ��

    TIM_CtrlPWMOutputs(TIM1, ENABLE );          //�߼���ʱ����λMOE
    TIM_Cmd( TIM1, ENABLE );       //�������ļ�����
}

void TIM2_Init()
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure={0};
	
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE );
    TIM_InternalClockConfig(TIM2);

    TIM_TimeBaseInitStructure.TIM_Period = focArr;
    TIM_TimeBaseInitStructure.TIM_Prescaler = psc;
    TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseInitStructure);    //���ú��ļ���������ģʽ

    TIM_ARRPreloadConfig(TIM2, ENABLE );           //�������ļ������Զ���װ��

    TIM_Cmd( TIM2, ENABLE );       //�������ļ�����
}


void TIM1_UP_INT_Init(){
    TIM_ClearFlag(TIM1, TIM_FLAG_Update);
    TIM_ITConfig(TIM1, TIM_IT_Update, ENABLE);

    NVIC_InitTypeDef NVIC_InitStructure = {0};
    NVIC_InitStructure.NVIC_IRQChannel = TIM1_UP_IRQn;  
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;//��ռ���ȼ�0��
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;  	   //�����ȼ�0��
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure); 
}

void TIM2_UP_INT_Init(){
    TIM_ClearFlag(TIM2, TIM_FLAG_Update);
    TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);

    NVIC_InitTypeDef NVIC_InitStructure = {0};
    NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;  
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;//��ռ���ȼ�0��
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;  	   //�����ȼ�0��
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure); 
}

void TIM1_CC_INT_Init(){
    TIM_ClearFlag(TIM1, TIM_FLAG_CC4);
    TIM_ITConfig(TIM1, TIM_FLAG_CC4, ENABLE);

    NVIC_InitTypeDef NVIC_InitStructure = {0};
    NVIC_InitStructure.NVIC_IRQChannel = TIM1_CC_IRQn;  
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;//��ռ���ȼ�0��
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;  	   //�����ȼ�0��
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure); 
}


void TIM1_SetPWMCVR(const uint16_t & uCVR, const uint16_t & vCVR, const uint16_t & wCVR){
    TIM1->CH1CVR = uCVR;
    TIM1->CH2CVR = vCVR;
    TIM1->CH3CVR = wCVR;
}

void TIM1_SetTGCVR(const uint16_t & tg){
    TIM1->CH4CVR = tg;
}

extern "C"{

void TIM2_CB(void){
	if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET){

        GPIOA->BCR = GPIO_Pin_12;

        t+=real_t(focDelta);

        if(isCali || setCali){
            caliMain();
        }else {
            focMain();
        }

        GPIOA->BSHR = GPIO_Pin_12;
		
        TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
    }   
}

void ADC_CB(void){

    if(ADC_GetITStatus(ADC1, ADC_IT_JEOC) != RESET){
        GPIOB->BCR = GPIO_Pin_8;

        static uint16_t tempData[4] = {0};
        static uint16_t last_cvr = 1;
        static uint16_t last_data = 0;

        uint8_t i = trigProg & 0b11;
        uint8_t j = (trigProg >> 2) & over_sample_mask;
        uint16_t this_cvr = (uint16_t)trigStamps[i];
        if(j == 0){
            currData[i] = tempData[i];
            tempData[i] = 0;
        }else{
            if(std::abs(this_cvr - last_cvr) < 0){
                tempData[i] += last_data;
            }else{
                last_data = std::clamp(ADC_GetInjectedConversionValue(ADC1, ADC_InjectedChannel_1) + Calibrattion_Val, 0, 4095);
                tempData[i] += last_data;
            }
        }

        last_cvr = (uint16_t)trigStamps[i];
        const uint16_t next_cvr = (uint16_t)trigStamps[(i + 1) & 0b11];
        TIM1_SetTGCVR(next_cvr);

        trigProg++;

        GPIOB->BSHR = GPIO_Pin_8;
        ADC_ClearITPendingBit(ADC1, ADC_IT_JEOC);
    }

    // GPIOA->BSHR = GPIO_Pin_12;
}
}


void SPI1_Init(){
	SPI_InitTypeDef  SPI_InitStructure;
	
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);

	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;  //����SPI�������˫�������ģʽ:SPI����Ϊ˫��˫��ȫ˫��
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;		//����SPI����ģʽ:����Ϊ��SPI
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_16b;		//����SPI�����ݴ�С:SPI���ͽ���8λ֡�ṹ
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;		//����ͬ��ʱ�ӵĿ���״̬Ϊ�ߵ�ƽ
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;	//����ͬ��ʱ�ӵĵڶ��������أ��������½������ݱ�����
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;		//NSS�ź���Ӳ����NSS�ܽţ�����������ʹ��SSIλ������:�ڲ�NSS�ź���SSIλ����
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_8;		//���岨����Ԥ��Ƶ��ֵ:������Ԥ��ƵֵΪ256
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;	//ָ�����ݴ����MSBλ����LSBλ��ʼ:���ݴ����MSBλ��ʼ
	SPI_InitStructure.SPI_CRCPolynomial = 7;	//CRCֵ����Ķ���ʽ
	SPI_Init(SPI1, &SPI_InitStructure);  //����SPI_InitStruct��ָ���Ĳ�����ʼ������SPIx�Ĵ���
	
	SPI_Cmd(SPI1, ENABLE); //ʹ��SPI����

    GPIOA->BCR = GPIO_Pin_15;
    while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);
    SPI_I2S_SendData(SPI1, 0);
    while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET);
    SPI_I2S_ReceiveData(SPI1);
    GPIOA->BSHR = GPIO_Pin_15;

}

uint16_t trans16(uint16_t data){
    uint16_t dataRx;

    GPIOA->BCR = GPIO_Pin_15;

	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);
    SPI_I2S_SendData(SPI1, data);
 
    while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET);
    dataRx =  SPI_I2S_ReceiveData(SPI1);
    // printf("%d\r\n", dataRx);

    GPIOA->BSHR = GPIO_Pin_15;
    return dataRx;
}

real_t MA730readLapPosition()
{
    uint16_t dataTx = 0;
    uint16_t dataRx = 0;

    dataRx = trans16(dataTx);
    real_t lapPosition = real_t(dataRx >> 2) / real_t(16384);

    return real_t(lapPosition);
}


void setUVWDuty(const real_t & uDutyTarget,const real_t & vDutyTarget,const real_t & wDutyTarget){
    // uDuty = std::clamp(uDuty, real_t(0), real_t(1));
    // vDuty = std::clamp(vDuty, real_t(0), real_t(1));
    // wDuty = std::clamp(wDuty, real_t(0), real_t(1));
    uDuty = uDutyTarget;
    vDuty = vDutyTarget;
    wDuty = wDutyTarget;

    uint16_t uCVR = int(uDutyTarget * real_t(pwmArr));
    uint16_t vCVR = int(vDutyTarget * real_t(pwmArr));
    uint16_t wCVR = int(wDutyTarget * real_t(pwmArr));


    TIM1_SetPWMCVR(uCVR, vCVR, wCVR);
}

void initTrigs(){
    trigStamps[0] = 1;
    trigStamps[1] = 1;
    trigStamps[2] = 1;
    trigStamps[3] = 1;
    trigProg = 0;
}

void setTrigs(const real_t & tgTwo,const real_t & tgOne){
    trigStamps[0] = pwmArr;
    trigStamps[1] = std::max(int(tgTwo * real_t(pwmArr)) + offset_cvr, 1);
    trigStamps[2] = std::max(int(tgOne * real_t(pwmArr)) + offset_cvr, 1);
    trigStamps[3] = 1;

    trig_sect = modu_sect;
}

void setDQDuty(const real_t & dDutyTarget,const real_t & qDutyTarget,const real_t & radTarget){
    dDuty = dDutyTarget;
    qDuty = qDutyTarget;

    // iDuty = std::sqrt(dDutyTarget * dDutyTarget + qDutyTarget * qDutyTarget);
    iDuty = std::clamp(iDuty, real_t(0), real_t(1));

    // modu_rad = radTarget + std::atan2(qDutyTarget, dDutyTarget);
    modu_sect = fsector(modu_rad, real_t(TAU / 6), 6) + 1;
    sixtant_theta = std::fmod(modu_rad, real_t(TAU / 6));
    
    // if(modu_rad > real_t(0))modu_sect = (int(modu_rad / real_t(TAU / 6))) % 6 + 1;
    // else modu_sect = (int(modu_rad / real_t(TAU / 6))) % 6 + 6;
    // real_t ta = iDuty * std::sin(sixtant_theta) * real_t(dutyScale);
    // real_t tb = iDuty * std::sin(real_t(TAU / 6) - sixtant_theta) * real_t(dutyScale);
    real_t ta = iDuty;
    real_t tb = iDuty;
    
    real_t t0 = (real_t(1) - ta - tb) / real_t(2);
    real_t t1 = (real_t(1) + ((modu_sect % 2 == 0 )? (tb - ta) : (ta - tb))) / real_t(2);
    real_t t2 = (real_t(1) + ta + tb) / real_t(2);
    switch (modu_sect){

    case 1:
        setUVWDuty(t2, t1, t0);
        break;
    case 2:
        setUVWDuty(t1, t2, t0);
        break;
    case 3:
        setUVWDuty(t0, t2, t1);
        break;
    case 4:
        setUVWDuty(t0, t1, t2);
        break;
    case 5:
        setUVWDuty(t1, t0, t2);
        break;
    case 6:
        setUVWDuty(t2, t0, t1);
        break;
    default:
        break;
    }

    setTrigs(std::mean(t1, t2), std::mean(t1, t0));
}

void setDQCurrent(const real_t & _dCurrTarget,const real_t & _qCurrTarget,const real_t & rad){
    const real_t kp = real_t(0.387f);
    const real_t maxDuty = real_t(1.0f);

    const real_t & theta = rad;
    // const real_t theta = real_t(0);
    // const real_t _s = std::sin(theta);
    // const real_t _c = std::cos(theta);
    real_t _c = theta;
    real_t _s = theta;

    dCurr = alphaCurr * _c + betaCurr * _s;
    qCurr = betaCurr * _c - alphaCurr * _s;


    static real_t _dDuty, _qDuty;

    _dDuty += kp * (_dCurrTarget - dCurr);
    _qDuty += kp * (_qCurrTarget - qCurr);

    _dDuty = std::clamp(_dDuty, real_t(0), maxDuty);
    _qDuty = std::clamp(_qDuty, -maxDuty, maxDuty);

    setDQDuty(_dDuty, _qDuty, rad);
}

int position2pole(iq_t & position){
    real_t pole = std::frac(position) * real_t(poles);
    return int(pole);
}

int position2poleSector(iq_t & position){
    real_t pole = std::frac(position) * real_t(poles * 6);
    return int(pole);
}


real_t position2rad(real_t & position){
    iq_t frac1 = std::fmod(position, real_t(1.0f / poles)) * real_t(poles);
    return real_t(TAU) * (std::frac(frac1));
}

real_t readLapPosition(){
    real_t undiredLapPostion = MA730readLapPosition();
    if (rsv) return real_t(real_t(1) - undiredLapPostion);
    else return real_t(undiredLapPostion);
    // else motorPosition.lapPosition = undiredLapPostion;
}

void locatePosition(){
    motorPosition.lapPosition = readLapPosition();
    motorPosition.lapPositionHome = motorPosition.lapPosition;
    motorPosition.lapPositionLast = motorPosition.lapPosition;
    motorPosition.accTurns = 0;
    motorPosition.accPosition = real_t(0);
    motorPosition.accPositionLast = motorPosition.accPosition;
}


void relocatePosition(real_t & offset){
    motorPosition.lapPositionHome += offset;
}

void updatePosition(){

    motorPosition.lapPosition = readLapPosition();

    real_t deltaLapPosition = motorPosition.lapPosition - motorPosition.lapPositionLast;

    if(deltaLapPosition > real_t(0.5f)){
        motorPosition.accTurns -= 1;
    }else if (deltaLapPosition < real_t(-0.5f)){
        motorPosition.accTurns += 1;
    }

    motorPosition.lapPositionLast = motorPosition.lapPosition;
    motorPosition.accPositionLast = motorPosition.accPosition;
    motorPosition.accPosition = real_t(motorPosition.accTurns) + (motorPosition.lapPosition - motorPosition.lapPositionHome);

    motorPosition.elecRad = position2rad(motorPosition.accPosition);
}

void focAlign(){
    setDQDuty(real_t(1), real_t(0), real_t(0));
    // Delay_Ms(300);
    delay(300);
    locatePosition();
}


void processMagSensor(){
    updatePosition();
}

inline real_t currData2curr(int16_t currData){
    return real_t(3.3 / 4096 / shunt_magnification / shunt_res * currData);
}

void processCurrentSensing(){

    const int16_t currDataThree = currData[0];
    const int16_t currDataTwo = currData[1];
    const int16_t currDataOne = currData[2];
    const int16_t currDataZero = currData[3];

    int16_t currDataFirst = currDataOne - currDataZero;
    int16_t currDataSecond = currDataTwo - currDataOne;
    int16_t currDataThird = currDataThree - currDataTwo;

    int16_t uCurrData, vCurrData, wCurrData;

    switch(trig_sect){
    
    case 1:
        uCurrData = currDataThird;
        vCurrData = currDataSecond;
        wCurrData = currDataFirst;
        break;

    case 2:
        uCurrData = currDataSecond;
        vCurrData = currDataThird;
        wCurrData = currDataFirst;
        break;

    case 3:
        uCurrData = currDataFirst;
        vCurrData = currDataThird;
        wCurrData = currDataSecond;
        break;

    case 4:
        uCurrData = currDataFirst;
        vCurrData = currDataSecond;
        wCurrData = currDataThird;
        break;

    case 5:
        uCurrData = currDataSecond;
        vCurrData = currDataFirst;
        wCurrData = currDataThird;
        break;

    case 6:
        uCurrData = currDataThird;
        vCurrData = currDataFirst;
        wCurrData = currDataSecond;
        break;

    default:
        uCurrData = 0;
        vCurrData = 0;
        wCurrData = 0;
    }

    uCurr = currData2curr(uCurrData);
    vCurr = currData2curr(vCurrData);
    wCurr = currData2curr(wCurrData);
    alphaCurr = (uCurr - std::mean(vCurr, wCurr)) * real_t(2.0 / 3.0);
    betaCurr = (vCurr - wCurr) * real_t(SQRT3/3);
}

void processController(){
    real_t omiga = real_t(poles * TAU * 0.02);
    // target = std::fmod(omiga * t, real_t(TAU));
    // target = real_t(TAU * 15) * std::sin(omiga * t);
    target = omiga * t;

    // target = real_t(0.2f);
    // const real_t & theta = target;
    
    // const real_t & theta = motorPosition.elecRad;
    // const real_t & pos = motorPosition.accPosition;
    // // const real_t & theta = target;
    // static PID cPid = PID(real_t(10), real_t(0), real_t(0));
    // cPid.setClamp(real_t(0.2f));
    // cur += cPid.update(target, motorPosition.accPosition);
    // cur = std::clamp(cur, real_t(-0.2f), real_t(0.2f));

    // static const real_t static_current = real_t(0.05);
    // static const real_t swap_k = real_t(60);
    // real_t temp1 = cur * swap_k;
    // real_t temp2 = temp1 * temp1;
    // real_t temp3 = temp2 * temp2;
    // // real_t dcur = static_current / (temp3 + real_t(1));
    // real_t dCur = real_t(0);
    // qCurrTarget += real_t(0.02)*cPid.update(target, pos);
    // qCurrTarget = std::clamp(qCurrTarget, real_t(-0.4), real_t(0.4));

    // dCurrTarget = std::cos(omiga * t) * real_t(0.07f) + real_t(0.12f);
    // qCurrTarget = std::sin(omiga * t) * real_t(0.07f) + real_t(0.12f);
    // qCurrTarget = real_t(0.2);
        // setDQCurrent(real_t(0), qCurrTarget, theta);
    // setDQCurrent(real_t(0.0), real_t(-0.2), theta);
    // setDQCurrent(real_t(0.0), real_t(-0.2), theta);
    // setDQCurrent(real_t(0.0), real_t(0), real_t(0));
    // setDQCurrent(dCurrTarget, qCurrTarget, real_t(0));
    // setDQCurrent(dcur,cur, motorPosition.elecRad);
    // setDQCurrent(real_t(0), cur, motorPosition.elecRad);
    // setDQDuty(real_t(0), cur, motorPosition.elecRad);
    setDQDuty(real_t(0), real_t(0), position2rad(target));
}

void focMain(){
    // processMagSensor();
    // processCurrentSensing();
    // processController();
}



void processCaliProgress(){
    static int cnt = 0;

    real_t pos = readLapPosition();
    static real_t offset = real_t(0);

    if(setCali){
        cnt = 0;
        offset = real_t(0);
        isCali = true;
        setCali = false;
        caliProgress = CaliProgress::checkMotor;

    }else{
        cnt++;
    }

    const int micros = 128;

    constexpr int stop_ms = 200;
    constexpr int cnt_stop = stop_ms * (focFreq / 1000);

    constexpr int locate_ms = 100;
    constexpr int cnt_locate = locate_ms * (focFreq / 1000);

    constexpr int skipPoles = 4;
    constexpr int cnt_prepare = 6 * micros * skipPoles;

    static int forwardBeginPoleSector = 0;
    constexpr int forwardTurns = 3;
    constexpr int fowardPoles = forwardTurns * poles;
    constexpr int cnt_forward = 6 * micros * fowardPoles;

    static int backwardBeginPoleSector = 0;
    constexpr int backwardTurns = forwardTurns;
    constexpr int backwardPoles = backwardTurns * poles;
    constexpr int cnt_backward = 6 * micros * backwardPoles;

    switch(caliProgress){
        case CaliProgress::checkMotor:

            cnt = 0;
            caliProgress = CaliProgress::forwardStop;

        case CaliProgress::forwardStop:

            if(cnt < cnt_stop){
                setDQCurrent(real_t(0), caliCurrent * real_t(cnt + 1) / real_t(cnt_stop), real_t(0));
                
                break;
            }
            cnt -= cnt_stop;
            caliProgress = CaliProgress::forwardPrepare;

        case CaliProgress::forwardPrepare:

            if(cnt < cnt_prepare){
                real_t deltaRad = real_t(cnt + 1) * real_t(TAU / 6 / micros);

                setDQCurrent(real_t(0), caliCurrent, deltaRad);
                break;
            }
            cnt -= cnt_prepare;
            caliProgress = CaliProgress::forwardInit;

        case CaliProgress::forwardInit:

            forwardBeginPoleSector = position2pole(pos);
            cnt = 0;
            caliProgress = CaliProgress::forwardRunning;

        case CaliProgress::forwardRunning:

            if(cnt < cnt_forward){
                real_t deltaRad = real_t(cnt + 1) * real_t(TAU / 6 / micros);

                setDQCurrent(real_t(0), caliCurrent, deltaRad);

                if(cnt % micros == 0){
                    int deltaPoleSector = cnt / micros;
                    int poleSector = (forwardBeginPoleSector + deltaPoleSector + 6 * poles) % (6 * poles);

                    real_t spinPos = std::frac(real_t(deltaPoleSector) / real_t(6 * poles));
                    real_t errPos = pos - spinPos;
                    real_t rad = position2rad(errPos);
                    // printf("%d, %.3f\r\n", poleSector, float(rad));
                    adjMap[poleSector] += rad / real_t(forwardTurns + backwardTurns);
                }

                break;
            }

            cnt -= cnt_forward;
            caliProgress = CaliProgress::backwardStop;
        
        case CaliProgress::backwardStop:

            if(cnt < cnt_stop){
                real_t _prog = real_t(PI) * real_t(cnt + 1) / real_t(cnt_stop);
                // real_t _s = std::sin(_prog);
                real_t _s = _prog;
                real_t _c = _prog;
                // real_t _c = std::cos(_prog);
                
                setDQCurrent(caliCurrent * _s, caliCurrent * _c, real_t(0));
                break;
            }
            cnt -= cnt_stop;
            caliProgress = CaliProgress::backwardPrepare;
            // caliProgress = CaliProgress::done;
            // break;

        case CaliProgress::backwardPrepare:

            if(cnt < cnt_prepare){
                real_t deltaRad = real_t(cnt + 1) * real_t(TAU / 6 / micros);

                setDQCurrent(real_t(0), -caliCurrent, -deltaRad);
                break;
            }
            cnt -= cnt_prepare;
            caliProgress = CaliProgress::backwardInit;
            // break;

        case CaliProgress::backwardInit:

            backwardBeginPoleSector = position2pole(pos);
            cnt = 0;
            caliProgress = CaliProgress::backwardRunning;

        case CaliProgress::backwardRunning:


            if(cnt < cnt_backward){
                real_t deltaRad = real_t(cnt + 1) * real_t(TAU / 6 / micros);

                setDQCurrent(real_t(0), -caliCurrent, -deltaRad);

                if(cnt % micros == 0){
                    int deltaPoleSector = cnt / micros;
                    int poleSector = (backwardBeginPoleSector - deltaPoleSector + 6 * poles * 256) % (6 * poles);
                    
                    real_t spinPos = real_t(deltaPoleSector) / real_t(6 * poles);
                    real_t errPos = pos + spinPos;
                    real_t rad = position2rad(errPos);
                    
                    // printf("%d, %.3f\r\n", poleSector, float(rad));
                    adjMap[poleSector] += rad / real_t(forwardTurns + backwardTurns);
                }

                break;
            }

            cnt -= cnt_backward;
            caliProgress = CaliProgress::locate;

        case CaliProgress::locate:

            if(cnt < cnt_locate){
                setDQCurrent(real_t(0), -caliCurrent, real_t(0));
                break;
            }

            locatePosition();
            cnt -= cnt_locate;
            caliProgress = CaliProgress::done;

        case CaliProgress::relocate:

            if(cnt < cnt_locate){
                setDQCurrent(real_t(0), -caliCurrent, real_t(0));

                updatePosition();
                offset += motorPosition.accPosition;

                break;
            }

            offset = offset / real_t(cnt_locate);
            relocatePosition(offset);
            cnt -= cnt_locate;
            caliProgress = CaliProgress::done;

        case CaliProgress::done:
        default:
            for(int i = 0; i < poles * 6; i++){
            }
            isCali = false;
            setDQCurrent(real_t(0), real_t(0), real_t(0));      
    }
}

void caliMain(){
    processCurrentSensing();
    processCaliProgress();
}

int bldc_main(){
    


    RCC_PCLK1Config(RCC_HCLK_Div1);
    RCC_PCLK2Config(RCC_HCLK_Div1);

    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
    SystemCoreClockUpdate();

    // Delay_Init();
    // USART_Printf_Init(921600);

    GPIO_Init();
    SPI1_Init();
    TIM1_Init();

    initTrigs();
    TIM1_SetTGCVR(1);

    ADC1_Init();
    ADC_IT_Init();

    TIM2_Init();

    caliCurrent = real_t(0.1f);
    limitCurrent = real_t(0.4f);
    openLoopCurrent = limitCurrent;
    
    setCali = true;
    


    // RCC_ClocksTypeDef RCC_CLK;
	// RCC_GetClocksFreq(&RCC_CLK);//Get chip frequencies

    // printf("------------------------\r\n");
	// printf("../system Clock Source : %d\r\n", (int)RCC_GetSYSCLKSource());
	// printf("APB1/PCLK1 : %dHZ\r\n", (int)RCC_CLK.PCLK1_Frequency);
	// printf("APB2/PCLK2 : %dHZ\r\n", (int)RCC_CLK.PCLK2_Frequency);
	// printf("../sysCLK     : %dHZ\r\n", (int)RCC_CLK.SYSCLK_Frequency);
	// printf("HCLK       : %dHZ\r\n", (int)RCC_CLK.HCLK_Frequency);

    // focAlign();
    TIM2_UP_INT_Init();
    // for(real_t x = real_t(-30.0f); x < real_t(30.0f); x+=real_t(0.1f)){
    //     printf("%.3f, %.3f\r\n", float(x), float(testFunc(x)));
    //     Delay_Ms(5);
    // }
    uart1.println("hello");
    adc1.enableAutoInject();

    // timer2.bindCb(TimerUtils::IT::Update, TIM2_CB);
    TIM2_CB();
    // adc1.bindCb(AdcUtils::IT::JEOC,ADC_CB);
    ADC_CB();
    while(true){
        // printf("%.3f, %d\r\n", float(theta), modu_sect);/
        // printf("%.3f, %.3f, %.3f\r\n",float(motorPosition.elecRad), float(target), float(motorPosition.accPosition));
        // printf("%d, ", trigData[trigProg & 0b11]);

        // const int16_t currDataThree = currData[0];


        // printf("%.3f, %.3f, %.3f\r\n", float(cur), float(motorPosition.accPosition), float(target));
        // printf("%d, %d, %d, ", TIM1->CH1CVR, TIM1->CH2CVR, TIM1->CH3CVR);
        // printf("%.3f, %.3f, %d\r\n", float(target), float(modu_rad), modu_sect);
        // printf("%d, ", TIM1->CH4CVR);
        // printf("%d, %d, %d, %d\r\n", currData[0], currData[1], currData[2], currData[3]);
        // printf("%.2f, %.2f, %.2f, %.2f, %.2f, %.2f\r\n", float(uCurr), float(vCurr), float(wCurr), float(alphaCurr), float(betaCurr), float(std::atan2(betaCurr, alphaCurr)));
        // printf("%d, %d, %d, ", uCurrData, vCurrData, wCurrData);
        // printf("%.3f, %.3f, %.3f\r\n", float(real_t(TIM1->CH1CVR)/real_t(TIM1->ATRLR)), float(real_t(TIM1->CH2CVR)/real_t(TIM1->ATRLR)), float(real_t(TIM1->CH3CVR)/real_t(TIM1->ATRLR)));
        // printf("%d, %d, %d\r\n", currDataFirst, currDataFirst + currDataSecond, currDataThird);

        // printf("%d, %d\r\n", int(uCurrData - int(real_t(vCurrData + wCurrData) / real_t (2))), int(real_t(vCurrData - wCurrData) * real_t(SQRT3 / 2) ));
        // printf("%.2f, %.2f,", float(real_t(PI) - motorPosition.elecRad), float(-std::atan2(alphaCurr, betaCurr)));
        // printf("%.3f, %.3f, %.3f\r\n", float(uCurr), float(vCurr), float(wCurr));
        // printf("%.3f, %.3f, %.3f\r\n", float(uDuty), float(vDuty), float(wDuty));
        // printf("%.3f, %.3f, %.3f, %.3f, %.3f\r\n", float(dCurr), float(qCurr), float(dCurrTarget), float(qCurrTarget), float(std::sqrt(dCurr * dCurr + qCurr * qCurr)));
        // printf("%.3f, %.3f, %.3f\r\n", float(target), float(motorPosition.accPosition), float(qCurrTarget));
        // std::fmod(target - motorPosition.accPosition * real_t(poles * TAU) - real_t(PI), real_t(TAU))-  real_t(PI)
        // Delay_Ms(2);
        uart1.println(t);
        delay(2);
    }
}