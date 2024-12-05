

autoload ("__imaq_list_devices__", which ("__imaq_handler__.oct"))
autoload ("__imaq_handler_open__", which ("__imaq_handler__.oct"));
autoload ("__imaq_preview_window_is_shown__", which ("__imaq_handler__.oct"));

#x = __imaq_list_devices__ ();
#x(1)

i = __imaq_handler_open__ ("mf", "windoze cam");
is_shown = __imaq_preview_window_is_shown__ (i)

v = __imaq_handler_open__ ("v4l2", "/dev/video0");
is_shown = __imaq_preview_window_is_shown__ (v)
