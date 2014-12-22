## 16.12.2014 Andreas Weber
## uvcvideo on Lenovo ThinkPad Edge E540

run ('../../../PKG_ADD')
addpath (canonicalize_file_name ('../../../inst'))
more off

## Discover hardware
hw = imaqhwinfo

## Open first available device
obj = videoinput ("v4l2", hw.device)

## Show available VideoFormats
## YUV 4:2:2 (YUYV) and MJPEG (compressed) are available thru uvcvideo,
## the other formats are emulated
set (obj, "VideoFormat").description

## Set to RGB24
set (obj, "VideoFormat", "RGB24")

## Start streaming
start (obj, 2)

[img, seq, t] = getsnapshot (obj);
image (img)

stop (obj)


#######

set (obj, "VideoFormat", "MJPG")
start(obj)

## Sometimes the first capured image with MJPG is invalid
## Quick & dirty solution: just skipt it
getsnapshot(obj);

for k=1:10
 img = getsnapshot(obj);
 fn = sprintf ("%03i.jpg", k)
 fflush (stdout);
 save_mjpeg_as_jpg (fn, img);
endfor
    
stop (obj)
