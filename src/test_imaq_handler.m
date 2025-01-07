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
autoload ("__imaq_handler_get_frameinterval__", which ("__imaq_handler__.oct"));
autoload ("__imaq_handler_set_frameinterval__", which ("__imaq_handler__.oct"));

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

# not yet implemented for mf
cap = __imaq_handler_querycap__ (x);

# enum_inputs, get_input and set_input isn't implemented for mf and returns dummy values
assert (numel (__imaq_handler_enum_inputs__(x)), 1);
assert (__imaq_handler_get_input__(x), 0);
__imaq_handler_set_input__(x, 0);

# Die Rückgabe unterscheidet sich sehr zwischen v4l2 und mf
# v4l2 gibt die 6 FOURCC BGR3, MJPG, RGB3, YU12, YUYV und YV12 zurück,
# mf   gibt die 3 FOURCC MJPG, NV12 und YUY2
#      und auch size, frame_rate und alle Kombinationen (daher numel == 312)

# TODO: sollte man in die Doku zu __imaq_handler_enum_formats__ packen (ins .cc file)
#{
  scalar structure containing the fields:

    size = 640   480
    frame_rate = 30    1
    fourcc = YUY2
    MF_MT_SUBTYPE_CLSID = {32595559-0000-0010-8000-00AA00389B71}
    flags_compressed = 0
    flags_emulated = 0

v4l2:

  scalar structure containing the fields:

    type = Video Capture
    description = YUYV 4:2:2
    pixelformat = YUYV
    fourcc = YUYV
    flags_compressed = 0
    flags_emulated = 0
#}

fmt = __imaq_handler_enum_formats__(x);
unique ({fmt.fourcc})

#fps = vertcat (fmt.frame_rate);
# unter windows gibt es krumme Werte für die frame_rate: 10000000/1333333
# das sollten wohl eher 15/2 sein...

# Es dürften keine doppelten vorkommen, wie kann man das überprüfen?
#tmp = arrayfun ("jsonencode", fmt, "UniformOutput", false);
#assert (numel (fmt) == numel (unique (tmp)));


#search_fmt = "MJPG";
search_size = [1280 960];

# nicht implementiert für mf, beides kann über __imaq_handler_enum_formats__ geholt werden
if (isunix ())
  search_fmt = "YUYV";
  frame_sizes = __imaq_handler_enum_framesizes__ (x, search_fmt)
  frame_intervals = __imaq_handler_enum_frameintervals__ (x, search_size, search_fmt)
endif

# Eigentlich müsste man für mf ja ein filter + unique machen auf den Daten, die loop_native_media_types () zurück gibt.
if (ispc ())
  search_fmt = "YUY2";

  # Nur MJPG, doppelte (wegen frameintervals) raus
  fmt_mask = strcmpi({fmt.fourcc}, search_fmt);
  frame_sizes = unique (vertcat(fmt(fmt_mask).size), "rows")

  # wo findet sich die frame size?
  size_mask = all (vertcat(fmt(fmt_mask).size) == search_size, 2);
  frame_intervals = vertcat(fmt(fmt_mask)(size_mask).frame_rate)
endif

# TODO: Das oben müsste man dann vermutlich in cl_mf_handler.cc nachimplementieren...

# Format und Größe setzen
__imaq_handler_s_fmt__(x, search_fmt, search_size);

# zurücklesen
__imaq_handler_g_fmt__(x)
#__imaq_handler_s_fmt__(x, "MJPG", [800 448])

#######################################################################################

__imaq_handler_get_frameinterval__ (x)
__imaq_handler_set_frameinterval__ (x, [1 5])

####################################################

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

##

__imaq_handler_streamon__ (x, 2)
[img, seq, timestamp] = __imaq_handler_capture__(x);
__imaq_handler_streamoff__ (x)

# man müsste img noch in ein RGB Format wandeln
%! tmp = cat (3, img.Y, kron(img.Cb, [1 1]), kron(img.Cr, [1 1]));
%! # convert to RGB with octave-forge image function ycbcr2rgb
%! pkg load image
%! rgb = ycbcr2rgb (tmp, "709");
%! image(rgb)
%! title ("YUYV, Standard 709")



