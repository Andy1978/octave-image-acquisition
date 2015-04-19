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
## @deftypefn {Function File} {@var{dev} =} __test__device__ ()
## Return device which should be used in tests, for example "/dev/video0"
## @end deftypefn

function dev = __test__device__()
  persistent warning_shown = 0;
  l = imaqhwinfo();
  if ( numel(l) > 1)
    dev = l(1).device;
    # only show warning once
    if (!warning_shown)
      warning("It appears that you have more than one v4l2 device. We will just use %s (the first returned from imaqhwinfo) for tests.", dev);
      warning_shown = 1;
    endif
  elseif (numel(l) == 0)
    warning("It appears that you have no v4l2 device installed. All tests may fail. Please connect one or try\n       $ modprobe v4l2loopback\n       $ gst-launch videotestsrc ! v4l2sink device=/dev/video0");
    dev = "/dev/null";
  else
    dev = l.device;
  endif
endfunction

%!assert (1)
