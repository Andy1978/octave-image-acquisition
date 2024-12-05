// Copyright (C) 2024 Andreas Weber <andy.weber.aw@gmail.com>
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
#include <dirent.h>
//#include <sys/types.h>
#include "cl_mf_handler.h"

//DEFINE_OV_TYPEID_FUNCTIONS_AND_DATA(mf_handler, "mf_handler", "mf_handler");

//bool mf_handler::type_loaded = false;

mf_handler::mf_handler ()
  : imaq_handler() /*,
    fd(-1), n_buffer(0), buffers(0), streaming(0),
    preview_window(0), _is_video_capture (0), _is_meta_capture(0)*/
{
  octave_stdout << "mf_handler C'Tor" << endl;
  octave_stdout << "mf_handler C'Tor, type_id() = " << type_id() << std::endl;

  //~ if (!type_loaded)
    //~ {
      //~ type_loaded = true;
      //~ register_type();
    //~ }
}

mf_handler::mf_handler (const mf_handler& m)
  : imaq_handler()
{
  octave_stdout << "mf_handler: the copy constructor shouldn't be called" << std::endl;
}

mf_handler::~mf_handler ()
{
  octave_stdout << "mf_handler D'Tor " << endl;

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
mf_handler::print (std::ostream& os, bool pr_as_read_syntax = false)
{
  os << "This is class mf_handler" << endl;
  //os << "dev = " << dev << ", fd = " << fd << ", n_buffer = " << n_buffer << ", streaming = " << ((streaming)? "true":"false") << endl;
}

typedef std::vector<std::string> dev_vec;

octave_map
mf_handler::list_devices ()
{
  octave_map retval;
  dev_vec files;
  files.push_back(std::string("foo"));
  files.push_back(std::string("bar"));
  files.push_back(std::string("baz"));

  octave_idx_type i = 0;
  for (dev_vec::iterator iter = files.begin();
      iter != files.end(); ++iter)
    {
      printf ("trying '%s'...\n", iter->c_str());
      fflush(stdout);
      
      //v4l2_handler h;
      octave_scalar_map caps;// = h.open(iter->c_str(), true);

      //if (! h.is_meta_capture ())
      {
        caps.assign ("device", *iter);
        retval.assign(i++, caps);
      }
      //else
        //printf ("INFO: list device ignores metadata interface '%s'...\n", iter->c_str());
    }
  return retval;
}

/*
// calls to xioctl should never fail.
// If it fails something unexpected happened
void
v4l2_handler::xioctl_name (int fh, unsigned long int request, void *arg, const char* name, const char* file, const int line)
{
  int r;
  do
    {
      r = v4l2_ioctl(fh, request, arg);
    }
  while (r == -1 && ((errno == EINTR) || (errno == EAGAIN)));

  if (r == -1)
    {
      error("%s:%i xioctl %s error %d, %s\n", file, line, name, errno, strerror(errno));
    }
}
*/

octave_scalar_map
mf_handler::open (string d, bool quiet)
{
  octave_scalar_map ret;
  
  octave_stdout << "mf_handler::open d = " << d << " called" << std::endl;
  
/*
  fd = v4l2_open(d.c_str(), O_RDWR | O_NONBLOCK, 0);
  if (fd < 0)
    {
      error("Cannot open device '%s'. Error %d, '%s'\n", d.c_str(), errno, strerror(errno));
    }
  else
    {
      dev = d; // store device path for later info output
	  ret = querycap ().scalar_map_value ();

	  if (!quiet && is_meta_capture ())
	    warning ("Device '%s' is a metadata interface device", d.c_str());
    }
*/
  return ret;
}

void
mf_handler::close ()
{
  //~ streamoff();
  //~ if (fd >= 0)
    //~ v4l2_close(fd);
  //~ fd = -1;
}

//~ v4l2_handler*
//~ get_v4l2_handler_from_ov (octave_value ov)
//~ {
  //~ if (ov.type_id() != v4l2_handler::static_type_id())
    //~ {
      //~ error("get_v4l2_handler_from_ov: Not a valid v4l2_handler");
      //~ return 0;
    //~ }

  //~ v4l2_handler* imgh = 0;
  //~ const octave_base_value& rep = ov.get_rep();
  //~ imgh = &((v4l2_handler &)rep);
  //~ return imgh;
//~ }
