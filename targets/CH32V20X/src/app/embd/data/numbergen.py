from PIL import Image, ImageDraw, ImageFont
import os
import fontTools.ttLib


img_size = (8,12)
img_offset = (0, -2)
font_size = 14

area = str(img_size[0] * img_size[1])

images = {}
arrs = {}

def get_gs_name(index):
    return f'g{index}'

def get_bn_name(index):
    return f'b{index}'

def get_gs_path(index):
    return f'src/app/embd/data/image/gs/digit_{index}.bmp'

def get_bn_path(index):
    return f'src/app/embd/data/image/bn/digit_{index}.bmp'

# 确保images文件夹存在
if not os.path.exists('images'):
    os.makedirs('images')


def generate_digit_image(digit:int):
    
    # 创建一个空白的RGB图像，背景为白色
    img = Image.new('RGB', img_size, color=(255, 255, 255))
    font = ImageFont.truetype("arial.ttf", font_size)
    draw = ImageDraw.Draw(img)
    # specifying coordinates and colour of text
    draw.text(img_offset, str(digit), (0, 0, 0), font = font)
    
    return img

def convert_gs_image_to_arr(img):
    w, h = img.size
    arr = []
    for y in range(h):
        for x in range(w):
            arr.append(img.getpixel((x, y))[0])
    return arr

def convert_bn_image_to_arr(img):
    w, h = img.size
    arr = []
    for y in range(h):
        for x in range(w):
            arr.append(0 if (img.getpixel((x, y))[0] < 128) else 255)
    return arr

def convert_arr_to_c_code(arr) -> str:
    c_code = '{' + ', '.join(str(x) for x in arr) + '},\r\n'
    return c_code


def convert_all_images_to_c_code(arrs):
    c_code = '/*this code is auto generated, DO NOT TOUCH */\r\n'

    c_code += '#pragma once\r\n#include <cstdint>\r\nscexpr uint8_t digit_images[10]['+area +'] = {\r\n'
    for i in range(10):
        c_code += convert_arr_to_c_code(arrs[get_gs_name(i)])

    c_code += '};\n'

    c_code += 'scexpr uint8_t digit_bina_images[10]['+ area +'] = {\r\n'
    for i in range(10):
        c_code += convert_arr_to_c_code(arrs[get_bn_name(i)])

    c_code += '};\n'
    return c_code

# 生成0到9的数字图像
for i in range(10):
    images[i] = generate_digit_image(i)
    arrs[get_gs_name(i)] = convert_gs_image_to_arr(images[i])
    arrs[get_bn_name(i)] = convert_bn_image_to_arr(images[i])

for i in range(10):
    img = images[i]
    img.save(get_gs_path(i))

c_path = 'src/app/embd/data/autogen'
code = convert_all_images_to_c_code(arrs)

with open(os.path.join(c_path, 'digit_images.hpp'), 'w') as f:
    f.write(code)