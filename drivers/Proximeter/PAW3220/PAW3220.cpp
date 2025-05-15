// https://github.com/dotdotchan/bs2x_sdk/blob/94121aa562f8f012d4194cb79f24ecd4665ae029/application/samples/products/ble_mouse/mouse_sensor/mouse_sensor_paw3220.c

// /**
//  * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved. \n
//  *
//  * Description: Test mouse sensor source \n
//  * Author: @CompanyNameTag \n
//  * History: \n
//  * 2023-2-09, Create file. \n
//  */

//  #include "osal_debug.h"
//  #include "osal_interrupt.h"
//  #include "stdbool.h"
//  #include "non_os.h"
//  #include "pinctrl.h"
//  #include "mouse_sensor_spi.h"
//  #include "mouse_sensor.h"
 
//  #define PIN_MOTION                  S_MGPIO21
//  #define SPI_RECV_DATA_LEN           1
//  #define SPI_SEND_DATA_LEN           2
//  #define PAW3220_DATA_BIT_LEN        12
//  #define SPI_THREE_WIRE_MODE         2
 
//  static const spi_mouse_cfg_t g_paw3220db_cfg[] = {
//      {WRITE, 0x09, 0x5A, NULL},
//      {WRITE, 0x4B, 0x00, NULL},
//      {WRITE, 0x5C, 0xD4, NULL},
//      {WRITE, 0x0D, 0x1A, NULL},
//      {WRITE, 0x0E, 0x1C, NULL},
//      {WRITE, 0x7F, 0x01, NULL},
//      {WRITE, 0x42, 0x4F, NULL},
//      {WRITE, 0x43, 0x93, NULL},
//      {WRITE, 0x44, 0x48, NULL},
//      {WRITE, 0x45, 0xF2, NULL},
//      {WRITE, 0x47, 0x4F, NULL},
//      {WRITE, 0x48, 0x93, NULL},
//      {WRITE, 0x49, 0x48, NULL},
//      {WRITE, 0x4A, 0xF3, NULL},
//      {WRITE, 0x64, 0x66, NULL},
//      {WRITE, 0x79, 0x08, NULL},
//      {WRITE, 0x7F, 0x00, NULL},
//      {WRITE, 0x09, 0x00, NULL},
//  };
 
//  static void paw3220_get_xy(int16_t *x, int16_t *y)
//  {
//      uint32_t ret = osal_irq_lock();
//      uint8_t motion = mouse_spi_read_reg(0x2);
//      if (!(motion & 0x80)) {
//          osal_printk("motion:0x%x\r\n", motion);
//          osal_irq_restore(ret);
//          *x = 0;
//          *y = 0;
//          return;
//      }
//      uint8_t _x = mouse_spi_read_reg(0x3);
//      uint8_t _y = mouse_spi_read_reg(0x4);
//      uint8_t xy = mouse_spi_read_reg(0x12);
//      /*
//      * Sensor combines 12-bit x and y data into three uint8_ts for transmission
//      * The higher four bits of recv_delta_xy are the higher four bits of the 12-bit x data
//      * The lower four bits of recv_delta_xy are the higher four bits of the 12-bit y data
//      */
 
//      int16_t temp_x =  (_x | ((xy & 0xf0) << 4));
//      int16_t temp_y =  (_y | ((xy & 0x0f) << 8));
//      *x = trans_to_16_bit((uint16_t)temp_x, PAW3220_DATA_BIT_LEN);
//      *y = -trans_to_16_bit((uint16_t)temp_y, PAW3220_DATA_BIT_LEN);
//      osal_irq_restore(ret);
//  }
 
//  static mouse_freq_t paw_3220_mouse_init(void)
//  {
//      mouse_sensor_spi_open(SPI_THREE_WIRE_MODE, 0, 0, 1);
//      mouse_sensor_spi_opration(g_paw3220db_cfg, sizeof(g_paw3220db_cfg) / sizeof(spi_mouse_cfg_t));
//      uint8_t pid = mouse_spi_read_reg(0);
//      osal_printk("pid0:0x%x\r\n", pid);
//      pid = mouse_spi_read_reg(1);
//      osal_printk("pid1:0x%x\r\n", pid);
//      return MOUSE_FREQ_2K;
//  }
 
//  mouse_sensor_oprator_t g_ble_paw3220_operator = {
//      .get_xy = paw3220_get_xy,
//      .init = paw_3220_mouse_init,
//  };
 
//  mouse_sensor_oprator_t ble_mouse_get_operator(void)
//  {
//      return g_ble_paw3220_operator;
//  }