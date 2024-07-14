#pragma once

#include "../nvcv2/nvcv2.hpp"

namespace NVCV2::Shape{
    namespace Cores{
        const int roberts_x[2][2] = {{-1, 0}, {0, 1}};
        const int roberts_y[2][2] = {{0, -1}, {1, 0}};
        const int prewiit_x[3][3] = {{-1, 0, 1}, {-1, 0, 1}, {-1, 0, 1}};
        const int prewiit_y[3][3] = {{-1, -1, -1}, {0, 0, 0}, {1, 1, 1}};
        const int sobel_x[3][3] = {{-1, 0, 1}, {-2, 0, 2}, {-1, 0, 1}};
        const int sobel_y[3][3] = {{-1, -2, -1}, {0, 0, 0}, {1, 2, 1}};
        const int scharr_x[3][3] = {{-3, 0, 3}, {-10, 0, 10}, {-3, 0, 3}};
        const int scharr_y[3][3] = {{-3, -10, -3}, {0, 0, 0}, {3, 10, 3}};
        const int laplacian_4[3][3] = {{0, -1, 0}, {-1, 4, -1}, {0, -1, 0}};
        const int laplacian_8[3][3] = {{-1, -1, -1}, {-1, 8, -1}, {-1, -1, -1}};
        const int gauss[3][3] = {{1, 1, 1}, {1, 2, 1}, {1, 1, 1}};
    }


    void convolution(ImageWritable<Grayscale> & dst, const ImageReadable<Grayscale> & src, const int core[3][3]);

    void gauss(ImageWritable<Grayscale> & dst, const ImageReadable<Grayscale> & src);
    void gauss(Image<Grayscale, Grayscale> src);
    __inline void sobel_x(ImageWritable<Grayscale> & dst, const ImageReadable<Grayscale> & src){convolution(dst, src, Cores::sobel_x);}
    __inline void sobel_y(ImageWritable<Grayscale> & dst, const ImageReadable<Grayscale> & src){convolution(dst, src, Cores::sobel_y);}
    __inline void scharr_x(ImageWritable<Grayscale> & dst, const ImageReadable<Grayscale> & src){convolution(dst, src, Cores::scharr_x);}
    __inline void scharr_y(ImageWritable<Grayscale> & dst, const ImageReadable<Grayscale> & src){convolution(dst, src, Cores::scharr_y);}

    void sobel_xy(Image<Grayscale, Grayscale> & dst, const ImageReadable<Grayscale> & src);

    void convolution(ImageWritable<Grayscale> & dst, const ImageReadable<Grayscale> & src, const int core[2][2]);

    void dilate(Image<Binary, Binary> & dst, const Image<Binary, Binary> & src);

    void dilate_xy(Image<Binary, Binary> & dst, const Image<Binary, Binary> & src);

    void dilate_y(Image<Binary, Binary> & dst, const Image<Binary, Binary> & src);

    void erosion(Image<Binary, Binary> & dst, const Image<Binary, Binary> & src);

    void erosion_x(Image<Binary, Binary> & dst, Image<Binary, Binary> & src);

    void dilate_x(Image<Binary, Binary> & dst, Image<Binary, Binary> & src);

    void erosion_y(Image<Binary, Binary> & dst, const Image<Binary, Binary> & src);

    void erosion_xy(Image<Binary, Binary> & dst, const Image<Binary, Binary> & src);

    void x4(ImageWritable<bool> & dst, const ImageReadable<bool> & src);

    void x4(ImageWritable<Grayscale> & dst, const ImageReadable<Grayscale> & src, const int m = 2);

    auto x4(const ImageReadable<Grayscale> & src, const int m = 2);

    void x2(Image<Grayscale, Grayscale> & dst, const Image<Grayscale, Grayscale> & src);

    auto x2(const Image<Grayscale, Grayscale> & src);

    void XN(Image<Binary, Binary> dst, const Image<Binary, Binary> & src, const int & m, const real_t & percent);

    void zhang_suen(Image<Binary, Binary> & dst,const Image<Binary, Binary> & src);

    void zhang_suen2(Image<Binary, Binary> & dst,const Image<Binary, Binary> & src);

    __inline void dilate(Image<Binary, Binary> & src){
        dilate(src, src);
    }

    __inline void dilate_xy(Image<Binary, Binary> & src){
        dilate_xy(src, src);
    }


    __inline void erosion(Image<Binary, Binary> & src){
        erosion(src, src);
    }

    __inline void erosion_xy(Image<Binary, Binary> & src){
        erosion_xy(src, src);
    }

    __inline void zhang_suen(Image<Binary, Binary> & src){
        zhang_suen(src, src);
    }

    __inline void zhang_suen2(Image<Binary, Binary> & src){
        zhang_suen2(src, src);
    }

    __inline void erosion_x(Image<Binary, Binary> & src){
        erosion_x(src, src);
    }
    
    __inline void erosion_y(Image<Binary, Binary> & src){
        erosion_y(src, src);
    }

    __inline void dilate_x(Image<Binary, Binary> & src){
        dilate_x(src, src);
    }


    __inline void dilate_y(Image<Binary, Binary> & src){
        dilate_y(src, src);
    }

    __inline void morph_open(Image<Binary, Binary> & dst, const Image<Binary, Binary> & src){
        Image<Binary, Binary> temp(dst.get_size());        
        erosion(temp, src);
        dilate(dst, temp);
    }

