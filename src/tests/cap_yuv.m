# Capture image in various YUV formats (not RGB, not MJPEG)

clear all
addpath ("../../inst") # for __test__device__
load_imaq

x = __imaq_handler_open__(__test__device__{:})

if (isunix ())
  fmts =  {"YUYV", "YU12", "YV12"};
elseif (ispc ())
  fmts =  {"YUY2", "NV12"};
  #fmts =  {"NV12"};
endif

for j = 1:numel (fmts)

  frame_sizes = __imaq_handler_enum_framesizes__ (x, fmts{j})
  __imaq_handler_s_fmt__ (x, fmts{j}, frame_sizes (end, :));
  __imaq_handler_streamon__ (x, 2)
  n = 50;
  tic ();
  for k = 1:n
    [img, seq, timestamp] = __imaq_handler_capture__(x, 0, 1);
  endfor
  t = toc ();
  __imaq_handler_streamoff__ (x)
  printf ("INFO: captured %i frames in %.2fs (%.1f fps)\n", n, t, n/t);

  print_stats = @(X) printf ("%s, min: %i, mean: %i, max: %i\n", typeinfo(X), min(X(:)), mean(X(:)), max(X(:)));

  printf ("Format %s, %ix%i\n", __imaq_handler_g_fmt__  (x).fourcc, columns (img.Y), rows (img.Y))
  __imaq_handler_get_frameinterval__ (x)
  print_stats (img.Y)
  print_stats (img.Cb)
  print_stats (img.Cr)

  r = __imaq_handler_YCbCr_to_RGB__ (img, 601);
  figure (j)
  imshow(r)
  title (fmts{j})

endfor

