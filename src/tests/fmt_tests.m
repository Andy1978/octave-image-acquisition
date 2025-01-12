addpath ("../inst")

# $ grep DEFUN_DLD __imaq_handler__.cc | grep -oP "__.*__" | xargs -I{} printf 'autoload ("%s", which ("__imaq_handler__.oct"));\n' {}
autoload ("__imaq_enum_devices__", which ("__imaq_handler__.oct"));
autoload ("__imaq_handler_open__", which ("__imaq_handler__.oct"));
autoload ("__imaq_handler_enum_formats__", which ("__imaq_handler__.oct"));
autoload ("__imaq_handler_s_fmt__", which ("__imaq_handler__.oct"));
autoload ("__imaq_handler_streamoff__", which ("__imaq_handler__.oct"));
autoload ("__imaq_handler_streamon__", which ("__imaq_handler__.oct"));
autoload ("__imaq_handler_capture__", which ("__imaq_handler__.oct"));

x = __imaq_handler_open__(__test__device__{:})
#fmt = __imaq_handler_enum_formats__(x)


# Die hier werden von __imaq_handler_enum_formats__ zurückgeben
# aber mir scheint, ich kann auch andere angeben: BGR24,
# das ist wohl ein Trugschluss... BGR24 wird zu BGR3

# v4l2
#fmt = {"YUYV", "MJPG", "RGB3", "BGR3", "YU12", "YV12"}
#fmt = {"YUYV", "RGB3", "YU12", "YV12"}
if (isunix ())
  fmt = {"YUYV", "YU12", "YV12"}
endif

# media foundation
if (ispc ())
  fmt = {"YUY2", "NV12"}
endif

#ctrls = __imaq_handler_queryctrl__(x);
#__imaq_handler_s_ctrl__(x, ctrls.brightness.id, 30);

for k = 1:numel (fmt)
  __imaq_handler_s_fmt__(x, fmt{k}, [1280 720]);
  __imaq_handler_streamon__ (x, 1)
  tic
  for j = 1:10
    [img, seq, timestamp, timecode] = __imaq_handler_capture__(x, 0, 1);
  endfor
  toc
  __imaq_handler_streamoff__ (x)
  figure (k)
  image (img)
  #mean(img.Y(:))
endfor
