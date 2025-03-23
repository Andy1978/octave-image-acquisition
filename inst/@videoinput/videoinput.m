## Copyright (C) 2024-2025 Andreas Weber <andy.weber.aw@gmail.com>
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
## @deftypefn {Function File} {@var{vi} =} videoinput ([@var{adaptorname}, @var{device}, [@var{format}]])
## Initializes a new video input object. Currently only "v4l2" and "mf" is available as adaptorname.
## If called without arguments, the first available device is used.
##
## @group
## @example
## obj = videoinput("v4l2", "/dev/video0")
## @result{} obj = videoinput
##      device             = /dev/video0
##      driver             = uvcvideo
##      card               = UVC Camera (046d:0825)
##      VideoInput         = 0
##      VideoResolution    = 960 x 720 px
##      VideoFormat = MJPEG
## @end example
## @end group
## @seealso{imaqhwinfo, @@videoinput/getsnapshot}
## @end deftypefn

function vi = videoinput (adaptorname, device, format)

  if (nargin == 0)
    l = imaqhwinfo ();
    n = numel (l);
    if (n == 0)
      error("It appears that you do not have have a compatible video capture device installed.");
    else
      # silently ignore if there is more than one device
      if (ispc ())
        adaptorname = "mf";
        vidata.PrettyName = l(1).name;
        device = l(1).symlink;
      elseif (isunix ())
        adaptorname = "v4l2";
        device = l(1).device;
      endif
    endif
  elseif (nargin == 1 || nargin > 3 || (nargin >= 2 && ! ischar(device)))
    print_usage();
  endif

  vidata.SelectedSourceName = device;
  vidata.imaqh = __imaq_handler_open__(adaptorname, device);
  vi = class (vidata, "videoinput");

  if (nargin == 3)
    set (vi, "VideoResolution", format);
  endif

endfunction

%!test
%! obj = videoinput(__test__device__(){:});
