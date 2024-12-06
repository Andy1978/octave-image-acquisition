// Copyright (C) 2014-2024 Andreas Weber <andy.weber.aw@gmail.com>
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

#include <cassert>
#include "cl_imaq_handler.h"

DEFINE_OV_TYPEID_FUNCTIONS_AND_DATA(imaq_handler, "imaq_handler", "imaq_handler");

bool imaq_handler::type_loaded = false;

imaq_handler::imaq_handler ()
  : octave_base_value(), preview_window(0)
{
  //octave_stdout << "imaq_handler C'Tor, type_loaded = " << type_loaded << endl;
  if (!type_loaded)
    {
      type_loaded = true;
      imaq_handler::register_type();
      //octave_stdout << "imaq_handler C'Tor, type_id() = " << type_id() << std::endl;
    }
}

//~ v4l2_handler::v4l2_handler (const v4l2_handler& m)
  //~ : octave_base_value()
//~ {
  //~ octave_stdout << "v4l2_handler: the copy constructor shouldn't be called" << std::endl;
//~ }

imaq_handler::~imaq_handler ()
{
  //octave_stdout << "imaq_handler D'Tor " << endl;

  // delete preview_window if active
  if (preview_window)
    {
      delete preview_window;
      preview_window = 0;
    }

  // stop streaming, unmap & free buffers, close v4l2 device
  close();
}

void
imaq_handler::print (std::ostream& os, bool pr_as_read_syntax = false)
{
  os << "This is class imaq_handler" << endl;
  //os << "dev = " << dev << ", fd = " << fd << ", n_buffer = " << n_buffer << ", streaming = " << ((streaming)? "true":"false") << endl;
}


octave_scalar_map
imaq_handler::open (string d, bool quiet)
{
  octave_scalar_map ret;
  octave_stdout << "imaq_handler::open (d = " << d << " called" << std::endl;
  return ret;
}

void
imaq_handler::close ()
{
  //streamoff();
  //if (fd >= 0)
  //  v4l2_close(fd);
  //fd = -1;
}

imaq_handler*
get_imaq_handler_from_ov (octave_value ov)
{
  //octave_stdout << "get_imaq_handler_from_ov ov.type_id() = " << ov.type_id() << std::endl;
  if (ov.type_id() != imaq_handler::static_type_id())
    {
      error("get_imaq_handler_from_ov: Not a valid imaq_handler");
      return 0;
    }

  imaq_handler* imgh = 0;
  const octave_base_value& rep = ov.get_rep();
  imgh = &((imaq_handler &)rep);
  return imgh;
}
