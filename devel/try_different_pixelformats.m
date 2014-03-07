## Quick & dirty check for different pixelformats
## Y (luminance) is always decimated to fit C

pkg unload image-acquisition
run ('../PKG_ADD')
addpath('../inst/')

obj = videoinput ("v4l2", "/dev/video0");

set(obj,"VideoFormat").pixelformat
set(obj,"VideoFormat").description
set(obj,"VideoFormat").fourcc

##### RGB3 aka RGB24

if (1)
  set(obj,"VideoFormat","RGB3")
  start(obj)
  img=getsnapshot(obj);
  figure(1)
  image(img)
  stop(obj)
endif

##### YUYV aka YUV422
# see http://www.linuxtv.org/downloads/v4l-dvb-apis/V4L2-PIX-FMT-YUYV.html
if (1)
  set(obj,"VideoFormat","YUYV")
  start(obj)
  img = getsnapshot(obj);
  s = get(obj ,"VideoResolution");
  y = reshape(img(1:2:end), s);
  cb = reshape(img(2:4:end), [s(1)/2, s(2)]);
  cr = reshape(img(4:4:end), [s(1)/2, s(2)]);
  # convert to RGB with of image function ycbcr2rgb
  pkg load image
  #YCbCr_standard = "601";
  YCbCr_standard = "709";
  rgb = ycbcr2rgb (cat (3,y(1:2:end,:)',cb',cr'), YCbCr_standard);
  figure(2)
  image(rgb)
  stop(obj)
endif

#### YV12 aka YVU420
# see http://www.linuxtv.org/downloads/v4l-dvb-apis/re23.html

if (1)
  set(obj,"VideoFormat","YV12")
  start(obj)
  img = getsnapshot(obj);
  s = get(obj ,"VideoResolution");
  assert(numel(img), prod(s)*1.5)
  y  = reshape(img(1:prod(s)), s);
  cr = reshape(img(prod(s)+1:prod(s)*1.25), s/2);
  cb = reshape(img(prod(s)*1.25+1:prod(s)*1.5), s/2);
  # convert to RGB with of image function ycbcr2rgb
  pkg load image
  #YCbCr_standard = "601";
  YCbCr_standard = "709";
  rgb = ycbcr2rgb (cat (3,y(1:2:end,1:2:end)',cb',cr'), YCbCr_standard);
  figure(3)
  image(rgb)
  stop(obj)
endif

##### MJPEG aka MJPG

if (1)
  set(obj,"VideoFormat","MJPG")
  start(obj)
  img = getsnapshot(obj);
  # okay, howto view this now?
  stop(obj)
endif
