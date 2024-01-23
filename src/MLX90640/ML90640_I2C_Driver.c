#include "MLX90640_I2C_Driver.h"

void MLX90640_I2CInit(void){
    I2C2_Init(MLX90640_ADDR, (uint16_t)400000);
}


int MLX90640_I2CRead(uint8_t slaveAddr, uint16_t startAddress, uint16_t nMemAddressRead, uint16_t *data)
{

	uint8_t* bp = (uint8_t*) data;

    int ack = 0;                               
    int cnt = 0;
    
    ack = I2C_Mem_Read(I2C2, (slaveAddr<<1), startAddress, I2C_MEMADD_SIZE_16BIT, bp, nMemAddressRead*2, 500);

    if (ack != 0)
    {
        return -1;
    }
    
    for(cnt=0; cnt < nMemAddressRead*2; cnt+=2) {
    	uint8_t tmpbytelsb = bp[cnt+1];
    	bp[cnt+1] = bp[cnt];
    	bp[cnt] = tmpbytelsb;
    }

    return 0;   
} 

int MLX90640_I2CWrite(uint8_t slaveAddr, uint16_t writeAddress, uint16_t data)
{

    uint8_t sa;
    int ack = 0;
    uint8_t cmd[2];
    static uint16_t dataCheck;

    sa = (slaveAddr << 1);

    cmd[0] = data >> 8;
    cmd[1] = data & 0x00FF;

    ack = I2C_Mem_Write(I2C2, sa, writeAddress, I2C_MEMADD_SIZE_16BIT, cmd, sizeof(cmd), 500);

    if (ack != 0)
    {
        return -1;
    }         
    
    MLX90640_I2CRead(slaveAddr,writeAddress,1, &dataCheck);
    
    if ( dataCheck != data)
    {
        return -2;
    }    
    
    return 0;
}

