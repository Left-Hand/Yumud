#include "string_utils.hpp"


void StringUtils::qtoa(const iq_t value, char * str, uint8_t eps){

	bool minus = value < 0;
    eps = MIN(eps, 5);
    auto abs_value = ABS((int32_t)value.value);

    uint32_t int_part = uint32_t(abs_value) >> GLOBAL_Q;
    uint32_t frac_part = uint32_t(abs_value) & ((1 << GLOBAL_Q )- 1);

    if(minus){
		str[0] = '-';
		StringUtils::itoa(int_part, str + 1, 10);
	}else{
		StringUtils::itoa(int_part, str, 10);

	}

	size_t end = strlen(str);

    if(eps){
		str[end] = '.';//add dot to seprate
		end += 1;//move to \0
		

        int32_t scale = 1;

        for(uint8_t i = 0; i < eps; i++){
            scale *= 10;
        }

        frac_part *= scale;
        frac_part >>= GLOBAL_Q;

        StringUtils::itoas(frac_part,str + end, 10, eps);
    }

}


void StringUtils::reverse_str(char * str, size_t len){
	if(len == 0) return;

	len -= 1;
	for(size_t i = 0; i < len / 2 + (len % 2); i++){
		char temp = str[i];
		str[i] = str[len - i];
		str[len - i] = temp;
	}
	str[len + 1] = '\0';
}

std::tuple<int, int, int> StringUtils::disassemble_fstr(const char * str, const size_t len){
    char * p = const_cast<char *>(str);

    int int_part = 0;
	int frac_part = 0;
	int scale = 1;
    bool minus = false;

    while(!((*p>='0'&& *p<='9')||*p=='+'||*p=='-'||*p=='.')&&*p!='\0'){
        p++;
    }

    if(*p=='\0'){
        int_part = 0;
        frac_part = 0;
	}else{
        if(*p=='-'){
            minus = true;
            p++;
        }

        if(*p=='+'){
            p++;
        }

        while(*p>='0'&&*p<='9'){
            int_part = int_part * 10 + (*p-'0');
            p++;
        }

        if(*p=='.'){
            p++;
            while(*p>='0'&&*p<='9'){
                frac_part = frac_part * 10 + (*p-'0');
                scale *= 10;
                p++;
			}
		}
	}

ret:
	if(minus){
		int_part = -int_part;
		frac_part = -frac_part;
	}

	return {int_part, frac_part, scale};
}

void StringUtils::itoa(int64_t value,char *str,uint8_t radix){
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

void StringUtils::itoas(int value,char *str,uint8_t radix, uint8_t size)  
{
	uint8_t i = 0;
    value = ABS(value);
	do{
		if(value%radix>9)
			str[i] = value%radix +'0'+7;
		else
			str[i] = value%radix +'0';
		i++;
	}while((value/=radix)>0 && i < size);
	for(;i< size; i++)str[i] = '0';
	reverse_str(str, size);
}

void StringUtils::iutoa(uint64_t value,char *str,uint8_t radix)
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



void StringUtils::ftoa(float number,char *buf, uint8_t eps)
{
    char str_int[12] = {0};
    char str_float[eps+1] = {0};

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

bool StringUtils::is_digit(const char chr){
    return chr >= '0' && chr <= '9';
}

bool StringUtils::is_numeric(const char* str, const size_t len) {
	bool hasDigit = false;
	bool hasDot = false;
	bool hasSign = false;

	for (size_t i = 0; str[i] != '\0'; i++) {
		char chr = str[i];
		if (is_digit(chr)) {
			hasDigit = true;
		} else if (chr == '.') {
			if (hasDot || !hasDigit) {
				return false; // 多个小数点或者小数点前没有数字，返回 false
			}
			hasDot = true;
		} else if (chr == '+' || chr == '-') {
			if (hasSign || hasDigit || hasDot) {
				return false; // 多个正负号或者正负号出现在数字之后，返回 false
			}
			hasSign = true;
		} else {
			return false; // 非数字、正负号或小数点，返回 false
		}
	}
	return hasDigit; // 至少包含一个数字才认为是合法数字
}

bool StringUtils::is_digit(const char * str, const size_t len){
    for(int i = 0; str[i]!= '\0'; i++){
        if(!is_digit(str[i])) return false;
    }
    return true;
}
int StringUtils::kmp_find(const char *src, const size_t src_len, const char *match, const size_t match_len) {
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

void StringUtils::str_replace(const char *src, const size_t src_len, const char *match, const char *replace, const size_t dst_len){
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


int StringUtils::atoi(const char * str, const size_t len) {
	int ret = 0;
	bool minus = false;

	for(size_t i = 0; i < len; i++){
		char chr = str[i];
	
		if(chr=='-'){
			minus = true;
		}

		while(chr>='0' and chr<='9'){
			ret *= 10;
			ret += chr - '0';
			if(ret < 0){
				ret= __INT_MAX__;
				break;
			}
		}
	}

	return minus ? (-ret) : ret;
}

float StringUtils::atof(const char * str, const size_t len) {
    auto [int_part, frac_part, scale] = disassemble_fstr(str, len);
    return(int_part + (float)frac_part / scale);
}

[[maybe_unused]] static void mystof(const char * str, const size_t len, int & int_part, int & frac_part, int & scale){
    scale = 1;
    bool minus = false;

	const size_t default_begin_index = len; 
	const size_t default_point_index = len;
	size_t begin_index = default_begin_index;
	size_t point_index = default_point_index;

	for(size_t i = 0; i < len; i++){
		char chr = str[i];
		
		if(begin_index == default_begin_index){
			if(StringUtils::is_digit(chr)){
				begin_index = i;
			}
		}
	
		if(chr=='\0'){
			int_part = 0;
			frac_part = 0;
			return;
		}

		if(chr == '.'){
			point_index = i + 1;
            break;
		}

		if(chr=='-'){
			minus = true;
		}
	}

	if(point_index < len){
		frac_part = StringUtils::atoi(str + point_index , len - (point_index));
	}

	int_part = StringUtils::atoi(str + begin_index, len - (point_index + 1));


	for(size_t i = 0; i < len - point_index; i++){
		scale *= 10;
	}

	if(minus){
		int_part = -int_part;
		frac_part = -frac_part;
	}
}
