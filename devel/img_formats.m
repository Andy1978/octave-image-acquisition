
## http://octave.sourceforge.net/image/overview.html image formats
## ycbcr2rgb
## rgb2ycbcr
## core:
## hsv2rgb
## rgb2hsv
## ntsc2rgb
## rgb2ntsc


## of image implementieren:
## whitepoint http://www.mathworks.de/de/help/images/ref/whitepoint.html
##            https://en.wikipedia.org/wiki/Standard_illuminant

## xyz2uint16 https://en.wikipedia.org/wiki/CIEXYZ und zurück?

## imapprox

## demosaic http://www.mathworks.de/de/help/images/ref/demosaic.html

num_bits = 12;

media_ctl_SGRBG(num_bits)

pkg unload all
run ('../PKG_ADD')
addpath('../inst/')

if(!exist("obj","var"))
  obj = videoinput ("v4l2", "/dev/video2");
endif

set(obj,"VideoFormat",sprintf("SGRBG%d",num_bits))
start(obj,4)
s = get(obj ,"VideoResolution")

exposure(100)

img = getsnapshot (obj);
img = getsnapshot (obj);
img = getsnapshot (obj);

stop(obj)

#image(uint8(img(:,:,1:3)/16))


# ReturnedColorSpace rgb, ycbcr, grayscale, bayer
# BayerSensorAlignment gbrg, grbg, bggr, rggb
