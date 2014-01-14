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
  if (nargin > 2)
    print_usage();
  endif
  if (nargin == 1)
    ctrls = __imaq_handler_queryctrl__(vi.imaqh);
    val = vertcat(__property_names__(vi), fieldnames(ctrls));
  else
    switch (prop)
      case "SelectedSourceName"
        val = vi.SelectedSourceName;
      case "DeviceCapabilities"
        val = __imaq_handler_querycap__(vi.imaqh);
      case "VideoInput"
        val = __imaq_handler_g_input__(vi.imaqh);
      case "VideoResolution"
        val = __imaq_handler_g_fmt__(vi.imaqh);
      otherwise
        # get controls
        ctrls = __imaq_handler_queryctrl__(vi.imaqh);
        if (isfield(ctrls, prop))
          val = __imaq_handler_g_ctrl__(vi.imaqh, ctrls.(prop).id);
        else
          error ("videoinput: get: invalid property name '%s'", prop);
        endif
    endswitch
  endif

endfunction
