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

void itoa(int64_t value,char *str,uint8_t radix)  
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
	for(;i<= size; i++)str[i] = '0';
	reverse_str(str, size);
}

void iutoa(uint64_t value,char *str,uint8_t radix)  
{
    int i=0;  

    do {  
        if(value%radix>9)  
            str[i] = value%radix +'0'+7;  
        else   
            str[i] = value%radix +'0';  
        i++;  
    } while((value/=radix)>0);  

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

	if(number < 0 && int_part == 0){
		str_int[0] = '-';
		itoa(int_part,str_int + 1,10); 
	}
	else itoa(int_part,str_int,10);   
 
    if(eps){  
        float scale = 1;
        for(uint8_t i = 0; i < eps; i++)
            scale *= 10;

        float_part *= scale;  
        itoas((int)(float_part),str_float, 10, eps);
    }

    int i = strlen(str_int);  
    str_int[i] = '.';  
    strcat(str_int,str_float);  
    strcpy(buf,str_int);   
}

int kmp_find(const char *src, const size_t src_len, const char *match, const size_t match_len) {
	size_t *table = (size_t *)malloc(match_len * sizeof(size_t));
	size_t i = 0, j = 1;
	table[0] = 0;
	while (j < match_len) {
		if (match[i] == match[j]) {
			table[j] = i + 1;
			i++; j++;
		} else {
			if (i != 0) {
				i = table[i - 1];
			} else {
				table[j] = 0;
				j++;
			}
		}
	}
	
	i = 0;
	j = 0;
	while (i < src_len && j < match_len) {
		if (src[i] == match[j]) {
			i++; j++;
		} else {
			if (j != 0) {
				j = table[j - 1];
			} else {
				i++;
			}
		}
	}
	
	free(table);
	
	if (j == match_len) {
		return i - j;
	} else {
		return -1;
	}
}

void str_replace(const char *src, const size_t src_len, const char *match, const char *replace, const size_t dst_len){
	char * find_ptr = (char *)src;
	size_t find_len = src_len;
	char * replace_ptr = (char *)match;
	
	while(1){
		int pos = kmp_find(find_ptr, find_len, replace_ptr, dst_len);
		if(pos < 0) break;
		else{
			find_ptr += pos;
			for(size_t i = 0; i < dst_len; i++){
				find_ptr[i] = replace[i];
			}
			find_ptr += dst_len;
		}
	}
}


int stoi(const char * str) {
    return atoi(str);
}

float stof(const char * str) {
    return atof(str);
}