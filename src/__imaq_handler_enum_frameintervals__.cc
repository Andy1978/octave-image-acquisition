// Copyright (C) 2014 Andreas Weber <andy.weber.aw@gmail.com>
//
// This program is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free Software
// Foundation; either version 3 of the License, or (at your option) any later
// version.
//
// This program is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
// details.
//
// You should have received a copy of the GNU General Public License along with
// this program; if not, see <http://www.gnu.org/licenses/>.

#include <octave/oct.h>
#include "__imaq_handler__.h"

DEFUN_DLD(__imaq_handler_enum_frameintervals__, args, nargout,
          "-*- texinfo -*-\n\
@deftypefn {Loadable Function} {@var{T} = } __imaq_handler_enum_frameintervals__ (@var{h}, @var{size})\n\
@deftypefnc {Loadable Function} {@var{T} = } __imaq_handler_enum_frameintervals__ (@var{h}, @var{size}, @var{format})\n\
Enumerate available frame intervals [s] from imaq_handler @var{h}.\n\
If no format is given, V4L2_PIX_FMT_RGB24 is assumed (TODO: implement me with string constants?!?).\n\
@end deftypefn")
{
  octave_value_list retval;
  int nargin = args.length();

  if (nargin < 2 || nargin>3)
    {
      print_usage();
      return retval;
    }

  imaq_handler* imgh = get_imaq_handler_from_ov(args(0));
  if (imgh)
    {
      Matrix s = args(1).matrix_value();
      unsigned int width = s(0);
      unsigned int height = s(1);
      if (error_state)
        {
          print_usage();
        }

      unsigned int pixel_format = V4L2_PIX_FMT_RGB24;
      if (nargin == 3)
        {
          unsigned int tmp_pixel_format = args(1).int_value();
          if (!error_state)
            pixel_format = tmp_pixel_format;
          else
            error("FORMAT not valid");
        }
      retval = octave_value(imgh->enum_frameintervals(pixel_format, width, height));
    }
  return retval;
}
