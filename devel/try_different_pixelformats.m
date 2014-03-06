obj = videoinput ("v4l2", "/dev/video0");

set(obj,"VideoFormat").pixelformat
set(obj,"VideoFormat").fourcc

set(obj,"VideoFormat","RGB3")
start(obj)
img=getsnapshot(obj);
stop(obj)

# see http://www.linuxtv.org/downloads/v4l-dvb-apis/V4L2-PIX-FMT-YUYV.html
set(obj,"VideoFormat","YUYV")
start(obj)
img = getsnapshot(obj);
s = get(obj ,"VideoResolution");
y = reshape(img(1:2:end), s);
cb = reshape(img(2:4:end), [s(1)/2, s(2)]);
cr = reshape(img(4:4:end), [s(1)/2, s(2)]);
# convert to RGB with of image function ycbcr2rgb
pkg load image
rgb = ycbcr2rgb (cat (3,y(1:2:end,:)',cb',cr'));
image(rgb)
stop(obj)
