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
## @deftypefn {Function File} {[@var{img}, @var{seq}, @var{t}] =} getsnapshot (@var{vi}, [@var{preview}])
## Get a snapshot from a videoinput object buffer.
## Streaming has to be enabled before calling getsnapshot.
## If @var{preview}==true the captured image is also shown in a separate FLTK window.
##
## @table @var
## @item @var{img}
## Retrieved image
## @item @var{seq}
## Set by the driver, counting the frames (not fields!) in sequence.
## @item struct @var{t}
## For input streams this is time when the first data byte was captured,
## as returned by the clock_gettime() function for the relevant clock id.
##
## @table @var
## @item @var{tv_sec}
## seconds
## @item @var{tv_usec}
## microseconds
## @end table
## @end table
## @seealso {start, preview}
## @end deftypefn

function [img, seq, timestamp] = getsnapshot (vi, pv=0)
  if (nargin < 1 || nargin>2)
    print_usage();
  endif
  [img, seq, timestamp] = __v4l2_handler_capture__(vi.imaqh, pv);
endfunction

%!test
%! obj = videoinput("v4l2", __test__device__);
%! oldval = get(obj, "VideoResolution");
%! default_size = set(obj, "VideoResolution")(1,:);
%! set(obj, "VideoResolution", default_size);
%! start(obj, 2)
%! img = getsnapshot(obj);
%! img = getsnapshot(obj, 1);
%! [img, seq]= getsnapshot(obj, 1);
%! # The v4l2 loopback device doesn't support the seqence numbering and returns always 0
%! if (!strcmp(get(obj, "DeviceCapabilities").driver, "v4l2 loopback"))
%!   %! assert (seq>=2);
%! endif
%! [img, seq, T]= getsnapshot(obj, 0);
%! assert (isstruct(T))
%! stop(obj)
%! set(obj, "VideoResolution", oldval);
