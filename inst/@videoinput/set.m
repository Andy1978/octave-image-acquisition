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
## @deftypefnx {Function File} {@var{val} =} set (@var{vi}, @var{prop}, @var{value}, @dots{})
## Set or modify properties of videoinput objects.
##
## The first variant (without @var{value}) returns a struct with possible values or range.
##
## @example
## @group
## set(obj, 'brightness')
## @result{} scalar structure containing the fields:
##
##    min = 0
##    max =  255
##    step =  1
##    default =  128
## @end group
## @end example
##
## There is a number of properties starting with an upper letter
## which are available on all videoinput devices. Only the non-obvious are
## described here.
##
## @table @var
## @item 'VideoResolution'
## Sets the width and height of the captured images. The driver attemps to
## adjust the values if the size requested is unavailable.
##
## @example
## set(obj, "VideoResolution", [960 700])
## @result{} warning: driver is sending image at 960x544 but 960x700 was requested
## @end example
##
## @item 'VideoFrameInterval'
## Time between frames in seconds as [numerator, denominator]. Conceptually fps=1/frame_interval.
##
## @item 'VideoFormat'
## Set the VideoFormat using FOURCC code (for example RGB3) or long name (RGB24).
## See set(obj, "VideoFormat") for a list.
## @end table
##
## @seealso{@@videoinput/get}
## @end deftypefn

function ret = set (vi, varargin)
  if (length (varargin) == 1) # show available values for control
    ret = __list_range__(vi, varargin{1});

  elseif (length (varargin) < 2 || rem (length (varargin), 2) != 0)
    error ('set: expecting property/value pairs');
  else
    while (length (varargin) > 1)
      prop = varargin{1};
      val = varargin{2};
      varargin(1:2) = [];
      if(ischar (prop))
        switch prop
          case 'ReturnedColorSpace'
            error ("Use set (VI, 'VideoFormat', FMT) to specify the returned image format");
          case 'BayerSensorAlignment'
            error ("Use set (VI, 'VideoFormat', FMT) to specify the returned image format");
          case 'VideoResolution'
            if (isvector (val) && isreal (val) && length (val) == 2)
              __v4l2_handler_s_fmt__(vi.imaqh, "", val);
            else
              error ('set VideoResolution: expects a real vector [width height]');
            endif
          case 'VideoInput'
            if (isscalar (val) && isreal (val))
              __v4l2_handler_s_input__(vi.imaqh, val);
            else
              error ('set VideoInput: expecting the value to be a scalar integer');
            endif
          case 'VideoFrameInterval'
            if (ismatrix (val) && isreal (val) && numel (val) == 2)
              __v4l2_handler_s_parm__(vi.imaqh, val);
            else
              error ('set VideoFrameInterval: expecting a 1x2 matrix with [numerator, denominator]');
            endif
          case 'VideoFormat'
            if (ischar (val))
              __v4l2_handler_s_fmt__(vi.imaqh, val, [0 0]);
            else
              error ('set VideoFormat: expecting a string');
            endif
          otherwise
            if (!__is_read_only_property__(prop))
              # could be a v4l2 control
              ctrls = __v4l2_handler_queryctrl__(vi.imaqh);
              if (isfield(ctrls, prop))
                __v4l2_handler_s_ctrl__(vi.imaqh, ctrls.(prop).id, val);
                v = __v4l2_handler_g_ctrl__(vi.imaqh, ctrls.(prop).id);
                if ( val != v)
                  warning("v4l2 driver limited set value %d to %d", val, v);
                endif
              else
                error ('set: invalid property of videoinput class');
              endif
            endif
        endswitch
      endif
    endwhile
  endif
endfunction

## List possible range or values for prop
function ret = __list_range__ (vi, prop)
  if (prop)
    if (!__is_read_only_property__ (prop))
      switch prop
        case 'ReturnedColorSpace'
          error ("Use set (VI, 'VideoFormat') to get a list of supported formats.")
        case 'BayerSensorAlignment'
          error ("Use set (VI, 'VideoFormat') to get a list of supported formats.")
        case 'VideoInput'
          # enumerate available inputs
          ret = __v4l2_handler_enuminput__ (vi.imaqh);
        case 'VideoResolution'
          # enumerate possible framerates
          fmt = __v4l2_handler_g_fmt__(vi.imaqh).pixelformat;
          ret = __v4l2_handler_enum_framesizes__ (vi.imaqh, fmt);
        case 'VideoFrameInterval'
          # return possible frameintervals for currently selected framesize
          fmt = __v4l2_handler_g_fmt__(vi.imaqh).pixelformat;
          current_frame_size = __v4l2_handler_g_fmt__ (vi.imaqh).size;
          ret = __v4l2_handler_enum_frameintervals__ (vi.imaqh, current_frame_size, fmt);
        case 'VideoFormat'
          ret = __v4l2_handler_enum_fmt__ (vi.imaqh);
        otherwise ## perhaps a v4l2 control?
          ctrls = __v4l2_handler_queryctrl__(vi.imaqh);
          if (isfield(ctrls, prop))
            # yes, it is a v4l2 property
            ret = getfield(ctrls, prop);
            ret = rmfield(ret , 'id'); # mask id
          else
            error('unknown property %s', prop);
          endif
      endswitch
    endif
  else
    error('PROP has to be a character array');
  endif
endfunction

function ret = __is_read_only_property__ (prop)
  switch prop
    case {'SelectedSourceName', 'DeviceCapabilities'}
      error('%s is a read-only property', prop);
      ret = 1;
    otherwise
      ret = 0;
  endswitch
endfunction

%!test
%! obj = videoinput ("v4l2", __test__device__);
%! svi = set (obj, "VideoInput");
%! assert (isstruct(svi))
%! set(obj, "VideoInput", 0);
%! s = set(obj, "VideoResolution");
%! assert (ismatrix(s))
%! set (obj, "VideoResolution", s(1,:))
%! if (rows(s)>1)
%!  set (obj, "VideoResolution", s(end,:))
%! endif

%!error set(obj, "SelectedSourceName")
%!error set(obj, "DeviceCapabilities")

%!test
%! obj = videoinput ("v4l2", __test__device__);
%! set (obj, 'VideoFormat', 'RGB24');

%!test
%! obj = videoinput ("v4l2", __test__device__);
%! fmts = set (obj, 'VideoFormat');
%! set (obj, 'VideoFormat', fmts(end).pixelformat);
%! set (obj, 'VideoFormat', 'RGB24');

%!test
%! obj = videoinput ("v4l2", __test__device__);
%! T = set (obj, 'VideoFrameInterval');
%! # not all drives support enumeration and query of trameintervals
%! if (rows(T) >= 1)
%!   set (obj, 'VideoFrameInterval', T(1,:));
%!   set (obj, 'VideoFrameInterval', T(end,:));
%! endif

%!warning
%! obj = videoinput ("v4l2", __test__device__);
%! # This shouldn't be supported by any camera and the driver
%! # clamps this to valid values but a warning should be displayed
%! set (obj, 'VideoFrameInterval', [1 10000]);
