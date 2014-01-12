// Copyright (C) 2013 Andreas Weber <andy.weber.aw@gmail.com>
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

static bool type_loaded = false;

DEFUN_DLD(__imaq_handler_open__, args, nargout,
          "-*- texinfo -*-\n\
@deftypefn {Loadable Function} {@var{h} =} __imaq_handler_open__ (@var{device})\n\
Creates an instance of imaq_handler for a v4l2 device and opens it.\n\
@seealso{getsnapshot}\n\
@end deftypefn")
{
  octave_value_list retval;
  int nargin = args.length ();

  if (nargin != 1)
    {
      print_usage();
      return retval;
    }

  if (!type_loaded)
    {
      imaq_handler::register_type();
      type_loaded = true;
    }
  string device = args(0).string_value();
  if (! error_state)
    {
      imaq_handler *h = new imaq_handler();
      h->open(device.c_str());
      retval.append(octave_value(h));
    }
  return retval;
}
