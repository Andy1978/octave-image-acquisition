# Capture image in various formats, frame sizes and frame intervals.
# This test will run several minutes...

clear all
load_imaq

x = __imaq_handler_open__(__test__device__{:})
fmts = {__imaq_handler_enum_formats__(x).fourcc}

#if (isunix ())
#  fmts =  {"YUYV", "YU12", "YV12"};
#elseif (ispc ())
#  fmts =  {"YUY2", "NV12", "MJPG"};
#  #fmts =  {"NV12"};
#endif

# Build parameter list
params = [];

for j = 1:numel (fmts)

  fmt = fmts{j};

	# get possible frame sizes for formats
  frame_sizes = __imaq_handler_enum_framesizes__ (x, fmts{j});

  # debugging
  frame_sizes = frame_sizes(randperm(rows (frame_sizes)), :);

  # get the median frame size, the 3/4 frame size and the biggest
  [~, idx] = sort (prod (frame_sizes, 2));
  u_idx = idx(round([1/2 3/4 1] * numel (idx)));

  # only use these frame sizes
  frame_sizes = frame_sizes(unique(u_idx),:);

  for k = 1:rows (frame_sizes)

    frame_size = frame_sizes(k, :);

    # get possible frame intervals
    frame_intervals = __imaq_handler_enum_frameintervals__ (x, fmt, frame_size);

    # get the median fps, the 3/4 fps size and the highest
    [~, idx] = sort (frame_intervals(:, 2)./frame_intervals (:,1));
    u_idx = idx(round([1/2 3/4 1] * numel (idx)));

    # only use these
    frame_intervals = frame_intervals(unique(u_idx),:);

    for l = 1:rows (frame_intervals)
      frame_interval = frame_intervals(l, :);

      params (end + 1).fourcc = fmt;
      params (end).frame_size = frame_size;
      params (end).frame_interval = frame_interval;
    endfor
  endfor
endfor

# Now check these in random order
params = params(randperm (numel (params)));

h = [];
for j = 1:numel (params)

  fps_exp = params(j).frame_interval(2)/params(j).frame_interval(1);
  printf ("%s, %4i x %4i, %4.1f fps, ", params(j).fourcc,
                                     params(j).frame_size(1),
                                     params(j).frame_size(2),
                                     fps_exp);

  __imaq_handler_s_fmt__ (x, params(j).fourcc, params(j).frame_size);
  __imaq_handler_set_frameinterval__ (x, params(j).frame_interval)
  assert (__imaq_handler_get_frameinterval__ (x), params(j).frame_interval);

  # auf etwa 3s Laufzeit pro Messung auslegen
  n = ceil (3 * params(j).frame_interval(2) / params(j).frame_interval(1));
  __imaq_handler_streamon__ (x, 2)
  tic ();
  for k = 1:n
    [img, seq, timestamp] = __imaq_handler_capture__(x, 0, 0);
    if (isempty (h))
      h = imshow (img);
      set(gca, "position", [0.025 0.025 0.95 0.95])
    else
      set (h, "cdata", img);
      drawnow;
    endif
  endfor
  t = toc ();
  __imaq_handler_streamoff__ (x)

  fps = n/t;

  printf ("captured %3i frames in %.2fs (%4.1f fps = %4.1f%%)\n", n, t, fps, fps/fps_exp*100);

endfor


#print_stats = @(X) printf ("%s, min: %i, mean: %i, max: %i\n", typeinfo(X), min(X(:)), mean(X(:)), max(X(:)));
#printf ("Format %s, %ix%i\n", __imaq_handler_g_fmt__  (x).fourcc, columns (img.Y), rows (img.Y))
#__imaq_handler_get_frameinterval__ (x)

#print_stats (img.Y)
#print_stats (img.Cb)
#print_stats (img.Cr)

#r = __imaq_handler_YCbCr_to_RGB__ (img, 601);
#figure (j)
#imshow(r)
#title (fmts{j})



