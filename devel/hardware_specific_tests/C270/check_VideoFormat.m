## 27.12.2014 Andreas Weber
## Check different pixelformats RGB3, YUYV, YV12, YU12 and MJPEG-
## YUYV and MJPG are supported by the C270 camera, other emulated from libv4l2

pkg unload all
run ("../../../PKG_ADD")
addpath (canonicalize_file_name ("../../../inst/"))

obj = videoinput ("v4l2", "/dev/video0");

#set(obj,"VideoFormat").pixelformat
#set(obj,"VideoFormat").description
#set(obj,"VideoFormat").fourcc
#set(obj,"VideoFormat").flags_emulated

##### RGB3 aka RGB24

if (1)
  set(obj, "VideoFormat", "RGB3")
  start(obj)
  img=getsnapshot(obj);
  figure(1)
  image(img)
  stop(obj)
  title ("RGB3");
  print ("RGB3.png")
endif

##### YUYV aka YUV422
# see https://www.kernel.org/doc/html/v6.1/userspace-api/media/v4l/pixfmt-packed-yuv.html
if (1)
  set(obj, "VideoFormat", "YUYV")
  start(obj)
  img = getsnapshot(obj);
  tmp = cat (3, img.Y, kron(img.Cb, [1 1]), kron(img.Cr, [1 1]));
  # convert to RGB with of image function ycbcr2rgb
  pkg load image
  #YCbCr_standard = "601";
  YCbCr_standard = "709";
  rgb = ycbcr2rgb (tmp, YCbCr_standard);
  figure(2)
  image(rgb)
  stop(obj)
  title ("YUYV");
  print ("YUYV.png")
endif

#### YV12 aka YVU420
# see https://www.kernel.org/doc/html/v6.1/userspace-api/media/v4l/pixfmt-yuv-planar.html

if (1)
  set(obj, "VideoFormat", "YV12")
  start(obj)
  img = getsnapshot(obj);
  tmp = cat (3, img.Y, kron(img.Cb, ones (2)), kron(img.Cr, ones (2)));

  # convert to RGB with of image function ycbcr2rgb
  pkg load image
  #YCbCr_standard = "601";
  YCbCr_standard = "709";
  rgb = ycbcr2rgb (tmp, YCbCr_standard);
  figure(3)
  image(rgb)
  stop(obj)
  title ("YV12");
  print ("YV12.png")
endif

#### YU12 aka YUV420
# see https://www.kernel.org/doc/html/v6.1/userspace-api/media/v4l/pixfmt-yuv-planar.html

if (1)
  set(obj,"VideoFormat","YU12")
  start(obj)
  img = getsnapshot(obj);
  tmp = cat (3, img.Y, kron(img.Cb, ones (2)), kron(img.Cr, ones (2)));

  # convert to RGB with of image function ycbcr2rgb
  pkg load image
  #YCbCr_standard = "601";
  YCbCr_standard = "709";
  rgb = ycbcr2rgb (tmp, YCbCr_standard);
  figure(4)
  image(rgb)
  stop(obj)
  title ("YU12");
  print ("YU12.png")
endif

##### MJPEG aka MJPG

if (1)
  set(obj,"VideoFormat","MJPG")
  start(obj)

  for k=1:10
    img = getsnapshot(obj);
    fn = sprintf ("MJPG_%03i.jpg", k);
    save_mjpeg_as_jpg (fn, img);
  endfor

  stop(obj)

  rgb = imread (fn);
  image (rgb);
  title ("MJPG");
  print ("MJPG.png");
endif
