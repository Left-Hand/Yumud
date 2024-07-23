from PIL import Image, ImageDraw, ImageFont
import os
import fontTools.ttLib

images = {}
arrs = {}


# 确保images文件夹存在
if not os.path.exists('images'):
    os.makedirs('images')


def generate_digit_image(digit:int):
    
    # 创建一个空白的RGB图像，背景为白色
    img = Image.new('RGB', (28, 28), color=(255, 255, 255))
    font = ImageFont.truetype("arial.ttf", 34)
    draw = ImageDraw.Draw(img)
    # specifying coordinates and colour of text
    draw.text((4, -5), str(digit), (0, 0, 0), font = font)
    
    return img

def convert_image_to_arr(img):
    w, h = img.size
    arr = []
    for y in range(h):
        for x in range(w):
            arr.append(img.getpixel((x, y))[0])
    return arr

def convert_arr_to_c_code(arr) -> str:
    c_code = '{' + ', '.join(str(x) for x in arr) + '},\r\n'
    return c_code


def convert_all_images_to_c_code():
    c_code = '/*this code is auto generated, DO NOT TOUCH */\r\n'
    c_code += '#pragma once\r\n#include <cstdint>\r\nstatic constexpr uint8_t digit_images[10][784] = {\r\n'
    for i in range(10):
        c_code += convert_arr_to_c_code(arrs[i])

    c_code += '};\n'
    return c_code

# 生成0到9的数字图像
for i in range(10):
    images[i] = generate_digit_image(i)
    arrs[i] = convert_image_to_arr(images[i])

for i in range(10):
    img = images[i]
    img.save(f'src/app/embd/data/image/digit_{i}.bmp')

# print(convert_arr_to_c_code(arrs[0]))
# print(convert_all_images_to_c_code())

c_path = 'src/app/embd/data/autogen'
code = convert_all_images_to_c_code()

with open(os.path.join(c_path, 'digit_images.hpp'), 'w') as f:
    f.write(code)