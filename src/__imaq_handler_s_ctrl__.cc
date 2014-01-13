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

DEFUN_DLD(__imaq_handler_s_ctrl__, args, nargout,
          "-*- texinfo -*-\n\
@deftypefn {Loadable Function} {} __imaq_handler_s_ctrl__ (@var{h}, @var{id}, @var{value})\n\
Set control @var{id} like brightness, contrast, saturation etc. in imaq_handler @var{h}.\n\
Use the field id from __imaq_handler_queryctrl__.\n\
@seealso{__imaq_handler_queryctrl__}\n\
@end deftypefn")
{
  octave_value_list retval;
  int nargin = args.length();

  if (nargin != 3)
    {
      print_usage();
      return retval;
    }

  imaq_handler* imgh = get_imaq_handler_from_ov(args(0));
  if (imgh)
    {
      unsigned int id = args(1).int_value();
      unsigned int value = args(2).int_value();
      if (!error_state)
        imgh->s_ctrl(id, value);
      else
        error("ID and VALUE has to be integer values");
    }
  return retval;
}
