#include "pixels.hpp"


namespace NVCV2::Pixels{
    void conv(ImageWritable<RGB565>& dst, const ImageReadable<Grayscale>& src) {
        for (auto x = 0; x < MIN(dst.get_size().x, src.get_size().x); x++) {
            for (auto y = 0; y < MIN(dst.get_size().y, src.get_size().y); y++) {
                dst[Vector2i{x, y}] = src[Vector2i{x, y}];
            }
        }
    }

    void conv(ImageWritable<RGB565>& dst, const ImageReadable<Binary>& src) {
        for (auto x = 0; x < MIN(dst.get_size().x, src.get_size().x); x++) {
            for (auto y = 0; y < MIN(dst.get_size().y, src.get_size().y); y++) {
                dst[Vector2i{x, y}] = src[Vector2i{x, y}];
            }
        }
    }


    static UniqueRandomGenerator lcg;

    void dyeing(ImageWritable<Grayscale>& dst, const ImageReadable<Grayscale>& src){
        for (auto x = 0; x < MIN(dst.get_size().x, src.get_size().x); x++) {
            for (auto y = 0; y < MIN(dst.get_size().y, src.get_size().y); y++) {
                dst[Vector2i{x, y}] = lcg[src[Vector2i{x, y}]];
            }
        }
    }

    Grayscale dyeing(const Grayscale in){
        return lcg[(uint8_t)in];
    }

    auto dyeing(const ImageReadable<Grayscale>& src){
        Image<Grayscale> tmp{src.get_size()};
        dyeing(tmp, src);
        return tmp;
    }

    void binarization(ImageWritable<Binary>& dst, const ImageReadable<Grayscale>& src, const Grayscale threshold){
        for (auto x = 0; x < std::min(dst.get_size().x, src.get_size().x); x++) {
            for (auto y = 0; y < std::min(dst.get_size().y, src.get_size().y); y++) {
                dst[Vector2i{x, y}] = src[Vector2i{x, y}].to_bina(threshold);
            }
        }
    }

    Image<Binary> binarization(const ImageReadable<Grayscale>& src, const Grayscale threshold){
        Image<Binary> dst{src.get_size()};
        binarization(dst, src, threshold);
        return dst;
    }

    void ostu(Image<Binary>& dst, const Image<Grayscale>& src){
        const Vector2i size = src.get_size();
        std::array<int, 256> statics;
        statics.fill(0);

        std::array<int, 256> sum_map;	
        sum_map.fill(0);

        std::array<int, 256> cnt_map;	
        cnt_map.fill(0);
        
        for(int cnt = 0; cnt < size.x * size.y; cnt++){
            statics[src[cnt]]++;
            cnt++;
        }
        
        {
            int current_sum = 0;
            int current_cnt = 0;
            for(int i = 0; i < 256; i++){
                current_sum += statics[i] * i;
                current_cnt += statics[i];
                
                sum_map[i] = current_sum;
                cnt_map[i] = current_cnt;
            }
        }
        
        const int total_sum = sum_map[255];
        const int total_cnt = cnt_map[255];
        
        real_t p1 = real_t();
        
        int max_i = 0;
        real_t max_t = real_t(0);
        
        for(int i = 0; i < 256; i++){
            int current_sum = sum_map[i];
            int current_cnt = cnt_map[i];
            
            int remain_sum = total_sum - current_sum;
            int remain_cnt = total_cnt - current_cnt;
            real_t m1 = real_t(current_sum) / current_cnt;
            real_t m2 = real_t(remain_sum) / (remain_cnt);
            
            real_t t = p1 * (1 - p1) * (m1 - m2) * (m1 - m2);
            if(t > max_t){
                max_i = i;
                max_t = t;
            }
            
            p1 += real_t(1.0f / 256.0f);
        }

        binarization(dst, src, max_i);
    }


    void iter_threshold(Image<Binary>& dst, const Image<Grayscale>& src, const real_t & k, const real_t & eps){
        const Vector2i size = src.get_size();
        std::array<int, 256> statics;
        statics.fill(0);

        std::array<int, 256> sum_map;	
        sum_map.fill(0);

        std::array<int, 256> cnt_map;	
        cnt_map.fill(0);
        
        for(int cnt = 0; cnt < size.x * size.y; cnt++){
            statics[src[cnt]]++;
            cnt++;
        }
        
        {
            int current_sum = 0;
            int current_cnt = 0;
            for(int i = 0; i < 256; i++){
                current_sum += statics[i] * i;
                current_cnt += statics[i];
                
                sum_map[i] = current_sum;
                cnt_map[i] = current_cnt;
            }
        }
        
        const int total_sum = sum_map[255];
        const int total_cnt = cnt_map[255];

        int last_i = 0;
        real_t last_t = real_t(0);

        for(int i = 0; i < 256; i++){
            int current_sum = sum_map[i];
            int current_cnt = cnt_map[i];
            
            int remain_sum = total_sum - current_sum;
            int remain_cnt = total_cnt - current_cnt;
            real_t m1 = real_t(current_sum) / current_cnt;
            real_t m2 = real_t(remain_sum) / (remain_cnt);
            
            real_t t = m1 * k + m2 * (1 - k);
            if(ABS(t - last_t) < eps){
                last_i = i;
                last_t = t;
            }
        }

        binarization(dst, src, last_i);
    }

