## 12.01.2014 Andreas Weber
## run demos and tests

addpath("../src/")
addpath("../inst/")

obj = videoinput("v4l2", "/dev/video0")

get(obj, "VideoResolution")

clear obj
obj = videoinput("v4l2", "/dev/video0", [1280 960])
get(obj, "VideoResolution")


set(obj, "VideoResolution", [640 480])

#clear obj

#imaqhwinfo
#obj = videoinput('v4l2', '/dev/video0')
#get(obj)
#preview(obj)
#frame = getsnapshot(obj)
#image(frame)