    __inline void morph_close(Image<Binary, Binary> & dst, const Image<Binary, Binary> & src){
        Image<Binary, Binary> temp(dst.get_size());        
        dilate(temp, src);
        erosion(dst, temp);
    }

    __inline void morph_close(Image<Binary, Binary> & src){
        morph_close(src, src);
    }

    __inline void morph_open(Image<Binary, Binary> & src){
        morph_open(src, src);
    }

    void convo_roberts_x(Image<Grayscale, Grayscale> & dst, Image<Grayscale, Grayscale> & src);

    void convo_roberts_xy(Image<Grayscale, Grayscale> & dst, Image<Grayscale, Grayscale> & src);

    __inline void convo_roberts_y(ImageWritable<Grayscale> & dst, const ImageReadable<Grayscale> & src){
        convolution(dst, src, Cores::roberts_y);
    }

    class Seed{
        public:            

            uint8_t warp_dir_index(const int8_t & _dir) const{
                if(_dir > 7){
                    return 0;
                }else if(_dir < 0){
                    return 7;
                }
                return _dir;
            }

            Vector2i forward(){
                m_jounrey ++;
                return run_pos += dirs[dir_index];
            }

            Vector2i backward(){
                m_jounrey--;
                return run_pos -= dirs[dir_index];
            }

            Vector2i drop() const{
                return run_pos + dirs[warp_dir_index(dir_index)];
            }

            uint8_t jounrey() const{
                return m_jounrey;
            }

            void reset(){
                run_pos = default_pos;
                m_jounrey = 0;
            }

            void move_to(const Vector2i& pos){
                run_pos = pos;
            }

            void spin(){
                dir_index = warp_dir_index(dir_index + (cw? -1 : 1));
            }

            void spin(const bool & not_inverse){
                dir_index = warp_dir_index(dir_index + (cw ^ (!not_inverse) ? -1 : 1));
            }

            bool is_cw()const{
                return cw;
            }

            Seed(const Vector2i & pos, const Direction & index = Direction::L, const bool _cw = true):default_pos(pos), dir_index((uint8_t)index), run_pos(pos), cw(_cw){;}

            // Seed(const Vector2i & pos)
            auto & operator = (const Vector2i & pos){
                run_pos = pos;
                m_jounrey = 0;
                return *this;
            }

            operator Vector2i()const{
                return run_pos;
                // m_jounrey = 0;
                // return *this;
            }

        protected:
            Vector2i default_pos;
            uint8_t dir_index;
            Vector2i run_pos;
            uint8_t m_jounrey = 0;
            const bool cw;

            static constexpr std::array<Vector2i, 8>dirs = {
                Vector2i{1,0},
                Vector2i{1,-1}, 
                Vector2i{0, -1}, 
                Vector2i{-1,-1},
                Vector2i{-1,0},
                Vector2i{-1,1},
                Vector2i{0,1},
                Vector2i{1,1}
            };
    };
    
    class CoastFinder{
    protected:
        using m_Image = ImageReadable<Binary>;

        const m_Image & src;
        Seed m_seed;
        uint32_t step_limit = 64;

    public:
        using Coast = sstl::vector<Vector2i, 80>;
        CoastFinder(const m_Image & image, const Seed & seed): src(image), m_seed(seed){;}

        Coast find(){
            Coast coast;
            Seed seed = m_seed;
            Vector2i p0;
            seed.reset();

            enum class SubStatus{
                APPROCH,
                TRACK
            };

            SubStatus status = SubStatus::APPROCH;

            while(seed.jounrey() < step_limit && src.has_point(seed)){
                switch(status){
                    case SubStatus::APPROCH:
                        seed.forward();
                        if(!src.has_point(seed)) break;

                        if(is_positive(seed)){
                            seed.backward();
                            
                            status = SubStatus::TRACK;
                        }
                        break;

                    case SubStatus::TRACK:
                        {
                            // if(Vector2i(seed) == p0) break;
                            static constexpr uint8_t spin_limit = 7;
                            int spins = 0;
                            if(is_edge(seed)){
                                while(is_edge(seed) && (spins < spin_limit)){
                                    seed.spin(true);
                                    spins++;
                                }
                            }else{
                                while((!is_edge(seed)) && (spins < spin_limit)){
                                    seed.spin(false);
                                    spins++;
                                }
                                seed.spin(true);
                            }

                            seed.forward();
                            if(src.has_point(seed)){
                                // if(coast.size() == 0) p0 = Vector2i(seed);
                                coast.push_back(seed);
                            }
                            break;
                        }
                    }
            }
            Coast ret;

            for(const auto & item : coast){
                if(src.has_point(item)){
                    ret.push_back(item);
                }else{
                    DEBUG_LOG("invalid point", item);
                }
            }

            return ret;
        }

        bool is_positive(const Vector2i & pos){
            return (uint8_t)src(pos);
        }

        bool is_edge(const Vector2i & pos, const Vector2i next_pos){
            // return (uint8_t)src(next_pos) - (uint8_t)src(pos) > (uint8_t)(Grayscale)(edge_threshold);
            return (!!src[next_pos]) && (!src[pos]); 
            // return ((uint8_t)src(next_pos) > positive_threshold) && ((uint8_t)src(pos) < positive_threshold);
        }

        bool is_edge(const Seed & seed){
            return is_edge(seed, seed.drop());
        }
    };

    
}
