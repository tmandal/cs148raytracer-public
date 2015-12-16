#
# Image average
#

import sys, os, numpy, PIL
from PIL import Image

assert len(sys.argv) > 2
num_samples = int(sys.argv[1].strip())
file_prefix = sys.argv[2].strip()
assert num_samples > 0
assert len(file_prefix) > 0

# Assuming all images are the same size, get dimensions of first image
w,h = Image.open(file_prefix + "0" + ".png").size

# Create a numpy array of floats to store the average (assume RGB images)
avg = numpy.zeros((h, w, 3), numpy.float32)

# Build up average pixel intensities, casting each image as an array of floats
for i in range(num_samples):
    image_file = file_prefix + str(i) + ".bin"
    imarr = numpy.fromfile(image_file, dtype=numpy.float32)
    imarr = numpy.reshape(imarr, (h, w, 3))
    avg = avg + imarr/num_samples

# Round values in array and cast as 8-bit integer
avg = numpy.clip(avg * 255.0, 0.0, 255.0)
avg = numpy.array(numpy.rint(avg), dtype=numpy.uint8)

# Generate, save and preview final image
out=Image.fromarray(avg, mode="RGB")
out.save("output.png", "png")
