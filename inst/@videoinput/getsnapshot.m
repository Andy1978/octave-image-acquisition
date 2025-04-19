## Copyright (C) 2014-2025 Andreas Weber <andy.weber.aw@gmail.com>
##
## This program is free software; you can redistribute it and/or modify it under
## the terms of the GNU General Public License as published by the Free Software
## Foundation; either version 3 of the License, or (at your option) any later
## version.
##
## This program is distributed in the hope that it will be useful, but WITHOUT
## ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
## FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
## details.
##
## You should have received a copy of the GNU General Public License along with
## this program; if not, see <http:##www.gnu.org/licenses/>.

## -*- texinfo -*-
## @deftypefn {Function File} {[@var{img}, @var{seq}, @var{ts}, @var{tc}] =} getsnapshot (@var{vi}, [@var{preview}], [@var{raw}])
## Get a snapshot from a videoinput object.
## Streaming has to be enabled before calling getsnapshot.
## If @var{preview} == true the captured image is also shown in a separate window.
##
## @table @var
## @item img
## Captured image.
##
## If @var{raw} == false (default) the format returned from the driver is
## converted to RGB3. Width and height matches @code{get(VI, "VideoResolution")}
##
## If @var{raw} == true the format returned by the driver
## (see @qcode{VideoFormat} property) is returned.
##
##   @table @var
##   @item @qcode{RGB3}, @qcode{RGB24}
##   @nospell{HxWx3} uint8 matrix with RGB values
##
##   @item @qcode{YUYV}, @qcode{YUV422}
##   scalar struct with fieldnames Y, Cb and Cr. The horizontal resolution of Cb and
##   Cr is half the horizontal resolution of Y.
##
##   @item @qcode{YV12}, @qcode{YVU420}, @qcode{YU12}, @qcode{YUV420}
##   scalar struct with fieldnames Y, Cb and Cr. The horizontal and vertical resolution
##   of Cb and Cr is half the resolution of Y.
##
##   @item @qcode{MJPG}, @qcode{MJPEG}
##   uint8 row vector with compressed MJPEG data. The length may vary from
##   frame to frame due to compression. You can save this as JPEG (add a huffman table) with
##   @example
##   @group
##     obj = videoinput();
##     set (obj, "VideoFormat", "MJPG");
##     start (obj);
##     img = getsnapshot (obj);
##     save_mjpeg_as_jpg ("capture.jpg", img);
##   @end group
##   @end example
##   @end table
## @item seq
## Set by the driver, counting the frames (not fields!) in sequence.
##
## @item struct @var{ts}
##   For input streams this is time when the first data byte was captured,
##   as returned by the clock_gettime() function for the relevant clock id.
##
##   @table @var
##   @item tv_sec
##   seconds
##   @item tv_usec
##   microseconds
##   @end table
##
## @item struct @var{tc}
## Timecode, see @url{https://www.kernel.org/doc/html/v6.1/userspace-api/media/v4l/buffer.html#c.V4L.v4l2_timecode}
## @end table
## @seealso {@@videoinput/start, @@videoinput/preview}
## @end deftypefn

function [img, seq, timestamp, timecode] = getsnapshot (vi, preview = 0, raw = 0)

  if (nargin < 1 || nargin > 3)
    print_usage();
  endif
  if (nargout <= 3)
    [img, seq, timestamp] = __imaq_handler_capture__(vi.imaqh, preview, raw);
  else
    [img, seq, timestamp, timecode] = __imaq_handler_capture__(vi.imaqh, preview, raw);
  endif
  #fmt = __imaq_handler_g_fmt__(vi.imaqh).pixelformat;
  #printf ("pixelformat = -%s-\n", fmt);

endfunction

%!test
%! obj = videoinput(__test__device__{:});
%! oldval = get(obj, "VideoResolution");
%! default_size = set (obj, "VideoResolution")(1,:);
%! set (obj, "VideoResolution", default_size);
%! # We expect here, that every driver (v4l2 and Media Foundation)
%! # on earth would support YUYV. This might not be the case...
%! set (obj, 'VideoFormat', 'YUYV');
%! start (obj, 2)
%! img = getsnapshot (obj);
%! do_preview = !isempty(getenv("DISPLAY")); # only if there is a display
%! img = getsnapshot (obj, do_preview);
%! [img, seq] = getsnapshot (obj, do_preview);
%! # The v4l2 loopback device doesn't support the seqence numbering and returns always 0
%! if (!strcmp(get(obj, "DeviceCapabilities").driver, "v4l2 loopback"))
%!   assert (seq >= 2);
%! endif
%! [img, seq, T] = getsnapshot(obj, 0);
%! stop (obj)
%! set (obj, "VideoResolution", oldval);

%!test
%! obj = videoinput (__test__device__{:});
%! fmts = {set(obj,"VideoFormat").fourcc};
%! for k = 1:numel (fmts)
%!   set (obj, "VideoFormat", fmts{k});
%!   s = get (obj, "VideoResolution");
%!   start (obj)
%!   # try to internally convert to RGB3
%!   # this might fail if there is no converter (yet)
%!   try
%!     img = getsnapshot (obj);
%!     assert (size (img), [fliplr(s) 3]);
%!   catch ME
%!     if (! strcmp (ME.identifier, 'image-acquisition:getsnapshot:unsupported-video-format'))
%!       stop (obj);
%!       rethrow (ME);
%!     endif
%!   end_try_catch
%!   # getting a raw representation (like for H264) should always work
%!   img = getsnapshot (obj, false, true); # ret raw
%!   stop (obj)
%! endfor

%!demo
%! obj = videoinput ();
%! start (obj)
%! img = getsnapshot (obj);
%! image (img)
%! stop (obj)
