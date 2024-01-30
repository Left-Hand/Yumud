#include "string_utils.h"

void itoa(int value,char *str,uint8_t radix)  
{
    int sign = 0;  
    char ps[12];  
    memset(ps,0,12);  
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

void itoas(int value,char *str,uint8_t radix, uint8_t size)  
{
	char ps[size + 1];  
	memset(ps,0,size + 1);  
	uint8_t i = 0;
	do{  
		if(value%radix>9)  
			ps[i] = value%radix +'0'+7;  
		else   
			ps[i] = value%radix +'0';  
		i++;  
	}while((value/=radix)>0);  
	
	for(;i<=size;i++) ps[i] = '0';
	
	for(uint8_t j = 0;j < size;j++)  
	{  
		str[j] = ps[size - j - 1];  
	}  
}



void iltoa(long long value,char *str,uint8_t radix)  
{
    int sign = 0;  
    char ps[22];  
    memset(ps,0,22);  
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

void iultoa(unsigned long long value,char *str,uint8_t radix)  
{
    int sign = 0;  
    char ps[22];  
    memset(ps,0,22);  
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
    char str_int[12];  
    char str_float[eps+1];  
    memset(str_int,0,12);  
    memset(str_float,0,eps+1);  

    long int_part = (long)number;  
    float float_part = number - (float)int_part;  

    itoa(int_part,str_int,10);  
 
    if(eps){  
        float scale = 1;
        for(uint8_t i = 0; i < eps; i++)
            scale *= 10;

        float_part *= scale;  
        itoas((int)(float_part + 0.5f),str_float, 10, eps);
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