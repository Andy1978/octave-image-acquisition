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
## @seealso{getsnapshot}
## @end deftypefn

## Author:  Andreas Weber <andy.weber.aw@gmail.com>
## Created: January 2014
## Version: 0.1

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

  vidata.imaqh = __imaq_handler_open__(vidata.SelectedSourceName);
  vi = class (vidata, "videoinput");
  
  if (nargin == 3)
    set(vi, "VideoResolution", format);
  endif
  
endfunction

%!test
%! huhu = 5
