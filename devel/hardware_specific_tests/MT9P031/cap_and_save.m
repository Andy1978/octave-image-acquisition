## 20.03.2014 Andreas Weber
## Aptina MT9P031 on beagleboard xM-Rev C

## FIXME: the first captured image is brighter than the following.

## possible values: 8, 10, 12
num_bits = 8;
set_mt9p031_SGRBG_bitdepth (num_bits);

pkg unload all
run ("../../../PKG_ADD")
addpath (canonicalize_file_name ("../../../inst/"))

if(!exist("obj","var"))
  obj = videoinput ("v4l2", "/dev/video2");
endif

set (obj, "VideoFormat", sprintf ("SGRBG%d", num_bits))
start (obj, 4)
s = get (obj, "VideoResolution")

set_mt9p031_exposure (1000);
## drop first capture
getsnapshot (obj);
img = getsnapshot (obj);
stop (obj)

img = SGRBGtoRGB(img);
img = uint8(img/2**(num_bits-8));

imwrite (img, "out.jpg")
