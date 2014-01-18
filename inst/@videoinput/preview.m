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
## @deftypefn {Function File} {} preview (@var{vi})
## Repeatedly get a snapshot from a videoinput object buffer and display
## it in a FLTK window. Stop execution with CTRL-C.
## @end deftypefn

function preview (vi)
  if (nargin != 1)
    print_usage();
  endif
  unwind_protect
    __v4l2_handler_streamon__(vi.imaqh, 2);
    disp("Hit CTRL+C to exit")
    fflush(stdout);
    # Show preview window if it's not already shown
    __v4l2_handler_capture__(vi.imaqh, 1);
    while(__v4l2_preview_window_is_shown__(vi.imaqh))
      __v4l2_handler_capture__(vi.imaqh, 2);
    endwhile
  unwind_protect_cleanup
    __v4l2_handler_streamoff__(vi.imaqh);
  end_unwind_protect
endfunction
