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
## @deftypefn {Function File} {} start (@var{vi}, @var{n})
## Start streaming with @var{n} buffers. It is recommended to use at least 2 buffers.
## @seealso{@@videoinput/stop}
## @end deftypefn

function start (vi, n = 2)
  if (nargin > 2 || nargin < 1)
    print_usage();
  endif
  __v4l2_handler_streamon__(vi.imaqh, n);
endfunction

# already tested in getsnapshot
