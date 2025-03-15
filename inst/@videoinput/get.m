## Copyright (C) 2014-2025 Andreas Weber <andy.weber.aw@gmail.com>
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
## @deftypefn {Function File} {@var{props} =} get (@var{vi})
## @deftypefnx {Function File} {@var{val} =} get (@var{vi}, @var{prop})
## Access property values of videoinput objects.
##
## Without @var{prop} a cell with available properties is returned.
## The entries starting with a upper letter are available on all devices, the others
## queried from and specific to the used driver.
##
## @example
## @group
## obj = videoinput ("v4l2", "/dev/video0");
## get (obj)
## @result{}
##  @{
##    [1,1] = SelectedSourceName
##    [2,1] = ReturnedColorSpace
##    [3,1] = BayerSensorAlignment
##    [4,1] = DeviceCapabilities
##    [5,1] = VideoInput
##    [6,1] = VideoResolution
##    [7,1] = VideoFrameInterval
##    [8,1] = VideoFormat
##    [9,1] = user_controls
##    [10,1] = brightness
##    [11,1] = contrast
##    [12,1] = saturation
##    [13,1] = white_balance_automatic
##    [14,1] = gain
##    [15,1] = power_line_frequency
##    [16,1] = white_balance_temperature
##    [17,1] = sharpness
##    [18,1] = backlight_compensation
##    [19,1] = camera_controls
##    [20,1] = auto_exposure
##    [21,1] = exposure_time_absolute
##    [22,1] = exposure_dynamic_framerate
##  @}
## @end group
## @end example
##
## @example
## @group
## obj = videoinput("v4l2", "/dev/video0");
## get (obj, "SelectedSourceName")
##   @result{} /dev/video0
## @end group
## @end example
##
## @example
## @group
## obj = videoinput ("v4l2", "/dev/video0");
## set(obj, "VideoResolution", [640 480]);
## get (obj, "VideoResolution")
##   @result{}    640   480
## @end group
## @end example
## @seealso{@@videoinput/set}
## @end deftypefn

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
        val = __imaq_handler_get_input__(vi.imaqh);
      case "VideoFrameInterval"
        val = __imaq_handler_get_frameinterval__(vi.imaqh);
      case "VideoResolution"
        val = __imaq_handler_g_fmt__(vi.imaqh).size;
      case "VideoFormat"
        val = __imaq_handler_g_fmt__(vi.imaqh).fourcc;
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

%!test
%! obj = videoinput(__test__device__{:});
%! props = get(obj);
%! assert(get(obj, "SelectedSourceName"), __test__device__{2});
%! caps = get(obj, "DeviceCapabilities");
%! video_in = get(obj, "VideoInput");
%! s = get(obj, "VideoResolution");

%!test
%! obj = videoinput(__test__device__{:});
%! T = get(obj, "VideoFrameInterval");

%!test
%! obj = videoinput(__test__device__{:});
%! f = get(obj, "VideoFormat");

%!error <invalid property name> get(videoinput(__test__device__{:}), "there_is_no_such_property")
