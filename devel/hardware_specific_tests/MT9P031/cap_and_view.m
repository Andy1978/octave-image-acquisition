## 20.03.2014 Andreas Weber
## Aptina MT9P031 on beagleboard xM-Rev C

## FIXME: the first captured image is brighter than the following.

## possible values: 8, 10, 12
num_bits = 8;
set_mt9p031_SGRBG_bitdepth (num_bits);

pkg unload all
run ('../../../PKG_ADD')
addpath('../../../inst/')

if(!exist("obj","var"))
  obj = videoinput ("v4l2", "/dev/video2");
endif

set (obj, "VideoFormat", sprintf ("SGRBG%d", num_bits))
start (obj, 4)
s = get (obj, "VideoResolution")

set_mt9p031_exposure (400);
img=getsnapshot (obj);
img=getsnapshot (obj);
stop(obj)

img = SGRBGtoRGB(img);
img = uint8(img/2**(num_bits-8));

figure(1)
image(img(1:5:end, 1:5:end,:))

r=img(:,:,1);
g=img(:,:,2);
b=img(:,:,3);

n=10:5:255;
[nr, x]=hist(r(:),n);
[ng, ~]=hist(g(:),n);
[nb, ~]=hist(b(:),n);

x=x(2:end);
figure(2)
plot(x,nr(2:end),"-xr",x,ng(2:end),"-xg",x,nb(2:end),"-xb")
grid on
