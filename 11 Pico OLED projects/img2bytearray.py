from io import BytesIO
from PIL import Image
import sys

if len(sys.argv) > 1:
    path_to_image = str(sys.argv[1])
    x = int(sys.argv[2])
    y = int(sys.argv[3])

    im = Image.open(path_to_image).convert('1')
    im_resize = im.resize((x,y))
    buf = BytesIO()
    im_resize.save(buf, 'ppm')
    byte_im = buf.getvalue()
    temp = len(str(x) + ' ' + str(y)) + 4
    print(byte_im[temp::])
else:
    print("please specify the location of image i.e img2bytearray.py /path/to/image width heigh")
