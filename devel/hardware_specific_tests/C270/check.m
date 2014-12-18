## 17.12.2014 Andreas Weber
## Logitech Webcam C270 on /dev/video0

pkg unload all
run ('../../../PKG_ADD')
addpath('../../../inst/')

hw = imaqhwinfo;

if (numel (hw) < 1)
  error ("No suitable v4l2 hardware found")
endif

obj = videoinput ("v4l2", hw.device);

set (obj, "VideoFormat", "RGB24")

## get a list with supported resolutions;
## the first row are default values
res = set (obj, "VideoResolution");

## Try all possible resolutions
%~ for k=1:rows (res)
  %~ printf ("trying to set resolution to %ix%i...\n", res(k, 1), res(k, 2));
  %~ set (obj, "VideoResolution", res(k, :));
  %~ start (obj);
  %~ [img, seq, t] = getsnapshot (obj, 0);
  %~ printf ("size of returned image is %ix%ix%i\n", size (img));
  %~ assert (size(img)([2 1]), res(k, :));
  %~ stop (obj);
  %~ fflush (stdout);
%~ endfor

## Select default
set (obj, "VideoResolution", res(1, :));

start (obj)
[img, seq, ts] = getsnapshot (obj);
stop (obj)

preview (obj)


