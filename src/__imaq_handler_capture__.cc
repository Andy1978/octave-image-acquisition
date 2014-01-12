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

DEFUN_DLD(__imaq_handler_capture__, args, nargout,
          "-*- texinfo -*-\n\
@deftypefn {Loadable Function} {@var{f} =} __imaq_handler_capture__ (@var{h},[@var{preview}])\n\
Get a snapshot from imaq_handler @var{h}\n\
@end deftypefn")
{
  octave_value_list retval;
  int nargin = args.length();

  if (nargin < 1 || nargin>2)
    {
      print_usage();
      return retval;
    }

  imaq_handler* imgh = get_imaq_handler_from_ov(args(0));
  if (imgh)
    {
      int preview = 0;
      if (nargin==2)
        preview = args(1).int_value();
      if (!error_state)
        {
          retval = octave_value(imgh->capture(nargout, preview));
        }
    }
  return retval;
}
