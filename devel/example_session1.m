## Example session for the page http://wiki.octave.org/Image_acquisition_package
## and README

run ('../PKG_ADD')
addpath('../inst')
page_screen_output(0)
diary ex1_diary.log
diary on

imaqhwinfo

obj = videoinput("v4l2", "/dev/video0")
get(obj)

get(obj, "DeviceCapabilities")

set(obj, "VideoResolution")
set(obj, "VideoResolution", [320 240])

get(obj, "brightness")
set(obj, "brightness")
set(obj, "brightness", 100)

preview(obj)

set(obj, "VideoResolution", [640 480]);
start(obj, 2)
img = getsnapshot(obj);

image(img)
imwrite(img, "ex1_a.png")

[img, seq, t] = getsnapshot(obj);
seq
t

stop(obj)

diary off

