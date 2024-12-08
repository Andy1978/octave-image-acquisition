autoload ("__imaq_enum_devices__", which ("__imaq_handler__.oct"));
autoload ("__imaq_handler_open__", which ("__imaq_handler__.oct"));
autoload ("__imaq_handler_querycap__", which ("__imaq_handler__.oct"));
autoload ("__imaq_handler_enum_inputs__", which ("__imaq_handler__.oct"));
autoload ("__imaq_handler_get_input__", which ("__imaq_handler__.oct"));
autoload ("__imaq_handler_set_input__", which ("__imaq_handler__.oct"));
autoload ("__imaq_handler_enum_formats__", which ("__imaq_handler__.oct"));
autoload ("__imaq_handler_enum_framesizes__", which ("__imaq_handler__.oct"));
autoload ("__imaq_handler_enum_frameintervals__", which ("__imaq_handler__.oct"));
autoload ("__imaq_handler_g_parm__", which ("__imaq_handler__.oct"));
autoload ("__imaq_handler_s_parm__", which ("__imaq_handler__.oct"));
autoload ("__imaq_handler_g_fmt__", which ("__imaq_handler__.oct"));
autoload ("__imaq_handler_s_fmt__", which ("__imaq_handler__.oct"));
autoload ("__imaq_handler_queryctrl__", which ("__imaq_handler__.oct"));
autoload ("__imaq_handler_g_ctrl__", which ("__imaq_handler__.oct"));
autoload ("__imaq_handler_s_ctrl__", which ("__imaq_handler__.oct"));
autoload ("__imaq_handler_streamoff__", which ("__imaq_handler__.oct"));
autoload ("__imaq_handler_streamon__", which ("__imaq_handler__.oct"));
autoload ("__imaq_handler_capture__", which ("__imaq_handler__.oct"));
autoload ("__imaq_preview_window_is_shown__", which ("__imaq_handler__.oct"));

if (0)
  #x = __imaq_enum_devices__ ()
  #x.name
  #x.symlink
  #x(1)

  #i = __imaq_handler_open__ ("mf", x(1).symlink);
  i = __imaq_handler_open__ ("v4l2", "/dev/video0");
  #is_shown = __imaq_preview_window_is_shown__ (i)

  #v = __imaq_handler_open__ ("v4l2", "/dev/video0");
  #is_shown = __imaq_preview_window_is_shown__ (v)

  __imaq_handler_enum_inputs__(i)
  __imaq_handler_get_input__(i)
  __imaq_handler_set_input__(i, 0)
  fmt = __imaq_handler_enum_formats__(i)
endif

addpath ("../inst")

x = __imaq_handler_open__(__test__device__{:});
s = __imaq_handler_enum_framesizes__(x, "RGB24");
default_size = s(1,:);
__imaq_handler_s_fmt__(x, "RGB24", default_size);
t = __imaq_handler_enum_frameintervals__(x, default_size, "RGB24");
#__imaq_handler_enum_fmt__(x).description
__imaq_handler_streamon__(x, 2);
[img, seq, timestamp] = __imaq_handler_capture__(x);
assert(size(img), [default_size(2), default_size(1), 3]);

################################

s = __imaq_handler_enum_framesizes__(x, "RGB24");
__imaq_handler_streamoff__(x);
__imaq_handler_s_fmt__(x, "RGB24", s(end,:));
ctrls = __imaq_handler_queryctrl__(x);
if (isfield(ctrls, "brightness"))
  min_brightness = ctrls.brightness.min;
  max_brightness = ctrls.brightness.max;
  __imaq_handler_s_ctrl__(x, ctrls.brightness.id, min_brightness);
  assert(__imaq_handler_g_ctrl__(x, ctrls.brightness.id), min_brightness)
  __imaq_handler_s_ctrl__(x, ctrls.brightness.id, max_brightness);
  assert(__imaq_handler_g_ctrl__(x, ctrls.brightness.id), max_brightness)
  v = round(max_brightness/2);
  __imaq_handler_s_ctrl__(x, ctrls.brightness.id, v);
  assert(__imaq_handler_g_ctrl__(x, ctrls.brightness.id), v);
endif

###############################

r = __imaq_handler_g_parm__(x);
