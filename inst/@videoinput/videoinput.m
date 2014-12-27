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
## @deftypefn {Function File} {@var{vi} =} videoinput (@var{adaptorname}, @var{device})
## @deftypefnx {Function File} {@var{vi} =} videoinput (@var{adaptorname}, @var{device}, @var{format})
## Initializes a new video input object. Currently only v4l2 is available as adaptorname.
##
## @group
## @example
## octave:> obj = videoinput("v4l2", "/dev/video0")
## @result{} obj = videoinput for v4l2
##      device             = /dev/video0
##      driver             = uvcvideo
##      card               = UVC Camera (046d:0825)
##      VideoInput         = 0
##      VideoResolution    = 1280 x 960 px
##      VideoFrameInterval = 1/30 s (30.0 fps)
## @end example
## @end group
## @seealso{@@videoinput/getsnapshot}
## @end deftypefn

function vi = videoinput (adaptorname, device, format)
  # defaults
  vidata = struct ("SelectedSourceName", "/dev/video0");

  if (nargin == 0 || nargin==1 || nargin>3)
    print_usage();
  endif
  if (nargin == 2)
    if (strcmp(adaptorname, "v4l2"))
      if (ischar(device))
        vidata.SelectedSourceName = device;
      else
        print_usage();
      endif
    else
      error("Only v4l2 adaptors are possible yet")
    endif
  endif

  vidata.imaqh = __v4l2_handler_open__(vidata.SelectedSourceName);
  vi = class (vidata, "videoinput");

  if (nargin == 3)
    set(vi, "VideoResolution", format);
  endif

endfunction

%!test
%! obj = videoinput("v4l2", __test__device__);
