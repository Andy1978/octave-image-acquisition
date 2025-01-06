addpath ("../inst")

# $ grep DEFUN_DLD __imaq_handler__.cc | grep -oP "__.*__" | xargs -I{} printf 'autoload ("%s", which ("__imaq_handler__.oct"));\n' {}
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

td = __test__device__; # uses imaqhwinfo, which calls __imaq_enum_devices__
if (isunix ())
  assert (td{1}, "v4l2")
elseif (ispc ())
  assert (td{1}, "mf")
endif
x = __imaq_handler_open__(td{:})

# enum_inputs, get_input and set_input isn't implemented on windoze and returns dummy values
assert (numel (__imaq_handler_enum_inputs__(x)), 1);
assert (__imaq_handler_get_input__(x), 0);
__imaq_handler_set_input__(x, 0);

# Die Rückgabe unterscheidet sich sehr zwischen v4l2 und mf
# v4l2 gibt die 6 FOURCC BGR3, MJPG, RGB3, YU12, YUYV und YV12 zurück,
# mf   gibt die 3 FOURCC MJPG, NV12 und YUY2
#      und auch size, frame_rate und alle Kombinationen (daher numel == 312)

fmt = __imaq_handler_enum_formats__(x)
unique ({fmt.fourcc})

fps = vertcat (fmt.frame_rate);

# unter windows gibt es krumme Werte für die frame_rate: 10000000/1333333
# das sollten wohl eher 15/2 sein...


