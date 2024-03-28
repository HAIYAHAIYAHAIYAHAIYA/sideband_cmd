import os
import sys

def BIT(pos):
    return (1 << pos)

# 获取对齐后的内容与对齐大小
def content_align(content, size, align, filled) :
    pat_size = 0
    out_size = len(content)
    if (size % align != 0) :
        pat_size = align - size % align
        content += bytearray(filled.to_bytes(1, "little") * pat_size)  # bytes(pat_size)
        out_size = len(content)

    # print("Size %dK (%d) = %d + %d" % ((out_size) / 1024, out_size, size, pat_size))
    return content, out_size

def file_write_content(file_name, content) :
    fp = open(file_name, "w+b")
    fp.write(content)
    fp.close()

def file_read_content(file) :
    fp = open(file, 'rb')
    data = fp.read()
    fp.close()
    return data

# 将字符串中的16进制转换成16进制数据
def str2hex(s):
    odata = 0
    su =s.upper()
    for c in su:
        tmp=ord(c)
        if tmp <= ord('9') :
            odata = odata << 4
            odata += tmp - ord('0')
        elif ord('A') <= tmp <= ord('F'):
            odata = odata << 4
            odata += tmp - ord('A') + 10
    return odata


# 将字符串中10进制或16进制转化成10进制数据
def get_config_val(str_content) :
    dat = str_content.upper()
    #print(f"{dat[0:1]}")
    if dat[0:2] == "0X" :
        #print(f"{dat[2:]}")
        hex_num = str2hex(dat[2:])
        dex_num = int(hex_num)
        #print(f"{int_num}")
    else :
        #print(f"{type(dat)}")
        dex_num = int(dat)

    return dex_num

def cur_file_dir():
    path = sys.path[0]
    if os.path.isdir(path):
        return path
    elif os.path.isfile(path):
        return os.path.dirname(path)