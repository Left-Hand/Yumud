#include "bmi270.hpp"

void BMI270::init(){
    uint8_t return_state = 0;
    // readReg(IMU660RA_CHIP_ID);                                   // ��ȡһ���豸ID ���豸����ΪSPIģʽ
    do{
        if(getChipId() != valid_chipid)                                               // IMU660RA �Լ�
        {
            // �������������˶�����Ϣ ������ʾ����λ��������
            // ��ô���� IMU660RA �Լ������ʱ�˳���
            // ���һ�½�����û������ ���û������ܾ��ǻ���
            BMI270_DEBUG("imu660ra self check error.");
            return_state = 1;
            break;
        }
        writeReg(IMU660RA_PWR_CONF, 0x00);                       // �رո߼�ʡ��ģʽ
        delay(1);
        writeReg(IMU660RA_INIT_CTRL, 0x00);                      // ��ʼ��ģ����г�ʼ������
        writeRegs(IMU660RA_INIT_DATA, bmi270_config_file, sizeof(bmi270_config_file));   // ��������ļ�
        writeReg(IMU660RA_INIT_CTRL, 0x01);                      // ��ʼ�����ý���
        delay(20);
        uint8_t ret = 0;
        readReg(IMU660RA_INT_STA, ret);
        if(ret)                       // ����Ƿ��������
        {
            // �������������˶�����Ϣ ������ʾ����λ��������
            // ��ô���� IMU660RA ���ó�ʼ���ļ�������
            // ���һ�½�����û������ ���û������ܾ��ǻ���
            BMI270_DEBUG("imu660ra init error.");
            return_state = 1;
            break;
        }
        writeReg(IMU660RA_PWR_CTRL, 0x0E);                       // ��������ģʽ  ʹ�������ǡ����ٶȡ��¶ȴ�����
        writeReg(IMU660RA_ACC_CONF, 0xA7);                       // ���ٶȲɼ����� ����ģʽ �����ɼ� 50Hz  ����Ƶ��
        writeReg(IMU660RA_GYR_CONF, 0xA9);                       // �����ǲɼ����� ����ģʽ �����ɼ� 200Hz ����Ƶ��

        // IMU660RA_ACC_SAMPLE �Ĵ���
        // ����Ϊ 0x00 ���ٶȼ�����Ϊ ��2  g   ��ȡ���ļ��ٶȼ����ݳ��� 16384  ����ת��Ϊ������λ������ ��λ g(m/s^2)
        // ����Ϊ 0x01 ���ٶȼ�����Ϊ ��4  g   ��ȡ���ļ��ٶȼ����ݳ��� 8192   ����ת��Ϊ������λ������ ��λ g(m/s^2)
        // ����Ϊ 0x02 ���ٶȼ�����Ϊ ��8  g   ��ȡ���ļ��ٶȼ����ݳ��� 4096   ����ת��Ϊ������λ������ ��λ g(m/s^2)
        // ����Ϊ 0x03 ���ٶȼ�����Ϊ ��16 g   ��ȡ���ļ��ٶȼ����ݳ��� 2048   ����ת��Ϊ������λ������ ��λ g(m/s^2)
        switch(IMU660RA_ACC_SAMPLE_DEFAULT)
        {
            default:
            {
                BMI270_DEBUG("IMU660RA_ACC_SAMPLE_DEFAULT set error.");
                return_state = 1;
            }break;
            case IMU660RA_ACC_SAMPLE_SGN_2G:
            {
                writeReg(IMU660RA_ACC_RANGE, 0x00);
                accel_scaler = (real_t)16384;
            }break;
            case IMU660RA_ACC_SAMPLE_SGN_4G:
            {
                writeReg(IMU660RA_ACC_RANGE, 0x01);
                accel_scaler = (real_t)8192;
            }break;
            case IMU660RA_ACC_SAMPLE_SGN_8G:
            {
                writeReg(IMU660RA_ACC_RANGE, 0x02);
                accel_scaler = (real_t)4096;
            }break;
            case IMU660RA_ACC_SAMPLE_SGN_16G:
            {
                writeReg(IMU660RA_ACC_RANGE, 0x03);
                accel_scaler = (real_t)2048;
            }break;
        }
        if(1 == return_state)
        {
            break;
        }

        // IMU660RA_GYR_RANGE �Ĵ���
        // ����Ϊ 0x04 ����������Ϊ ��125  dps    ��ȡ�������������ݳ��� 262.4   ����ת��Ϊ������λ������ ��λΪ ��/s
        // ����Ϊ 0x03 ����������Ϊ ��250  dps    ��ȡ�������������ݳ��� 131.2   ����ת��Ϊ������λ������ ��λΪ ��/s
        // ����Ϊ 0x02 ����������Ϊ ��500  dps    ��ȡ�������������ݳ��� 65.6    ����ת��Ϊ������λ������ ��λΪ ��/s
        // ����Ϊ 0x01 ����������Ϊ ��1000 dps    ��ȡ�������������ݳ��� 32.8    ����ת��Ϊ������λ������ ��λΪ ��/s
        // ����Ϊ 0x00 ����������Ϊ ��2000 dps    ��ȡ�������������ݳ��� 16.4    ����ת��Ϊ������λ������ ��λΪ ��/s
        switch(IMU660RA_GYRO_SAMPLE_DEFAULT)
        {
            default:
            {
                BMI270_DEBUG("IMU660RA_GYRO_SAMPLE_DEFAULT set error.");
                return_state = 1;
            }break;
            case IMU660RA_GYRO_SAMPLE_SGN_125DPS:
            {
                writeReg(IMU660RA_GYR_RANGE, 0x04);
                gyro_scaler = (real_t)262.4;
            }break;
            case IMU660RA_GYRO_SAMPLE_SGN_250DPS:
            {
                writeReg(IMU660RA_GYR_RANGE, 0x03);
                gyro_scaler = (real_t)131.2;
            }break;
            case IMU660RA_GYRO_SAMPLE_SGN_500DPS:
            {
                writeReg(IMU660RA_GYR_RANGE, 0x02);
                gyro_scaler = (real_t)65.6;
            }break;
            case IMU660RA_GYRO_SAMPLE_SGN_1000DPS:
            {
                writeReg(IMU660RA_GYR_RANGE, 0x01);
                gyro_scaler = (real_t)32.8;
            }break;
            case IMU660RA_GYRO_SAMPLE_SGN_2000DPS:
            {
                writeReg(IMU660RA_GYR_RANGE, 0x00);
                gyro_scaler = (real_t)16.4;
            }break;
        }
        if(1 == return_state)
        {
            break;
        }
    }while(0);

    // return return_state;
    // {
    //     uint8_t chipid = getChipId();
    //     if(chipid != valid_chipid){
    //         BMI270_DEBUG("BAD CHIP");
    //         return;
    //     }
    // }

    // writeReg(0x7c, 0x00);
    // delayMicroseconds(450);
    // writeReg(0x59, 0x00);
    // writeReg(0x59, 0x01);
    // writeReg(0x7d, 0x0e);
    // writeReg(0x40, 0xA8);
    // writeReg(0x42, 0xA9);
    // writeReg(0x7c, 0x02);

}