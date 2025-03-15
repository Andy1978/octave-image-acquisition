# Capture image in various YUV formats (not RGB, not MJPEG)

clear all
load_imaq

x = __imaq_handler_open__(__test__device__{:})
#fmts = {__imaq_handler_enum_formats__(x).fourcc}

if (isunix ())
  fmts =  {"YUYV", "YU12", "YV12"};
elseif (ispc ())
  #fmts =  {"YUY2", "NV12"};
  fmts =  {"NV12"};
endif

for j = 1:numel (fmts)

	fmt = fmts{j};
	printf ("Testing FOURCC '%s'...\n", fmt);

	# get possible frame sizes for formats
  frame_sizes = __imaq_handler_enum_framesizes__ (x, fmt)

	# set the biggest (in terms of pixels) frame size
	[~, idx] = max (prod (frame_sizes, 2));
	frame_size = frame_sizes (idx, :);

  __imaq_handler_s_fmt__ (x, fmt, frame_size);

	# get possible frame intervals
	frame_intervals = __imaq_handler_enum_frameintervals__ (x, fmt, frame_size)

	fps = frame_intervals(:, 2)./frame_intervals (:,1)

	# testweise nicht das maximum
	__imaq_handler_set_frameinterval__ (x, [1 30])

  __imaq_handler_streamon__ (x, 2)
  n = 50;
  tic ();
  for k = 1:n
    # with v4l2, no all driver return a timecode
    #[img, seq, timestamp, timecode] = __imaq_handler_capture__(x, 0, 1);
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

