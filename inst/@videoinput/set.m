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
## this program; if not, see <http:##www.gnu.org/licenses/>.<http://www.gnu.org/licenses/>.

## -*- texinfo -*-
## @deftypefn {Function File} {@var{props} =} set (@var{vi}, @var{prop})
## @deftypefnx {Function File} {@var{val} =} set (@var{vi}, @var{prop}, @var{value})
## Set or modify properties of videoinput objects.
##
## TODO: please document me and add examples, see get
##
## @seealso{get}
## @end deftypefn

function ret = set (vi, varargin)
  if (length (varargin) == 1) ## show available values for controls
    prop = varargin{1};
    ctrls = __v4l2_handler_queryctrl__(vi.imaqh);
      if (isfield(ctrls, prop))
        ret = getfield(ctrls, prop);
        ret = rmfield(ret , "id");
      else
        # TODO: implement that
        # set(obj, "VideoInput" calls __v4l2_handler_enuminput__
        # set(obj, "VideoResolution" calls __v4l2_handler_enum_framesizes__
        # set(obj, "VideoFrameIntervals" calls __v4l2_handler_enum_frameintervals__
        error("only implemented for v4l2 video controls")
      endif
  elseif (length (varargin) < 2 || rem (length (varargin), 2) != 0)
    error ("set: expecting property/value pairs");
  else
    while (length (varargin) > 1)
      prop = varargin{1};
      val = varargin{2};
      varargin(1:2) = [];
      if (ischar (prop) && strcmp (prop, "VideoResolution"))
        if (isvector (val) && isreal (val) && length (val) == 2)
          __v4l2_handler_s_fmt__(vi.imaqh, val);
        else
          error ("set: expecting the value to be a real vector [width height]");
        endif
      elseif (ischar (prop) && strcmp (prop, "VideoInput"))
        if (isscalar (val) && isreal (val))
          __v4l2_handler_s_input__(vi.imaqh, val);
        else
          error ("set: expecting the value to be a integer");
        endif
      else  #ctrls
        ctrls = __v4l2_handler_queryctrl__(vi.imaqh);
        if (isfield(ctrls, prop))
          __v4l2_handler_s_ctrl__(vi.imaqh, ctrls.(prop).id, val);
        else
          error ("set: invalid property of videoinput class");
        endif
      endif
    endwhile
  endif
endfunction
