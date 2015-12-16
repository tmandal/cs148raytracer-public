#
# Image stitching
#

import sys, os, numpy, PIL
from PIL import Image

assert len(sys.argv) > 3
num_grids_r = int(sys.argv[1].strip())
num_grids_c = int(sys.argv[2].strip())
file_prefix = sys.argv[3].strip()
assert num_grids_r > 0
assert num_grids_c > 0
assert len(file_prefix) > 0

# Assuming all images are the same size, get dimensions of first image
w,h = Image.open(file_prefix + "0_0" + ".png").size

final_image = numpy.zeros((h, w, 3), numpy.uint8)

for r in range(num_grids_r):
    for c in range(num_grids_c):
        image_file = file_prefix + str(r) + "_" + str(c) + ".png"
        if (os.path.isfile(image_file)):
            image_array = numpy.array(Image.open(image_file), dtype=numpy.uint8)
            final_image = final_image + image_array

out=Image.fromarray(final_image, mode="RGB")
out.save("output.png", "png")
