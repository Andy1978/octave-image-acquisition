addpath("../src")
addpath("../inst")

imaqhwinfo

obj = videoinput ("v4l2", "/dev/video0")
get(obj, "SelectedSourceName")
get(obj, "DeviceCapabilities")

get(obj)

c1 = get(obj, "VideoResolution")
get(obj, "SelectedSourceName")

set(obj, "VideoResolution", [960 720])
c2 = get(obj, "VideoResolution")

set(obj, "VideoInput", 0)
get(obj, "VideoInput")

set(obj, "brightness", 123)
old_b = get(obj, "brightness")
set(obj, "brightness", 323)
new_b = get(obj, "brightness")  #driver clamps to max

set(obj,"power_line_frequency")

set(obj, "brightness", 100)

start_streaming(obj)
frame = getsnapshot(obj);
image(frame)
stop_streaming(obj)

preview(obj)
