#include "tb.h"
#include "../drivers/Display/Polychrome/ST7789/st7789.hpp"
#include "../drivers/Wireless/Radio/CH9141/CH9141.hpp"
#include "../drivers/Proximeter/VL53L0X/vl53l0x.hpp"
#include "../drivers/Camera/MT9V034/mt9v034.hpp"

#include "../nvcv2/pixels.hpp"
#include "../nvcv2/shape.hpp"
#include "../nvcv2/geometry.hpp"
#include "../nvcv2/two_pass.hpp"
#include "../nvcv2/flood_fill.hpp"

#include "../thirdparty/lodepng/lodepng.h"

using namespace ymd::nvcv2;

#ifdef CH32V30X




enum class TransType:uint8_t{
    GS_0,
    GS_1,
    GS_2,
    GS_3,
    RGB,
    STR
};

struct PieceHeader{
    uint16_t header;
    TransType trans_type;
};

struct ImagePieceUnit:public PieceHeader{
    uint32_t hash;
    uint8_t time_stamp;
    uint8_t size_x;
    uint8_t size_y;
    uint16_t data_index;
};


constexpr uint32_t hash_djb2_buffer(const uint8_t *p_buff, int p_len, uint32_t p_prev = 5381) {
	uint32_t hash = p_prev;

	for (int i = 0; i < p_len; i++) {
		hash = ((hash << 5) + hash) + p_buff[i]; /* hash * 33 + c */
	}

	return hash;
}

class Transmitter:public IOStream{
protected:
    scexpr size_t str_tx_buf_size = 512;
    scexpr size_t str_rx_buf_size = 512;
    scexpr size_t img_tx_buf_size = 1024;

public:
    enum class Type{
        IMG0,
        IMG1,
        IMG2,
        IMG3,
        STR,
    };

    IOStream & instance;
    Uart & logger = uart2;

    RingBuf<str_tx_buf_size> str_tx_buf;
    RingBuf<str_rx_buf_size> str_rx_buf;
    RingBuf<img_tx_buf_size> img_buf;

    void write(const char data) override{
        str_rx_buf.addData(data);
    }

    void read(char & data) override{
        data = str_rx_buf.getData();
    }

    using InputStream::read;

    size_t available() const override{
        return str_rx_buf.available();
    }

    size_t pending() const override{
        return str_tx_buf.available();
    }

protected:
    scexpr uint16_t header = 0x54A8;
    uint8_t time_stamp;
public:
    Transmitter(IOStream & _instance):instance(_instance){;}
    void sendBlockData(ImagePieceUnit & unit, const uint8_t * data_from, const size_t len){

        unit.header = header;
        unit.trans_type = TransType::GS_0;
        unit.hash = hash_djb2_buffer(data_from, len);
        unit.time_stamp = time_stamp;

        instance.write((const char *)&unit, sizeof(unit));
        instance.write((const char *)(data_from), len);
    }

    auto compress_png(const ImageWithData<Grayscale, Grayscale> & img){
        std::vector<uint8_t>buffer;
        lodepng::State state;
        lodepng::encode(buffer, (const uint8_t * )img.data.get(), img.get_size().x, img.get_size().y, state);
        return buffer;
    }

    void transmit(const ImageWithData<Grayscale, Grayscale> & img, const uint8_t index){
        constexpr size_t mtu = 180;
        const auto & img_size = img.get_size();
        size_t len = img_size.x * img_size.y;

        const uint8_t * buf = (const uint8_t *)img.data.get();
        uint8_t block_total = len / mtu + bool(len % mtu);
        uint8_t block_number = 0;

        while(true){

            uint16_t block_start = block_number * mtu;
            uint16_t block_end = block_start + std::min(len - block_start, mtu);

            ImagePieceUnit unit;
            unit.size_x = img_size.x;
            unit.size_y = img_size.y;
            unit.data_index = block_start;

            sendBlockData(unit, (const uint8_t *)buf + block_start, block_end - block_start);

            block_number++;

            if(block_number >= block_total){
                break;
            }

        }

        time_stamp++;
    }
};


