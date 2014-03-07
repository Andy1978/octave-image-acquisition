## 07. März 2014 Andreas Weber
## Aptina MT9P031 on beagleboard xM-Rev C
##
## sudo media-ctl -r -l '"mt9p031 2-0048":0->"OMAP3 ISP CCDC":0[1], "OMAP3 ISP CCDC":1->"OMAP3 ISP CCDC output":0[1]'
## sudo media-ctl -V '"mt9p031 2-0048":0 [SGRBG12 2592x1944], "OMAP3 ISP CCDC":1 [SGRBG12 2592x1944]'

pkg unload image-acquisition
run ('../PKG_ADD')
addpath('../inst/')

obj = videoinput ("v4l2", "/dev/video2");
#cam_subdev = videoinput ("v4l2", "/dev/v4l-subdev8");

set(obj,"VideoFormat","SGRBG12")
start(obj,4)
s = get(obj ,"VideoResolution")
cnt = 30
t = zeros(cnt, 1);
for j=1:cnt
  tic();
  img=getsnapshot (obj);
  t(j) = toc ();
endfor
t
img=reshape(img, s)';
assert(numel(img), prod(s))

g1 = img(1:2:end, 1:2:end);
g2 = img(2:2:end, 2:2:end);
g = (g1+g2)/2;
r = img(1:2:end, 2:2:end);
b = img(2:2:end, 1:2:end);
nimg = cat(3, r, g, b);

#image(uint8(nimg/16))
image(uint8(nimg(1:3:end,1:3:end,:)/16))

stop(obj)

#clear obj

