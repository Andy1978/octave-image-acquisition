## Example session for http://wiki.octave.org/Image_acquisition_package
## and ../doc/README

## copy & paste code below into terminal.
## Is this a bug in diary that commands doesn't show up in diary if
## run as script?

run ('../PKG_ADD')
addpath ('../inst')
more off

PS1 (" octave:> ");
PS2 (" >");

fn = "ex1_diary.log";
if (exist (fn, "file"))
  unlink (fn);
endif

diary ex1_diary.log
diary on

disp ("== Example session ==");
disp ("Lines starting with octave> are executed on the octave prompt.");

disp ("=== Load the package and list available hardware ===");

pkg load image-acquisition
imaqhwinfo

disp ("=== Open the v4l2 device and output the result ===");

obj = videoinput("v4l2", "/dev/video0")

disp ("=== Query which properties are available for the used device ===");
disp ("The first 8 ones, starting with an upper letter are fixed, the other specific to the used v4l2 device.");
get(obj)

disp ("=== Set VideoFormat to RGB3 aka RGB24 ===");
set(obj, "VideoFormat", "RGB3");

disp ("=== List available video resolutions ===");
set(obj, "VideoResolution")

disp ("=== Set the video resolution to 320x240px ===");
set(obj, "VideoResolution", [320 240])

disp ("=== Get the current brightness value ===");
get(obj, "brightness")

disp ("=== Query possible range for brightness ===");
set(obj, "brightness")

disp ("=== Set a new value for brightness ===");
set(obj, "brightness", 100)

disp ("=== Start preview ===");
preview(obj)
disp ("Close it with CTRL+C or with [X] on the preview window")

disp ("=== Use higher resolution and start streaming with 2 buffers ===");
set(obj, "VideoResolution", [640 480]);
start(obj, 2)

disp ("=== Get an image from the buffers, view and save it ===");
img = getsnapshot(obj);

image(img)
imwrite(img, "ex1_a.png")

[img, seq, t] = getsnapshot(obj);
seq
t

disp ("=== Stop streaming ===");
stop(obj)

diary off
