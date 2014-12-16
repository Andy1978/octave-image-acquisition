## 21.03.2014 Andreas Weber
## MT9P031 an video2
## Logitech Webcam C270 an video7

pkg unload all
run ('../PKG_ADD')
addpath('../inst/')

obj = videoinput ("v4l2", "/dev/video0");
set(obj,"VideoFormat","RGB24")
start(obj,4)
s = fliplr(get(obj ,"VideoResolution"))
fmt = get(obj, "VideoFormat")

for n=1:3
  img = getsnapshot (obj);
  assert(size(img), [s, 3]);
endfor
stop(obj)
#clear obj

## bug mit ReturnedColorSpace finden... Ist kein bug.. siehe
## https://www.gnu.org/software/octave/doc/interpreter/Manipulating-Classes.html#Manipulating-Classes
## Note that as Octave does not implement pass by reference, than the
## modified object is the return value of the set method and it must be called like 

set(obj,"ReturnedColorSpace", "bayer")
get(obj, "ReturnedColorSpace")
