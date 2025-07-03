#include "FT6336.hpp"

// https://www.iotword.com/23534.html

#define FT6636_DEBUG_EN

#ifdef FT6636_DEBUG_EN
#define FT6636_DEBUG(...) DEBUG_PRINTLN(__VA_ARGS__);
#define FT6636_PANIC(...) PANIC(__VA_ARGS__)
#define FT6636_ASSERT(cond, ...) ASSERT(cond, __VA_ARGS__)
#else
#define FT6636_DEBUG(...)
#define FT6636_PANIC(...)  PANIC_NSRC()
#define FT6636_ASSERT(cond, ...) ASSERT_NSRC(cond)
#endif

using namespace ymd;
using namespace ymd::drivers;

using Error = FT6336::Error;

static constexpr size_t CTP_MAX_TOUCH = 2;

typedef struct
{
    uint8_t ptNum                    : 4;
    uint8_t tpDown                   : 1;
    uint8_t tpPress                  : 1;
    uint8_t res                      : 2;
} Status_bit;

typedef struct
{
    uint16_t xpox[CTP_MAX_TOUCH];
    uint16_t ypox[CTP_MAX_TOUCH];
    union
    {
        uint8_t status;
        Status_bit status_bit;
    };
}stru_pos;

Result<size_t, Error> FT6336::get_touch_cnt(){
    // uint8_t buf[4];
    // uint8_t i = 0;
    // uint8_t set = FT_FALSE;;
    // uint8_t pointNub = 0;
    // static uint8_t cnt = 0;
    
    // if( touch_isOK == FT_FALSE )
    //     return set;
    
    // cnt++;
    // if((cnt%10)==0 || cnt<10)
    // {
    //     // read number of touch points 
    //     ft6336_RdReg(FT_REG_NUM_FINGER,&pointNub,1);

    //     pointNub= pointNub&0x0f;
    //     if( pointNub && (pointNub < 3) )
    //     {
    //         cnt=0;
    //         // read the point value 
    //         pPos->status_bit.tpDown = 1;
    //         pPos->status_bit.tpPress = 1;
    //         pPos->status_bit.ptNum = pointNub;
            
    //         for( i=0; i < CTP_MAX_TOUCH; i++)
    //         {

    //             ft6336_RdReg( touch_press_reg[i], buf, 4 );
    //             if( pPos->status_bit.ptNum )
    //             {
    //                 switch(lcddev.dir)
    //                 {
    //                     case 0:
    //                         pPos->xpox[i]=((uint16_t)(buf[0]&0X0F)<<8)+buf[1];
    //                         pPos->ypox[i]=((uint16_t)(buf[2]&0X0F)<<8)+buf[3];
    //                         break;
    //                     case 1:
    //                         pPos->ypox[i]=lcddev.height-(((uint16_t)(buf[0]&0X0F)<<8)+buf[1]);
    //                         pPos->xpox[i]=((uint16_t)(buf[2]&0X0F)<<8)+buf[3];
    //                         break;
    //                     case 2:
    //                         pPos->xpox[i]=lcddev.width-(((uint16_t)(buf[0]&0X0F)<<8)+buf[1]);
    //                         pPos->ypox[i]=lcddev.height-(((uint16_t)(buf[2]&0X0F)<<8)+buf[3]);
    //                         break;
    //                     case 3:
    //                         pPos->ypox[i] = ((uint16_t)(buf[0]&0X0F)<<8)+buf[1];
    //                         pPos->xpox[i] = lcddev.width-(((uint16_t)(buf[2]&0X0F)<<8)+buf[3]);
    //                         break;
    //                 } 
    //             }
    //         } 
            
    //         set = FT_TRUE;
    //         if( pPos->xpox[0]==0 && pPos->ypox[0]==0)
    //         {
    //             pPos->status = 0;
    //         }
    //     }
    // }
    
    // if( pPos->status_bit.ptNum == 0)
    // { 
    //     if( pPos->status_bit.tpDown )
    //     {   
    //         pPos->status_bit.tpDown = 0;
    //     }
    //     else
    //     { 
    //         pPos->xpox[0] = 0xffff;
    //         pPos->ypox[0] = 0xffff;
    //         pPos->status = 0;
    //     }
    // }
    
    // if( cnt>240 )
    //    cnt=10;
    
    // return set;

    return Err(Error::Unspecified);
};