    void max_entropy(const Image<Grayscale>& src, const int thresh){
        const Vector2i size = src.get_size();
        float probability = 0.0; //概率
        float max_Entropy = 0.0; //最大熵
        int totalpix = size.x * size.y;

        Histogram hist;

        for(int cnt = 0; cnt < size.x * size.y; cnt++){
            hist[src[cnt]]++;
            cnt++;
        }

        for (int i = 0; i < 256; ++i){
    
            float HO = 0.0; //前景熵
            float HB = 0.0; //背景熵
    
            //计算前景像素数
            int frontpix = 0;
            for (int j = 0; j < i; ++j){
                frontpix += hist[j];
            }
            //计算前景熵
            for (int j = 0; j < i; ++j){
                if (hist[j] != 0){
                    probability = (float)hist[j] / frontpix;
                    HO = HO + probability*log(1/probability);
                }
            }
    
            //计算背景熵
            for (int k = i; k < 256; ++k){
                if (hist[k] != 0){
                    probability = (float)hist[k] / (totalpix - frontpix);
                    HB = HB + probability*log(1/probability);
                }
            }
    
            //计算最大熵
            if(HO + HB > max_Entropy){
                max_Entropy = HO + HB;
                // thresh = i + p;
            }
        }
 
    }
    int get_huang_fuzzy_threshold(Histogram hist){
        int X, Y;
        int First, Last;
        int Threshold = -1;
        real_t BestEntropy = 114514;
        real_t Entropy;
        //   找到第一个和最后一个非0的色阶值
        for (First = 0; First <int(hist.size()) && hist[First] == 0; First++) ;
        for (Last = hist.size() - 1; Last > First && hist[Last] == 0; Last--) ;

        if (First == Last) return First;                // 图像中只有一个颜色
        if (First + 1 == Last) return First;            // 图像中只有二个颜色

        // 计算累计直方图以及对应的带权重的累计直方图
        
        Histogram S;
        Histogram W;            // 对于特大图，此数组的保存数据可能会超出int的表示范围，可以考虑用long类型来代替
        S[0] = hist[0];
        for (Y = First > 1 ? First : 1; Y <= Last; Y++)
        {
            S[Y] = S[Y - 1] + hist[Y];
            W[Y] = W[Y - 1] + Y * hist[Y];
        }

        // 建立公式（4）及（6）所用的查找表
        // real_t[] Smu = new real_t[Last + 1 - First];
        std::array<real_t, 256> Smu;

        // DEBUG_VALUE(First);
        // DEBUG_VALUE(Last);

        for (Y = 1; Y < Last + 1 - First; Y++)
        {
            real_t mu = 1 / (1 + (real_t)Y / (Last - First));               // 公式（4）
            // DEBUG_VALUE(mu);
            Smu[Y] = -mu * log(mu) - (1 - mu) * log(1 - mu);      // 公式（6）
            // DEBUG_VALUE(Smu[Y]);
            // Smu[Y] = log(mu);
            // S
            // DEBUG_VALUE(Smu[Y]);
        }

        // DEBUG_TRAP("hi");
        // 迭代计算最佳阈值
        // DEBUG_VALUE(First);
        // DEBUG_VALUE(Last);
        for (Y = First; Y <= Last; Y++)
        {
            Entropy = 0;
            int mu = (int)round((real_t)W[Y] / S[Y]);             // 公式17
            for (X = First; X <= Y; X++)
                Entropy += Smu[ABS(X - mu)] * hist[X];
            mu = (int)round((real_t)(W[Last] - W[Y]) / (S[Last] - S[Y]));  // 公式18

            for (X = Y + 1; X <= Last; X++)
                Entropy += Smu[ABS(X - mu)] * hist[X];       // 公式8

            if (BestEntropy > Entropy){
                BestEntropy = Entropy;      // 取最小熵处为最佳阈值
                Threshold = Y;
                // DEBUG_VALUE(Y);
            }
        }
        
        // DEBUG_VALUE(Threshold);
        return Threshold;
    }

    int huang(Image<Binary>& dst, const Image<Grayscale>& src){
        // DEBUG_PRINT("huang");
        Histogram hist;
        hist.fill(0);
        auto size = dst.get_size();
            // DEBUG_VALUE(size);
        for(int i = 0; i < size.x * size.y; i++){
            // DEBUG_VALUE(src[i]);
            hist[src[i]]++;
        }
        // for(int i = 0; i < 256; i++){
            // DEBUG_PRINT(i, hist[i]);
        // }
        return get_huang_fuzzy_threshold(hist);
    }


    void inverse(Image<Grayscale>& src) {
        for (auto i = 0; i < src.get_size().x * src.get_size().y; i++) {
            src[i] = ~uint8_t(src[i]);
        }
    }

    void gamma(Image<Grayscale>& src, const real_t ga) {
        static real_t last_ga = 1;
        static std::array<Grayscale, 256> lut;

        if(ga != last_ga){
            last_ga = ga;
            lut.fill(0);
            for(int i = 0; i < 256; i++){
                lut[i] = std::clamp(uint8_t(pow(real_t(i)/256, ga)*256), uint8_t(0), uint8_t(255));
            }
        }

        for (auto i = 0; i < src.get_size().x * src.get_size().y; i++) {
            src[i] = lut[uint8_t(src[i])];
        }
    }



    void sum_with(Image<Grayscale> & src, Image<Grayscale>& op) {
        for (auto i = 0; i < src.get_size().x * src.get_size().y; i++) {
            src[i] = MIN((uint8_t)src[i] + (uint8_t)op[i], 255);
        }
    }

    void sub_with(Image<Grayscale> & src, Image<Grayscale>& op) {
        for (auto i = 0; i < src.get_size().x * src.get_size().y; i++) {
            src[i] = MAX((uint8_t)src[i] - (uint8_t)op[i], 0);
        }
    }


    void mask_with(Image<Grayscale> & src, const ImageReadable<Binary>& op) {
        for (auto i = 0; i < src.get_size().x * src.get_size().y; i++) {
            src[i] = (uint8_t)op[i] ? src[i] : Grayscale(0);
        }
    }
}

