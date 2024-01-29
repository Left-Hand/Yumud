#include "string_utils.h"

static void reverse(char *str) {
    int start = 0;
    int end = strlen(str) - 1;

    while (start < end) {
        char temp = str[start];
        str[start] = str[end];
        str[end] = temp;
        start++;
        end--;
    }
}

// void ftoa(float value, char *str, uint8_t eps) {
//     int8_t sign = (value >= 0) ? 1 : -1;
//     double abs_value = abs(value);
//     int32_t int_part = (int32_t)abs_value;

//     int i = 0;
//     do {
//         str[i++] = abs((int)(int_part % 10)) + '0';
//         int_part /= 10;
//     } while (int_part > 0);

//     if (sign < 0)
//         str[i++] = '-';

//     reverse(str);

//     if (eps > 0) {
//         str[i++] = '.';

//         double fraction_part = abs_value - (double)((int32_t)abs_value);
//         for (uint8_t j = 0; j < eps; j++) {
//             fraction_part *= 10;
//             int digit = (int)fraction_part;
//             str[i++] = digit + '0';
//             fraction_part -= digit;
//         }
//     }

//     str[i] = '\0';
// }

// void itoa(int value, char *str, uint8_t radix) {
//     int i = 0;
//     uint8_t is_negative = 0;

//     if (value == 0) {
//         str[i++] = '0';
//         str[i] = '\0';
//         return;
//     }

//     if (value < 0 && radix == 10) {
//         is_negative = 1;
//         value = -value;
//     }

//     while (value != 0) {
//         int remainder = value % radix;
//         str[i++] = (remainder > 9) ? (remainder - 10) + 'a' : remainder + '0';
//         value = value / radix;
//     }

//     if (is_negative) {
//         str[i++] = '-';
//     }

//     str[i] = '\0';

//     reverse(str);
// }


void itoa(int value,char *str,uint8_t radix)  
{
    int sign = 0;  
    //char *s = str;   
    char ps[32];  
    memset(ps,0,32);  
    int i=0;  
    if(value < 0)  
    {  
        sign = -1;  
        value = -value;  
    }  
    do  
    {  
        if(value%radix>9)  
            ps[i] = value%radix +'0'+7;  
        else   
            ps[i] = value%radix +'0';  
        i++;  
    }while((value/=radix)>0);  
    if(sign<0)  
        ps[i] = '-';  
    else  
        i--;  
    for(int j=i;j>=0;j--)  
    {  
        str[i-j] = ps[j];  
    }  
}

void ftoa(float number,char *buf, uint8_t eps)  
{
    long int_part;  
    double float_part;  
    char str_int[32];  
    char str_float[32];  
    memset(str_int,0,32);  
    memset(str_float,0,32);  
    int_part = (long)number;  
    float_part = number - int_part;  
    // 整数部分处理   
    itoa(int_part,str_int,10);  
    // 小数部分处理   
    if(eps>0)  
    {  
        float scale = 1;
        for(uint8_t i = 0; i < eps; i++)
            scale *= 10;

        float_part =abs(scale*float_part);  
        itoa((int)float_part,str_float,10);
    }

    int i = strlen(str_int);  
    str_int[i] = '.';  
    strcat(str_int,str_float);  
    strcpy(buf,str_int);   
}

int stoi(const char * str) {
    return atoi(str);
}

float stof(const char * str) {
    return atof(str);
}