void st77xx_tb(IOStream & logger, Spi & spi){
    auto & led = portC[14];
    auto & lcd_blk = portC[7];
    auto & light = portC[6];

    auto & lcd_cs = portD[6];
    auto & lcd_dc = portD[7];
    auto & dev_rst = portB[7];

    led.outpp();
    lcd_blk.outpp(1);
    light.outpp(1);

    spi.bindCsPin(lcd_cs, 0);
    spi.init(144000000);

    SpiDrv SpiDrvLcd = SpiDrv(spi, 0);
    DisplayerPhySpi SpiInterfaceLcd{SpiDrvLcd, lcd_dc, dev_rst};

    ST7789 tftDisplayer(SpiInterfaceLcd, Vector2i(135, 240));

    {//init tft
        tftDisplayer.init();
        tftDisplayer.setDisplayOffset({53, 40}); 
        tftDisplayer.setFlipX(true);
        tftDisplayer.setFlipY(true);
        tftDisplayer.setSwapXY(false);
        tftDisplayer.setFormatRGB(true);
        tftDisplayer.setFlushDirH(false);
        tftDisplayer.setFlushDirV(false);
        tftDisplayer.setInversion(true);
    }

    Painter<RGB565> painter = Painter<RGB565>();
    painter.bindImage(tftDisplayer);
    tftDisplayer.fill(RGB565::BLACK);

    I2cSw SccbSw(portD[2], portC[12]);
    SccbSw.init(400000);
    MT9V034 camera(SccbSw);

    [[maybe_unused]] auto plot_gray = [&](ImageWithData<Grayscale, Grayscale> & src, const Rect2i & area){
        tftDisplayer.puttexture_unsafe(area, src.data.get());
    };

    [[maybe_unused]] auto plot_bina = [&](ImageWithData<Binary, Binary> & src, const Rect2i & area){
        tftDisplayer.puttexture_unsafe(area, src.data.get());
    };

    camera.init();

    I2cDrv              vl_drv(SccbSw, VL53L0X::default_i2c_addr);
    VL53L0X             vl(vl_drv);
    vl.init();
    vl.enableContMode();
    vl.startConv();

    // while(true){
    //     led = !led;
    //     tftDisplayer.fill(RGB565::BLACK);
    //     // tftDisplayer.fill(RGB565::GREEN);
    //     painter.drawFilledCircle({40,60 + 30 * sin(8 * t)}, 8);
    // }

    // mnist_tb();
    uart7.init(1000000, CommMethod::Blocking);

    CH9141 ch9141{uart7, portC[1], portD[3]};
    ch9141.init();

    Transmitter trans{ch9141};
    // Transmitter trans{logger};

    while(true){
        led = !led;
        // continue;
        auto img = Shape::x4(camera,2);
        auto piece = Shape::x4(img,4);
        Pixels::inverse(img);
        auto bina = Pixels::binarization(img, 200);

        vl.update();
        // real_t dist = vl.getDistance();

        // logger.println(dist);

        // Pixels::gamma(img, 0.1);
        plot_gray(img, img.get_window());
        // plot_bina(bina, bina.get_window() + Vector2i{0, img.size.y});
        // int dummy = 0;

        plot_bina(bina, bina.get_window() + Vector2i{0, img.size.y});

        Shape::FloodFill ff;
        auto map = ff.run(bina);

        // Pixels::dyeing(map, map);
        // plot_gray(map, map.get_window() + Vector2i{0, img.size.y * 2});

        // Shape::TwoPass tp(bina.w * bina.h);
        // map = tp.run(bina);

        // Pixels::dyeing(map, map);
        // plot_gray(map, map.get_window() + Vector2i{img.size.x, 0});
        // tp.run(img, bina, dummy);

        // tftDisplayer.puttexture_unsafe(img.get_window(), img.data.get());
        // plot_gray(camera, Rect2i{Vector2i{188/4, 0}, Vector2i{188/4, 60}});
        // painter.drawImage()
        // tftDisplayer.fill(RGB565::BLACK);
        // tftDisplayer.fill(RGB565::RED);
        // tftDisplayer.fill(RGB565::BLUE);
        // painter.drawString({0,0}, "Hello");
        // delay(100);
        // tftDisplayer.fill(RGB565::YELLOW);

        painter.setColor(RGB565::GREEN);
        // auto piece = img.clone({0, 0, 14, 28});


        // painter.drawHollowRect({0,0,28,28});


        const auto & blobs = ff.blobs();
        painter.setColor(RGB565::RED);
        for(const auto & blob : blobs){
            painter.drawRoi(blob.rect);
        }

        trans.transmit(piece, 0);
        painter.drawString(Vector2i{0,230-60}, toString(vl.getDistance()));
        // painter.drawString(Vector2i{0,230-50}, toString(trans.compress_png(piece).size()));

        // delay(300);

        // painter.drawHollowRect(Rect2i{12,12,60,30});

        // delay(10);
        // const auto & blob = blobs[0];
        // printf("%d, %d, %d, %d\r\n", blob.rect.x, blob.rect.w, blob.rect.h, blob.area);
        // printf("%d\r\n", blobs.size());
    }
}

#endif