#include "pixels.hpp"
#include "core/math/realmath.hpp"

namespace ymd::nvcv2::pixels{

    class UniqueRandomGenerator {
    private:
        static constexpr uint8_t m = 251;
        static constexpr uint8_t a = 37;
        static constexpr uint8_t c = 71;
        std::array<uint8_t, 256> data;

        static constexpr uint8_t iter(const uint8_t x){
            return (a * x + c) % m;
        }
    public:
        UniqueRandomGenerator() {
            // Initialize with unique random numbers using LCG
            std::array<bool, 256> used{};
            used.fill(0);
            // uint8_t x = 0; // Initial seed
            uint8_t x_next = iter(0);
            for (size_t i = 0; i < 256; ++i) {
                // Find next unique number
                uint8_t x_new = x_next;
                // while(used[x_new]){
                    x_new = iter(x_new);
                // }
                x_next = x_new;
                // Mark x as used and assign to data[i]
                used[x_next] = true;
                data[i] = x_next;
            }
            data[0] = 0;
        }

        uint8_t operator[](const uint8_t index) const{
            return data[index];
        }
    };

    void conv(Image<RGB565>& dst, const Image<Gray>& src) {
        for (auto x = 0u; x < MIN(dst.size().x, src.size().x); x++) {
            for (auto y = 0u; y < MIN(dst.size().y, src.size().y); y++) {
                dst[Vec2u{x, y}] = color_cast<RGB565>(src[Vec2u{x, y}]);
            }
        }
    }

    void conv(Image<RGB565>& dst, const Image<Binary>& src) {
        for (auto x = 0u; x < MIN(dst.size().x, src.size().x); x++) {
            for (auto y = 0u; y < MIN(dst.size().y, src.size().y); y++) {
                dst[Vec2u{x, y}] = color_cast<RGB565>(src[Vec2u{x, y}]);
            }
        }
    }


    static UniqueRandomGenerator lcg;

    void dyeing(Image<Gray>& dst, const Image<Gray>& src){
        for (auto x = 0u; x < MIN(dst.size().x, src.size().x); x++) {
            for (auto y = 0u; y < MIN(dst.size().y, src.size().y); y++) {
                dst[Vec2u{x, y}] = Gray::from_u8(lcg[src[Vec2u{x, y}].to_u8()]);
            }
        }
    }

    Gray dyeing(const Gray in){
        return Gray::from_u8(lcg[in.to_u8()]);
    }

    auto dyeing(const Image<Gray>& src){
        Image<Gray> tmp{src.size()};
        dyeing(tmp, src);
        return tmp;
    }

    void binarization(Image<Binary>& dst, const Image<Gray>& src, const Gray threshold){
        for (auto x = 0u; x < std::min(dst.size().x, src.size().x); x++) {
            for (auto y = 0u; y < std::min(dst.size().y, src.size().y); y++) {
                dst[Vec2u{x, y}] = src[Vec2u{x, y}].to_binary(threshold);
            }
        }
    }

    Image<Binary> binarization(const Image<Gray>& src, const Gray threshold){
        Image<Binary> dst{src.size()};
        binarization(dst, src, threshold);
        return dst;
    }

    void ostu(Image<Binary>& dst, const Image<Gray>& src){
        const Vec2u size = src.size();
        std::array<uint, 256> statics;
        statics.fill(0);

        std::array<uint, 256> sum_map;	
        sum_map.fill(0);

        std::array<uint, 256> cnt_map;	
        cnt_map.fill(0);
        
        for(auto cnt = 0u; cnt < size.x * size.y; cnt++){
            statics[src[cnt].to_u8()]++;
            cnt++;
        }
        
        {
            int current_sum = 0;
            int current_cnt = 0;
            for(size_t i = 0; i < 256; i++){
                current_sum += statics[i] * i;
                current_cnt += statics[i];
                
                sum_map[i] = current_sum;
                cnt_map[i] = current_cnt;
            }
        }
        
        const uint total_sum = sum_map[255];
        const uint total_cnt = cnt_map[255];
        
        real_t p1 = 0;
        
        int max_i = 0;
        real_t max_sep = 0;
        
        for(size_t i = 0; i < 256; i++){
            const int current_sum = sum_map[i];
            const int current_cnt = cnt_map[i];
            
            const int remain_sum = total_sum - current_sum;
            const int remain_cnt = total_cnt - current_cnt;
            const real_t m1 = real_t(current_sum) / current_cnt;
            const real_t m2 = real_t(remain_sum) / remain_cnt;
            
            real_t t = p1 * (1 - p1) * (m1 - m2) * (m1 - m2);
            if(t > max_sep){
                max_i = i;
                max_sep = t;
            }
            
            p1 += real_t(1.0f / 256.0f);
        }

        binarization(dst, src, Gray::from_u8(max_i));
    }


