# Capture MJPG image

clear all
addpath ("../../inst") # for __test__device__
load_imaq

x = __imaq_handler_open__(__test__device__{:})

# set biggest framesize. FIXME: are we sure the return value is sorted?
if (isunix ())
  s = __imaq_handler_enum_framesizes__ (x, "MJPG")(end,:);
else
  s = [1280 960];
endif
__imaq_handler_s_fmt__ (x, "MJPG", s);

# get highest framerate
if (isunix ())
  fi = __imaq_handler_enum_frameintervals__ (x, s, "MJPG");
  fps = fi(:,2)./fi(:,1);
  [~, idx] = min (fps);
  __imaq_handler_set_frameinterval__ (x, fi(idx, :));
endif
__imaq_handler_get_frameinterval__ (x)

# start streaming
__imaq_handler_streamon__ (x, 2)
n = 50;
tic ();
for k = 1:n
  # FIXME: mir scheint das sind aktuell immer 8,2 fps, egal welche frameinterval und framesize ich einstelle
  [img, seq, timestamp] = __imaq_handler_capture__(x, 0, 1);
endfor
t = toc ();
__imaq_handler_streamoff__ (x)
printf ("INFO: captured %i frames in %.2fs (%.1f fps)\n", n, t, n/t);

rgb = __imaq_handler_JPG_to_RGB__ (img);
image(rgb)
