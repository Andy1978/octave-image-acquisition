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
## @deftypefn {Function File} {@var{rgb} =} SGRBGtoRGB (@var{raw})
## Convert RAW bayer image with SGRBG filter alignment to RGB image
## without interpolation.
##
## FIXME: We should implement "demosaic" instead and use this.
## @end deftypefn

function rgb = SGRBGtoRGB(img)

  # cast to uint16 to avoid overflow
  # in average calculation g = (g1+g2)/2 below
  g1 = uint16 (img(1:2:end, 1:2:end));
  g2 = uint16 (img(2:2:end, 2:2:end));
  g = (g1 + g2) / 2;
  r = img(1:2:end, 2:2:end);
  b = img(2:2:end, 1:2:end);
  rgb = cat (3, r, g, b);

endfunction
