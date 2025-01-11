# Capture MJPG image

clear all
addpath ("../../inst") # for __test__device__
load_imaq

x = __imaq_handler_open__(__test__device__{:})
__imaq_handler_s_fmt__ (x, "MJPG", [1280 720]);
__imaq_handler_set_frameinterval__ (x, [1 30]);

__imaq_handler_streamon__ (x, 2)
n = 500;
do
  [img, seq, timestamp] = __imaq_handler_capture__(x, 1, 0);
  #image (img);
  #drawnow ();
until (! __imaq_preview_window_is_shown__(x))

__imaq_handler_streamoff__ (x)
