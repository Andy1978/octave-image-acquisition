# Capture MJPG frame from Logitech C270
# (libjpeg complains with "Corrupt JPEG data: 8 extraneous bytes before marker 0xd0" in this case

clear all
pkg load image
addpath ("../../inst")
addpath ("../")

autoload ("__imaq_enum_devices__", which ("__imaq_handler__.oct"));
autoload ("__imaq_handler_open__", which ("__imaq_handler__.oct"));
autoload ("__imaq_handler_s_fmt__", which ("__imaq_handler__.oct"));
autoload ("__imaq_handler_streamoff__", which ("__imaq_handler__.oct"));
autoload ("__imaq_handler_streamon__", which ("__imaq_handler__.oct"));
autoload ("__imaq_handler_capture__", which ("__imaq_handler__.oct"));
autoload ("__imaq_handler_YCbCr_to_RGB__", which ("__imaq_handler__.oct"));

x = __imaq_handler_open__(__test__device__{:})
__imaq_handler_s_fmt__ (x, "MJPG", [1280 720]);
__imaq_handler_streamon__ (x, 2)
for k = 1:5
  [img, seq, timestamp] = __imaq_handler_capture__(x, 0, 0);
  timestamp
endfor
__imaq_handler_streamoff__ (x)

save_mjpeg_as_jpg ("foo.jpg", img)

fid = fopen ("foo.raw", "wb");
fwrite (fid, img);
fclose (fid);
