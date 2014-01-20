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
## @deftypefn {Function File} {@var{list} =} imaqhwinfo ()
## Returns a struct array with v4l2 devices in /dev/. Links are not resolved.
##
## @group
## @example
## imaqhwinfo()
## @result{} scalar structure containing the fields:
##
##    driver = uvcvideo
##    card = UVC Camera (046d:0825)
##    bus_info = usb-0000:00:16.2-2
##    version = 3.2.51
##    capabilities =  83886081
##    device = /dev/video0
## @end example
## @end group
## @end deftypefn

function ret = imaqhwinfo()
  ret = __v4l2_list_devices__ ();
endfunction

%!test
%! d = imaqhwinfo();
