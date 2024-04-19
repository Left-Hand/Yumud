import tkinter

def main():


    try:
        # 尝试打开文件
        with open('hello', 'r+') as f:
            # 获取当前文件内容
            content = f.read()
            # 获取调用次数（从文件内容中提取）
            count = int(content.strip()) if content.strip().isdigit() else 0
            # 将调用次数加一
            count += 1
            # 将新的调用次数写入文件
            f.seek(0)
            f.write(str(count))
            f.truncate()
    except FileNotFoundError:
        # 如果文件不存在，则创建文件并写入调用次数为1
        with open('hello', 'w') as f:
            f.write('1')


    top = tkinter.Tk()
    # 进入消息循环
    top.mainloop()

if __name__ == "__main__":
    main()