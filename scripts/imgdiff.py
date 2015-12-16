#
# Image diff
#

import sys, os, numpy, PIL
from PIL import Image, ImageChops

point_table = ([0] + ([255] * 255))

def black_or_b(a, b):
    diff = ImageChops.difference(a, b)
    diff = diff.convert('L')
    diff = diff.point(point_table)
    new = diff.convert('RGB')
    new.paste(b, mask=diff)
    return new

a = Image.open(sys.argv[1].strip())
b = Image.open(sys.argv[2].strip())

#c = black_or_b(a, b)
#c.save(sys.argv[3].strip())

imarr_a=numpy.array(a,dtype=numpy.int)
imarr_b=numpy.array(b,dtype=numpy.int)

diff = imarr_b - imarr_a
diff.tofile(sys.argv[3].strip(), sep=" ")
