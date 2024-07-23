from PIL import Image, ImageDraw, ImageFont
import os
import fontTools.ttLib


img_size = (280, 280)
img_offset = (40, 0)
font_size = 200


images = {}
arrs = {}

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

    img.save(f'src/app/embd/data/image/show/digit_{digit}.bmp')

for i in range(10):
    generate_digit_image(i)