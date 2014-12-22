## Copyright (C) 2013 Andreas Weber <andy.weber.aw@gmail.com>
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

function display (vi)
  printf("%s = videoinput for v4l2\n", inputname(1));
  sp = repmat(' ',1,length(inputname(1))+3);
  printf("%sdevice               = %s\n", sp, get(vi, "SelectedSourceName"));

  caps = get(vi, "DeviceCapabilities");
  printf("%sdriver               = %s\n", sp, caps.driver);
  printf("%scard                 = %s\n", sp, caps.card);

  printf("%sVideoInput           = %d\n", sp, get(vi, "VideoInput"));
  s = get(vi, "VideoResolution");
  printf("%sVideoResolution      = %d x %d px\n", sp, s(1), s(2));

  fmt = get(vi, "VideoFormat");
  printf("%sVideoFormat          = %s\n", sp, fmt);

  # not supported by all devices (for example media interfaces subdefs)
  # FIXME what should we do?
  # T = get(vi, "VideoFrameInterval");
  # printf("%sVideoFrameInterval   = %d/%d s (%.1f fps)\n", sp, T(1), T(2), T(2)/T(1));

endfunction