    void iter_threshold(
            Image<Binary>& dst, 
            const Image<Gray>& src, 
            const real_t k, 
            const real_t eps
    ){
        const Vec2u size = src.size();
        std::array<int, 256> statics;
        statics.fill(0);

        std::array<int, 256> sum_map;	
        sum_map.fill(0);

        std::array<int, 256> cnt_map;	
        cnt_map.fill(0);
        
        for(auto cnt = 0u; cnt < size.x * size.y; cnt++){
            statics[src[cnt].to_u8()]++;
            cnt++;
        }
        
        {
            int current_sum = 0;
            int current_cnt = 0;
            for(size_t i = 0; i < 256; i++){
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

        for(size_t i = 0; i < 256; i++){
            int current_sum = sum_map[i];
            int current_cnt = cnt_map[i];
            
            int remain_sum = total_sum - current_sum;
            int remain_cnt = total_cnt - current_cnt;
            real_t m1 = real_t(current_sum) / current_cnt;
            real_t m2 = real_t(remain_sum) / remain_cnt;
            
            real_t t = m1 * k + m2 * (1 - k);
            if(ABS(t - last_t) < eps){
                last_i = i;
                last_t = t;
            }
        }

        binarization(dst, src, Gray::from_u8(last_i));
    }

    void calc_max_entropy(const Image<Gray>& src, const int thresh){
        const Vec2u size = src.size();
        float probability = 0.0; //概率
        float max_Entropy = 0.0; //最大熵
        int totalpix = size.x * size.y;

        Histogram hist;

        for(uint cnt = 0; cnt < size.x * size.y; cnt++){
            hist[src[cnt].to_u8()]++;
            cnt++;
        }

        for (size_t i = 0; i < 256; ++i){
    
            float HO = 0.0; //前景熵
            float HB = 0.0; //背景熵
    
            //计算前景像素数
            int frontpix = 0;
            for (uint j = 0; j < i; ++j){
                frontpix += hist[j];
            }
            //计算前景熵
            for (uint j = 0; j < i; ++j){
                if (hist[j] != 0){
                    probability = (float)hist[j] / frontpix;
                    HO = HO + probability*float(log(real_t(1)/real_t::from(probability)));
                }
            }
    
            //计算背景熵
            for (uint k = i; k < 256; ++k){
                if (hist[k] != 0){
                    probability = (float)hist[k] / (totalpix - frontpix);
                    HB = HB + probability*float(log(1/real_t::from(probability)));
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
        real_t Entropy = 0;
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
        std::array<real_t, 256> Smu = {0};

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

    int huang(Image<Binary>& dst, const Image<Gray>& src){
        // DEBUG_PRINT("huang");
        Histogram hist;
        hist.fill(0);
        auto size = dst.size();
            // DEBUG_VALUE(size);
        for(size_t i = 0u; i < size.x * size.y; i++){
            // DEBUG_VALUE(src[i]);
            hist[src[i].to_u8()]++;
        }
        // for(size_t i = 0; i < 256; i++){
            // DEBUG_PRINT(i, hist[i]);
        // }
        return get_huang_fuzzy_threshold(hist);
    }


    void inverse(Image<Gray>& src) {
        for (size_t i = 0u; i < src.size().x_mul_y(); i++) {
            src[i] = Gray::from_u8(~src[i].to_u8());
        }
    }

    void gamma(Image<Gray>& src, const real_t ga) {
        static real_t last_ga = 1;
        static std::array<Gray, 256> lut;

        if(ga != last_ga){
            last_ga = ga;
            lut.fill(Gray::from_black());
            for(size_t i = 0; i < 256; i++){
                lut[i] = Gray::from_u8(CLAMP(
                    uint8_t(pow(real_t(i)/256, ga)*256), 
                    uint8_t(0), 
                    uint8_t(255))
                );
            }
        }

        for (size_t i = 0u; i < src.size().x_mul_y(); i++) {
            src[i] = lut[src[i].to_u8()];
        }
    }



    void sum_with(Image<Gray> & src, Image<Gray>& op) {
        for (size_t i = 0u; i < src.size().x_mul_y(); i++) {
            src[i] = Gray::from_u8(MIN(src[i].to_u8() + op[i].to_u8(), 255));
        }
    }

    void sub_with(Image<Gray> & src, Image<Gray>& op) {
        for (size_t i = 0u; i < src.size().x_mul_y(); i++) {
            src[i] = Gray::from_u8(MAX(src[i].to_u8() - op[i].to_u8(), 0));
        }
    }


    void mask_with(Image<Gray> & src, const Image<Binary>& op) {
        for (size_t i = 0u; i < src.size().x_mul_y(); i++) {
            src[i] = op[i].is_white() ? src[i] : Gray::from_black();
        }
    }
}

