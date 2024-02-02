# Diff Details

Date : 2024-02-02 13:34:43

Directory c:\\RV32\\CH32V203\\LearnADC

Total : 46 files,  686 codes, 136 comments, 241 blanks, all 1063 lines

[Summary](results.md) / [Details](details.md) / [Diff Summary](diff.md) / Diff Details

## Files
| filename | language | code | comment | blank | total |
| :--- | :--- | ---: | ---: | ---: | ---: |
| [.eide/eide.json](/.eide/eide.json) | JSON | 5 | 0 | 0 | 5 |
| [LearnADC.code-workspace](/LearnADC.code-workspace) | JSON with Comments | 2 | 0 | 0 | 2 |
| [src/HX711/HX711.c](/src/HX711/HX711.c) | C | 73 | 3 | 16 | 92 |
| [src/HX711/HX711.h](/src/HX711/HX711.h) | C | 28 | 0 | 10 | 38 |
| [src/SSD1306/ssd1306.cpp](/src/SSD1306/ssd1306.cpp) | C++ | 19 | 4 | 8 | 31 |
| [src/SSD1306/ssd1306.hpp](/src/SSD1306/ssd1306.hpp) | C++ | 66 | 0 | 16 | 82 |
| [src/ST7735/st7735.cpp](/src/ST7735/st7735.cpp) | C++ | 99 | 12 | 13 | 124 |
| [src/ST7735/st7735.hpp](/src/ST7735/st7735.hpp) | C++ | 52 | 0 | 13 | 65 |
| [src/ST7789V2/st7789.cpp](/src/ST7789V2/st7789.cpp) | C++ | 36 | 3 | 9 | 48 |
| [src/ST7789V2/st7789.hpp](/src/ST7789V2/st7789.hpp) | C++ | 77 | 0 | 19 | 96 |
| [src/ST7789/st7789.c](/src/ST7789/st7789.c) | C | 1 | 0 | -1 | 0 |
| [src/TTP229/TTP229.c](/src/TTP229/TTP229.c) | C | 44 | 2 | 12 | 58 |
| [src/TTP229/TTP229.h](/src/TTP229/TTP229.h) | C | 24 | 0 | 8 | 32 |
| [src/bus/bus.cpp](/src/bus/bus.cpp) | C++ | -1 | -5 | -4 | -10 |
| [src/bus/bus.hpp](/src/bus/bus.hpp) | C++ | 8 | 0 | 7 | 15 |
| [src/bus/i2c/i2csw.cpp](/src/bus/i2c/i2csw.cpp) | C++ | 0 | 0 | 1 | 1 |
| [src/bus/i2c/i2csw.hpp](/src/bus/i2c/i2csw.hpp) | C++ | 0 | 38 | 14 | 52 |
| [src/bus/printer.cpp](/src/bus/printer.cpp) | C++ | -16 | 14 | 1 | -1 |
| [src/bus/printer.hpp](/src/bus/printer.hpp) | C++ | 7 | -10 | 1 | -2 |
| [src/bus/spi/spi2.cpp](/src/bus/spi/spi2.cpp) | C++ | 119 | 0 | 26 | 145 |
| [src/bus/spi/spi2.hpp](/src/bus/spi/spi2.hpp) | C++ | 59 | 0 | 20 | 79 |
| [src/bus/spi/spi2_hs.cpp](/src/bus/spi/spi2_hs.cpp) | C++ | 5 | 0 | 1 | 6 |
| [src/bus/spi/spi2_hs.hpp](/src/bus/spi/spi2_hs.hpp) | C++ | 38 | 0 | 14 | 52 |
| [src/bus/uart/uart1.cpp](/src/bus/uart/uart1.cpp) | C++ | -23 | 0 | -7 | -30 |
| [src/bus/uart/uart1.hpp](/src/bus/uart/uart1.hpp) | C++ | 8 | 0 | 4 | 12 |
| [src/bus/uart/uart2.cpp](/src/bus/uart/uart2.cpp) | C++ | -22 | 0 | -7 | -29 |
| [src/bus/uart/uart2.hpp](/src/bus/uart/uart2.hpp) | C++ | 8 | 0 | 4 | 12 |
| [src/main.cpp](/src/main.cpp) | C++ | 50 | 53 | 18 | 121 |
| [src/ringbuf/ringbuf.cpp](/src/ringbuf/ringbuf.cpp) | C++ | -23 | 0 | -7 | -30 |
| [src/ringbuf/ringbuf.hpp](/src/ringbuf/ringbuf.hpp) | C++ | -49 | 0 | -14 | -63 |
| [types/buffer/buffer.hpp](/types/buffer/buffer.hpp) | C++ | 27 | 0 | 10 | 37 |
| [types/buffer/ringbuf/ringbuf_t.hpp](/types/buffer/ringbuf/ringbuf_t.hpp) | C++ | 45 | 0 | 13 | 58 |
| [types/buffer/stack/stack_t.hpp](/types/buffer/stack/stack_t.hpp) | C++ | 0 | 52 | 15 | 67 |
| [types/rect2/rect2.hpp](/types/rect2/rect2.hpp) | C++ | -4 | 0 | -6 | -10 |
| [types/rect2/rect2_t.hpp](/types/rect2/rect2_t.hpp) | C++ | 26 | 0 | 12 | 38 |
| [types/rect2/rect2_t.tpp](/types/rect2/rect2_t.tpp) | C++ | 34 | 5 | 10 | 49 |
| [types/rgb.h](/types/rgb.h) | C | 37 | 0 | 1 | 38 |
| [types/string/String.cpp](/types/string/String.cpp) | C++ | -63 | -9 | -11 | -83 |
| [types/string/String.hpp](/types/string/String.hpp) | C++ | -12 | -5 | -1 | -18 |
| [types/string/pgmspace.h](/types/string/pgmspace.h) | C | -100 | -21 | -11 | -132 |
| [types/string/string_utils.c](/types/string/string_utils.c) | C | -16 | 0 | 7 | -9 |
| [types/string/string_utils.h](/types/string/string_utils.h) | C | 1 | 0 | 0 | 1 |
| [types/vector2/Vector2.hpp](/types/vector2/Vector2.hpp) | C++ | -96 | 0 | -23 | -119 |
| [types/vector2/Vector2.tpp](/types/vector2/Vector2.tpp) | C++ | -193 | 0 | -40 | -233 |
| [types/vector2/vector2_t.hpp](/types/vector2/vector2_t.hpp) | C++ | 102 | 0 | 28 | 130 |
| [types/vector2/vector2_t.tpp](/types/vector2/vector2_t.tpp) | C++ | 204 | 0 | 42 | 246 |

[Summary](results.md) / [Details](details.md) / [Diff Summary](diff.md) / Diff Details