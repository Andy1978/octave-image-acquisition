## Copyright (C) 2014 Andreas Weber <andy.weber.aw@gmail.com>
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
## @deftypefn {Function File} {[@var{img}, @var{seq}, @var{ts}, @var{tc}] =} getsnapshot (@var{vi}, [@var{preview}])
## Get a snapshot from a videoinput object buffer.
## Streaming has to be enabled before calling getsnapshot.
## If @var{preview}==true the captured image is also shown in a separate FLTK window.
##
## @table @var
## @item img
## Captured image. The type and size of @var{img} depends on the @qcode{VideoFormat} property of @var{vi}.
## H and W below refers to the height and width returned from @code{get(VI, "VideoResolution")}.
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
##     set (obj, "VideoFormat", "MJPG")
##     start (obj)
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
## Timecode, see @url{http://linuxtv.org/downloads/v4l-dvb-apis/buffer.html#v4l2-timecode}
## @end table
## @seealso {@@videoinput/start, @@videoinput/preview}
## @end deftypefn

function [img, seq, timestamp, timecode] = getsnapshot (vi, pv=0)

  if (nargin < 1 || nargin>2)
    print_usage();
  endif
  if (nargout <= 3)
    [img, seq, timestamp] = __v4l2_handler_capture__(vi.imaqh, pv);
  else
    [img, seq, timestamp, timecode] = __v4l2_handler_capture__(vi.imaqh, pv);
  endif
  #fmt = __v4l2_handler_g_fmt__(vi.imaqh).pixelformat;
  #printf ("pixelformat = -%s-\n", fmt);

endfunction

%!test
%! obj = videoinput("v4l2", __test__device__);
%! oldval = get(obj, "VideoResolution");
%! default_size = set (obj, "VideoResolution")(1,:);
%! set (obj, "VideoResolution", default_size);
%! set (obj, 'VideoFormat', 'RGB24');
%! start (obj, 2)
%! img = getsnapshot (obj);
%! img = getsnapshot (obj, 1);
%! [img, seq] = getsnapshot (obj, 1);
%! # The v4l2 loopback device doesn't support the seqence numbering and returns always 0
%! if (!strcmp(get(obj, "DeviceCapabilities").driver, "v4l2 loopback"))
%!   assert (seq >= 2);
%! endif
%! [img, seq, T] = getsnapshot(obj, 0);
%! assert (isstruct(T))
%! stop (obj)
%! set (obj, "VideoResolution", oldval);

%!test
%! obj = videoinput("v4l2", __test__device__);
%! set (obj, "VideoFormat", "RGB3")
%! start (obj)
%! img = getsnapshot (obj);
%! stop (obj)

%!test
%! obj = videoinput ("v4l2", __test__device__);
%! # see http://www.linuxtv.org/downloads/v4l-dvb-apis/V4L2-PIX-FMT-YUYV.html
%! set (obj, "VideoFormat", "YUYV")
%! start (obj)
%! img = getsnapshot (obj);
%! stop (obj)

%!test
%! obj = videoinput ("v4l2", __test__device__);
%! # see see http://www.linuxtv.org/downloads/v4l-dvb-apis/re23.html
%! set (obj, "VideoFormat", "YU12")
%! start (obj)
%! img = getsnapshot(obj);
%! stop (obj)

%!demo
%! obj = videoinput ("v4l2", __test__device__);
%! # see http://www.linuxtv.org/downloads/v4l-dvb-apis/V4L2-PIX-FMT-YUYV.html
%! set(obj,"VideoFormat","YUYV")
%! start(obj)
%! img = getsnapshot(obj);
%! tmp = cat (3, img.Y, kron(img.Cb, [1 1]), kron(img.Cr, [1 1]));
%! # convert to RGB with octave-forge image function ycbcr2rgb
%! pkg load image
%! rgb = ycbcr2rgb (tmp, "709");
%! image(rgb)
%! title ("YUYV, Standard 709")
%! stop(obj)
