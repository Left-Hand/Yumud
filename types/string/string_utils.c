#include "string_utils.h"

void reverse_str(char * str, size_t len){
	if(len == 0) return;

	len -= 1;
	for(size_t i = 0; i < len / 2 + (len % 2); i++){
		char temp = str[i];
		str[i] = str[len - i];
		str[len - i] = temp;
	}
	str[len + 1] = '\0';
}

void itoa(int value,char *str,uint8_t radix)  
{
    int sign = 0;  
    int i=0;  
    if(value < 0){  
        sign = -1;  
        value = -value;  
    }
    do {  
        if(value%radix>9)  
            str[i] = value%radix +'0'+7;  
        else   
            str[i] = value%radix +'0';  
        i++;  
    } while((value/=radix)>0);  
    
    if(sign<0) {
        str[i] = '-';
        i++;
    } 

    reverse_str(str, i);
}

void itoas(int value,char *str,uint8_t radix, uint8_t size)  
{
	uint8_t i = 0;
    value = abs(value);
	do{  
		if(value%radix>9)  
			str[i] = value%radix +'0'+7;  
		else   
			str[i] = value%radix +'0';  
		i++;  
	}while((value/=radix)>0 && i < size);  
	
	reverse_str(str, size);
}



void iltoa(long long value,char *str,uint8_t radix)  
{
    int sign = 0;  
    int i=0;  
    if(value < 0){  
        sign = -1;  
        value = -value;  
    }
    do {  
        if(value%radix>9)  
            str[i] = value%radix +'0'+7;  
        else   
            str[i] = value%radix +'0';  
        i++;  
    } while((value/=radix)>0);  
    
    if(sign<0) {
        str[i] = '-';
        i++;
    } 

    reverse_str(str, i);
}

void iultoa(unsigned long long value,char *str,uint8_t radix)  
{
    int sign = 0;  
    int i=0;  
    if(value < 0){  
        sign = -1;  
        value = -value;  
    }
    do {  
        if(value%radix>9)  
            str[i] = value%radix +'0'+7;  
        else   
            str[i] = value%radix +'0';  
        i++;  
    } while((value/=radix)>0);  
    
    if(sign<0) {
        str[i] = '-';
        i++;
    } 

    reverse_str(str, i);
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