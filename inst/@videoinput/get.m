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
## Access property values of videoinput objects.

function val = get (vi, prop)
  
  switch (prop)
    case "SelectedSourceName"
      val = vi.SelectedSourceName;
    case "VideoResolution"
      val = __imaq_handler_g_fmt__(vi.imaqh);
    otherwise
      error ("videoinput: get: invalid property name '%s'", prop);
  endswitch

endfunction
