#pragma once

#include"types/vector2.hpp"

//https://blog.csdn.net/shandianfengfan/article/details/130799228
#define RANSAC_K 2
 
//y=ax+b
void lineplofit(std::vector<ymd::Vector2<float>>& points_list, int points_num, float* a, float* b)
{
    float sum_x2 = 0.0;
    float sum_y = 0.0;
    float sum_x = 0.0;
    float sum_xy = 0.0;
 
 
    int num = points_num;
 
 
    int i;
    for (i = 0; i < num; ++i)
    {
        sum_x2 += points_list[i].x * points_list[i].x;
        sum_y += points_list[i].y;
        sum_x += points_list[i].x;
        sum_xy += points_list[i].x * points_list[i].y;
    }
 
 
    float tmp = num * sum_x2 - sum_x * sum_x;
    if (abs(tmp) > 0.000001f)
    {
        *a = (num * sum_xy - sum_x * sum_y) / tmp;
        *b = (sum_x2 * sum_y - sum_x * sum_xy) / tmp;
    }
    else
    {
        *a = 0;
        *b = 0;
    }
}

//获取0~n-1范围内的num个随机数
static void GetRansacRandomNum(int n, int num, int p[])
{
    int i = 0, j;
 
 
    int r = rand() % n;
    p[0] = r;
    i++;
 
 
    while (1)
    {
        int status = 1;
        r = rand() % n;
 
 
        for (j = 0; j < i; j++)
        {
            if (p[j] == r)
            {
                status = 0;
                break;
            }
        }
 
 
        if (status == 1)
        {
            p[i] = r;
            i++;
        }
 
 
        if (i == num)
            break;
    }
}
 
 
void RansacPolyfitLine(std::vector<ymd::Vector2<float>> p, int iter_num, float alpha, float* a, float* b)
{
    int r_idx[RANSAC_K];
 
 
    std::vector<ymd::Vector2<float>> pick_p;
 
 
    srand((unsigned)time(NULL));
 
 
    int max_inline_num = 0;
 
 
    std::vector<ymd::Vector2<float>> inline_p;
    std::vector<ymd::Vector2<float>> max_inline_p;
    std::vector<float> d_list;
 
 
    int n = p.size();
 
 
    for (int i = 0; i < iter_num; i++)   //总共迭代iter_num次
    {
        GetRansacRandomNum(n, RANSAC_K, r_idx);  //生成RANSAC_K个不重复的0~n-1的随机数
 
 
        pick_p.clear();
        //随机选择2个点
        for (int j = 0; j < RANSAC_K; j++)
        {       
            pick_p.push_back(p[r_idx[j]]);
        }
 
 
        float aa = 0, bb = 0;
        //使用以上随机选择的两个点来计算一条直线
        lineplofit(pick_p, RANSAC_K, &aa, &bb);
 
 
        float mind = 99999999.9f;
        float maxd = -99999999.9f;
        d_list.clear();
        //计算所有点到以上计算直线的距离，并记录最大最小距离
        for (int j = 0; j < n; j++)
        {
            float d = abs(aa * p[j].x - p[j].y + bb) / sqrt(aa * aa + 1.0f);
            d_list.push_back(d);
            mind = MIN(mind, d);
            maxd = MAX(maxd, d);
        }
        //根据0~1的α值和最大最小距离计算阈值
        float threld = mind + (maxd - mind) * alpha;
 
 
        inline_p.clear();
        for (int j = 0; j < n; j++)
        {
            //判断如果点距离小于阈值则将该点加入内点集合
            if (d_list[j] < threld)
            {               
                inline_p.push_back(p[j]);                          
            }
        }
        //判断如果以上内点集合的点数大于历史最大内点数，则替换历史最大内点数集合
        if (max_inline_num < inline_p.size())
        {
            max_inline_num = inline_p.size();
            max_inline_p.swap(inline_p);
        }
    }
    //判断如果历史最大内点数大于等于2，则使用历史最大内点数集合来计算直线
    if (max_inline_num >= RANSAC_K)
    {
        lineplofit(max_inline_p, max_inline_p.size(), a, b);
    }
    else  //否则RANSAC算法失败
    {
        *a = 0;
        *b = 0;
    }
}