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
#include <sys/types.h>
#include <dirent.h>
#include "cl_v4l2_handler.h"

static bool type_loaded = false;

// PKG_ADD: autoload ("__v4l2_handler_open__", which ("__v4l2_handler__.oct"));
// PKG_DEL: autoload ("__v4l2_handler_open__", which ("__v4l2_handler__.oct"), "remove");
DEFUN_DLD(__v4l2_handler_open__, args, nargout,
          "-*- texinfo -*-\n\
@deftypefn {Loadable Function} {@var{h} =} __v4l2_handler_open__ (@var{device})\n\
Creates an instance of v4l2_handler for a v4l2 device and opens it.\n\
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
      v4l2_handler::register_type();
      type_loaded = true;
    }
  string device = args(0).string_value ();
  if (! error_state)
    {
      v4l2_handler *h = new v4l2_handler ();
      h->open (device.c_str ());
      retval.append (octave_value (h));
    }
  return retval;
}

// PKG_ADD: autoload ("__v4l2_handler_querycap__", which ("__v4l2_handler__.oct"));
// PKG_DEL: autoload ("__v4l2_handler_querycap__", which ("__v4l2_handler__.oct"), "remove");
DEFUN_DLD(__v4l2_handler_querycap__, args, nargout,
          "-*- texinfo -*-\n\
@deftypefn {Loadable Function} {@var{c} = } __v4l2_handler_querycap__ (@var{h})\n\
Query device capabilities, driver name, card type etc. from v4l2_handler @var{h}.\n\
@end deftypefn")
{
  octave_value_list retval;
  int nargin = args.length ();

  if (nargin != 1)
    {
      print_usage ();
      return retval;
    }

  v4l2_handler* imgh = get_v4l2_handler_from_ov (args(0));
  if (imgh)
    {
      retval = imgh->querycap ();
    }
  return retval;
}
// INPUTS

// PKG_ADD: autoload ("__v4l2_handler_enuminput__", which ("__v4l2_handler__.oct"));
// PKG_DEL: autoload ("__v4l2_handler_enuminput__", which ("__v4l2_handler__.oct"), "remove");
DEFUN_DLD(__v4l2_handler_enuminput__, args, nargout,
          "-*- texinfo -*-\n\
@deftypefn {Loadable Function} {@var{inputs} = } __v4l2_handler_enuminput__ (@var{h})\n\
Enumerate video inputs from v4l2_handler @var{h}.\n\
Returns a struct with informations for all avaliable v4l2 inputs.\n\
@end deftypefn")
{
  octave_value_list retval;
  int nargin = args.length ();

  if (nargin != 1)
    {
      print_usage ();
      return retval;
    }

  v4l2_handler* imgh = get_v4l2_handler_from_ov (args(0));
  if (imgh)
    {
      retval = imgh->enuminput ();
    }
  return retval;
}

// PKG_ADD: autoload ("__v4l2_handler_g_input__", which ("__v4l2_handler__.oct"));
// PKG_DEL: autoload ("__v4l2_handler_g_input__", which ("__v4l2_handler__.oct"), "remove");
DEFUN_DLD(__v4l2_handler_g_input__, args, nargout,
          "-*- texinfo -*-\n\
@deftypefn {Loadable Function} {@var{N} =} __v4l2_handler_g_input__ (@var{h})\n\
Query the current video input from v4l2_handler @var{h}.\n\
@end deftypefn")
{
  octave_value_list retval;
  int nargin = args.length ();
  if (nargin != 1)
    {
      print_usage ();
      return retval;
    }

  v4l2_handler* imgh = get_v4l2_handler_from_ov (args(0));
  if (imgh)
    {
      retval = octave_value(imgh->g_input ());
    }
  return retval;
}

// PKG_ADD: autoload ("__v4l2_handler_s_input__", which ("__v4l2_handler__.oct"));
// PKG_DEL: autoload ("__v4l2_handler_s_input__", which ("__v4l2_handler__.oct"), "remove");
DEFUN_DLD(__v4l2_handler_s_input__, args, nargout,
          "-*- texinfo -*-\n\
@deftypefn {Loadable Function} {} __v4l2_handler_s_input__ (@var{h}, @var{n})\n\
Select video input @var{n} from v4l2_handler @var{h}.\n\
@end deftypefn")
{
  octave_value_list retval;
  int nargin = args.length ();

  if (nargin != 2)
    {
      print_usage ();
      return retval;
    }

  v4l2_handler* imgh = get_v4l2_handler_from_ov (args(0));
  if (imgh)
    {
      int num = args(1).int_value ();
      if (!error_state)
        imgh->s_input (num);
      else
        error("N has to be a integer selecting the desired video input, starting from  0.");
    }
  return retval;
}
// FORMAT

// PKG_ADD: autoload ("__v4l2_handler_enum_fmt__", which ("__v4l2_handler__.oct"));
// PKG_DEL: autoload ("__v4l2_handler_enum_fmt__", which ("__v4l2_handler__.oct"), "remove");
DEFUN_DLD(__v4l2_handler_enum_fmt__, args, nargout,
          "-*- texinfo -*-\n\
@deftypefn {Loadable Function} {@var{formats} = } __v4l2_handler_enum_fmt__ (@var{h})\n\
Enumerate image formats from v4l2_handler @var{h}.\n\
Returns a struct with informations for all avaliable v4l2 formats.\n\
@end deftypefn")
{
  octave_value_list retval;
  int nargin = args.length ();

  if (nargin != 1)
    {
      print_usage ();
      return retval;
    }

  v4l2_handler* imgh = get_v4l2_handler_from_ov (args(0));
  if (imgh)
    {
      retval = imgh->enum_fmt ();
    }
  return retval;
}

// PKG_ADD: autoload ("__v4l2_handler_enum_framesizes__", which ("__v4l2_handler__.oct"));
// PKG_DEL: autoload ("__v4l2_handler_enum_framesizes__", which ("__v4l2_handler__.oct"), "remove");
DEFUN_DLD(__v4l2_handler_enum_framesizes__, args, nargout,
          "-*- texinfo -*-\n\
@deftypefn {Loadable Function} {@var{sizes} = } __v4l2_handler_enum_framesizes__ (@var{h}, @var{format})\n\
Enumerate available frame sizes from v4l2_handler @var{h}.\n\
@end deftypefn")
{
  octave_value_list retval;
  int nargin = args.length ();

  if (nargin != 2)
    {
      print_usage ();
      return retval;
    }

  v4l2_handler* imgh = get_v4l2_handler_from_ov (args(0));
  if (imgh)
    {
      string pixel_format = args(1).string_value ();
      retval = octave_value(imgh->enum_framesizes (pixel_format));
    }
  return retval;
}

// PKG_ADD: autoload ("__v4l2_handler_enum_frameintervals__", which ("__v4l2_handler__.oct"));
// PKG_DEL: autoload ("__v4l2_handler_enum_frameintervals__", which ("__v4l2_handler__.oct"), "remove");
DEFUN_DLD(__v4l2_handler_enum_frameintervals__, args, nargout,
          "-*- texinfo -*-\n\
@deftypefn {Loadable Function} {@var{T} = } __v4l2_handler_enum_frameintervals__ (@var{h}, @var{size}, @var{format})\n\
Enumerate available frame intervals from v4l2_handler @var{h}.\n\
Return a Nx2 matrix with numerator, denominator.\n\
@end deftypefn")
{
  octave_value_list retval;
  int nargin = args.length ();

  if (nargin != 3)
    {
      print_usage ();
      return retval;
    }

  v4l2_handler* imgh = get_v4l2_handler_from_ov (args(0));
  if (imgh)
    {
      Matrix s = args(1).matrix_value ();
      unsigned int width = s(0);
      unsigned int height = s(1);
      if (error_state)
        {
          print_usage();
        }
      string pixel_format = args(2).string_value ();
      retval = octave_value(imgh->enum_frameintervals (pixel_format, width, height));
    }
  return retval;
}

// PKG_ADD: autoload ("__v4l2_handler_g_parm__", which ("__v4l2_handler__.oct"));
// PKG_DEL: autoload ("__v4l2_handler_g_parm__", which ("__v4l2_handler__.oct"), "remove");
DEFUN_DLD(__v4l2_handler_g_parm__, args, nargout,
          "-*- texinfo -*-\n\
@deftypefn {Loadable Function} {@var{T} = } __v4l2_handler_g_parm__ (@var{h})\n\
Return current frame interval as numerator, denominator.\n\
@end deftypefn")
{
  octave_value_list retval;
  int nargin = args.length ();

  if (nargin != 1)
    {
      print_usage ();
      return retval;
    }

  v4l2_handler* imgh = get_v4l2_handler_from_ov (args(0));
  if (imgh)
    {
      retval = octave_value(imgh->g_parm ());
    }
  return retval;
}

// PKG_ADD: autoload ("__v4l2_handler_s_parm__", which ("__v4l2_handler__.oct"));
// PKG_DEL: autoload ("__v4l2_handler_s_parm__", which ("__v4l2_handler__.oct"), "remove");
DEFUN_DLD(__v4l2_handler_s_parm__, args, nargout,
          "-*- texinfo -*-\n\
@deftypefn {Loadable Function} {@var{T} = } __v4l2_handler_s_parm__ (@var{h}, @var{s})\n\
Set frame interval numerator and denominator.\n\
@end deftypefn")
{
  octave_value_list retval;
  int nargin = args.length ();

  if (nargin != 2)
    {
      print_usage ();
      return retval;
    }

  v4l2_handler* imgh = get_v4l2_handler_from_ov (args(0));
  if (imgh)
    {
      imgh->s_parm(args(1).matrix_value ());
    }
  return retval;
}

// PKG_ADD: autoload ("__v4l2_handler_g_fmt__", which ("__v4l2_handler__.oct"));
// PKG_DEL: autoload ("__v4l2_handler_g_fmt__", which ("__v4l2_handler__.oct"), "remove");
DEFUN_DLD(__v4l2_handler_g_fmt__, args, nargout,
          "-*- texinfo -*-\n\
@deftypefn {Loadable Function} @var{fmt} = __v4l2_handler_g_fmt__ (@var{h})\n\
Get format pixelformat, size[width height].\n\
@end deftypefn")
{
  octave_value_list retval;
  int nargin = args.length ();

  if (nargin != 1)
    {
      print_usage ();
      return retval;
    }

  v4l2_handler* imgh = get_v4l2_handler_from_ov (args(0));
  if (imgh)
    {
      retval = octave_value(imgh->g_fmt ());
    }
  return retval;
}

// PKG_ADD: autoload ("__v4l2_handler_s_fmt__", which ("__v4l2_handler__.oct"));
// PKG_DEL: autoload ("__v4l2_handler_s_fmt__", which ("__v4l2_handler__.oct"), "remove");
DEFUN_DLD(__v4l2_handler_s_fmt__, args, nargout,
          "-*- texinfo -*-\n\
@deftypefn {Loadable Function} __v4l2_handler_s_fmt__ (@var{h}, @var{fmt}, @var{size})\n\
Set format @var{fmt}, @var{size} (V4L2_FIELD_INTERLACED).\n\
@end deftypefn")
{
  octave_value_list retval;
  int nargin = args.length ();

  if (nargin != 3)
    {
      print_usage ();
      return retval;
    }

  v4l2_handler* imgh = get_v4l2_handler_from_ov (args(0));
  if (imgh)
    {
      string fmt = args(1).string_value ();
      Matrix s = args(2).matrix_value ();
      unsigned int xres = s(0);
      unsigned int yres = s(1);
      if (! error_state)
        {
          imgh->s_fmt (fmt, xres, yres);
        }
    }
  return retval;
}
// CONTROLS

// PKG_ADD: autoload ("__v4l2_handler_queryctrl__", which ("__v4l2_handler__.oct"));
// PKG_DEL: autoload ("__v4l2_handler_queryctrl__", which ("__v4l2_handler__.oct"), "remove");
DEFUN_DLD(__v4l2_handler_queryctrl__, args, nargout,
          "-*- texinfo -*-\n\
@deftypefn {Loadable Function} {@var{controls} = } __v4l2_handler_queryctrl__ (@var{h})\n\
Query controls like brightness, contrast, saturation etc. from v4l2_handler @var{h}.\n\
Use the field id for calls to __v4l2_handler_s_ctrl__.\n\
@seealso{__v4l2_handler_s_ctrl__}\n\
@end deftypefn")
{
  octave_value_list retval;
  int nargin = args.length ();

  if (nargin != 1)
    {
      print_usage ();
      return retval;
    }

  v4l2_handler* imgh = get_v4l2_handler_from_ov (args(0));
  if (imgh)
    {
      retval = imgh->queryctrl ();
    }
  return retval;
}

// PKG_ADD: autoload ("__v4l2_handler_g_ctrl__", which ("__v4l2_handler__.oct"));
// PKG_DEL: autoload ("__v4l2_handler_g_ctrl__", which ("__v4l2_handler__.oct"), "remove");
DEFUN_DLD(__v4l2_handler_g_ctrl__, args, nargout,
          "-*- texinfo -*-\n\
@deftypefn {Loadable Function} {@var{value} =} __v4l2_handler_g_ctrl__ (@var{h}, @var{id})\n\
Get value for control @var{id} from v4l2_handler @var{h}.\n\
Use the field id from __v4l2_handler_queryctrl__.\n\
@seealso{__v4l2_handler_queryctrl__}\n\
@end deftypefn")
{
  octave_value_list retval;
  int nargin = args.length ();

  if (nargin != 2)
    {
      print_usage ();
      return retval;
    }

  v4l2_handler* imgh = get_v4l2_handler_from_ov (args(0));
  if (imgh)
    {
      unsigned int id = args(1).int_value ();
      if (!error_state)
        retval = octave_value(imgh->g_ctrl (id));
      else
        error("ID has to be an integer value");
    }
  return retval;
}

// PKG_ADD: autoload ("__v4l2_handler_s_ctrl__", which ("__v4l2_handler__.oct"));
// PKG_DEL: autoload ("__v4l2_handler_s_ctrl__", which ("__v4l2_handler__.oct"), "remove");
DEFUN_DLD(__v4l2_handler_s_ctrl__, args, nargout,
          "-*- texinfo -*-\n\
@deftypefn {Loadable Function} {} __v4l2_handler_s_ctrl__ (@var{h}, @var{id}, @var{value})\n\
Set control @var{id} like brightness, contrast, saturation etc. in v4l2_handler @var{h}.\n\
Use the field id from __v4l2_handler_queryctrl__.\n\
@seealso{__v4l2_handler_queryctrl__}\n\
@end deftypefn")
{
  octave_value_list retval;
  int nargin = args.length ();

  if (nargin != 3)
    {
      print_usage ();
      return retval;
    }

  v4l2_handler* imgh = get_v4l2_handler_from_ov (args(0));
  if (imgh)
    {
      unsigned int id = args(1).int_value ();
      unsigned int value = args(2).int_value ();
      if (!error_state)
        imgh->s_ctrl (id, value);
      else
        error("ID and VALUE has to be integer values");
    }
  return retval;
}
// STREAMING

// PKG_ADD: autoload ("__v4l2_handler_streamoff__", which ("__v4l2_handler__.oct"));
// PKG_DEL: autoload ("__v4l2_handler_streamoff__", which ("__v4l2_handler__.oct"), "remove");
DEFUN_DLD(__v4l2_handler_streamoff__, args, nargout,
          "-*- texinfo -*-\n\
@deftypefn {Loadable Function} __v4l2_handler_streamoff__ (@var{h})\n\
Stop streaming.\n\
@seealso{streamoff}\n\
@end deftypefn")
{
  octave_value_list retval;
  int nargin = args.length ();

  if (nargin != 1)
    {
      print_usage ();
      return retval;
    }

  v4l2_handler* imgh = get_v4l2_handler_from_ov (args(0));
  if (imgh)
    imgh->streamoff ();
  return retval;
}

// PKG_ADD: autoload ("__v4l2_handler_streamon__", which ("__v4l2_handler__.oct"));
// PKG_DEL: autoload ("__v4l2_handler_streamon__", which ("__v4l2_handler__.oct"), "remove");
DEFUN_DLD(__v4l2_handler_streamon__, args, nargout,
          "-*- texinfo -*-\n\
@deftypefn {Loadable Function} __v4l2_handler_streamon__ (@var{h}, @var{n})\n\
Start streaming with @var{n} buffers. It is recommended to use at least 2 buffers.\n\
@seealso{streamoff, getsnapshot}\n\
@end deftypefn")
{
  octave_value_list retval;
  int nargin = args.length ();

  if (nargin != 2)
    {
      print_usage ();
      return retval;
    }

  v4l2_handler* imgh = get_v4l2_handler_from_ov (args(0));
  if (imgh)
    {
      unsigned int n_buffers = args(1).int_value ();
      if (! error_state)
        {
          imgh->streamon (n_buffers);
        }
    }
  return retval;
}
// CAPTURES

// PKG_ADD: autoload ("__v4l2_handler_capture__", which ("__v4l2_handler__.oct"));
// PKG_DEL: autoload ("__v4l2_handler_capture__", which ("__v4l2_handler__.oct"), "remove");
DEFUN_DLD(__v4l2_handler_capture__, args, nargout,
          "-*- texinfo -*-\n\
@deftypefn {Loadable Function} {@var{f} =} __v4l2_handler_capture__ (@var{h}, [@var{preview}])\n\
Get a snapshot from v4l2_handler @var{h}\n\
@end deftypefn")
{
  octave_value_list retval;
  int nargin = args.length ();

  if (nargin < 1 || nargin>2)
    {
      print_usage ();
      return retval;
    }

  v4l2_handler* imgh = get_v4l2_handler_from_ov (args(0));
  if (imgh)
    {
      int preview = 0;
      if (nargin==2)
        preview = args(1).int_value ();
      if (!error_state)
        {
          retval = imgh->capture (nargout, preview);
        }
    }
  return retval;
}

typedef std::vector<std::string> dev_vec;

static bool is_v4l_dev(const char *name)
{
  return !memcmp(name, "video", 5) ||
    !memcmp(name, "radio", 5) ||
    !memcmp(name, "vbi", 3) ||
    !memcmp(name, "v4l-subdev", 10);
}

// PKG_ADD: autoload ("__v4l2_list_devices__", which ("__v4l2_handler__.oct"));
// PKG_DEL: autoload ("__v4l2_list_devices__", which ("__v4l2_handler__.oct"), "remove");
DEFUN_DLD(__v4l2_list_devices__, args, nargout,
          "-*- texinfo -*-\n\
@deftypefn {Loadable Function} {@var{l} =} __v4l2_list_devices__ ()\n\
List v4l2 devices in /dev/. It doesn't resolve links.\n\
Use '$ v4l2-ctl --list-devices' for more details.\n\
@end deftypefn")
{
// Most of this code was taken from v4l2-ctl-common.cpp:list_devices()
// which is part of the v4l-utils (http://git.linuxtv.org/v4l-utils.git).
// Thanks to Kevin Thayer (Copyright (C) 2003-2004),
// Hans Verkuil (Copyright (C) 2004, 2006, 2007) and the linuxtv community.

  octave_map retval;
  DIR *dp;
  struct dirent *ep;
  dev_vec files;

  dp = opendir("/dev");
  if (dp == NULL) {
    error ("Couldn't open /dev/ directory");
    return octave_value();
  }
  while ((ep = readdir(dp)))
    if (is_v4l_dev(ep->d_name))
      files.push_back(std::string("/dev/") + ep->d_name);
  closedir(dp);

  octave_idx_type i=0;
  for (dev_vec::iterator iter = files.begin();
      iter != files.end(); ++iter)
    {
      v4l2_handler h;
      h.open(iter->c_str());
      octave_scalar_map caps = h.querycap ().scalar_map_value();
      caps.assign ("device", *iter);
      retval.assign(i++, caps);
    }
  return octave_value (retval);
}

// PKG_ADD: autoload ("__v4l2_preview_window_is_shown__", which ("__v4l2_handler__.oct"));
// PKG_DEL: autoload ("__v4l2_preview_window_is_shown__", which ("__v4l2_handler__.oct"), "remove");
DEFUN_DLD(__v4l2_preview_window_is_shown__, args, nargout,
          "-*- texinfo -*-\n\
@deftypefn {Loadable Function} {@var{l} =} __v4l2_preview_window_is_shown__ (@var{h})\n\
Return preview_window->shown().\n\
@end deftypefn")
{
  octave_value ret;
  if (args.length () != 1)
    {
      print_usage ();
      return ret;
    }

  v4l2_handler* imgh = get_v4l2_handler_from_ov (args(0));
  if (imgh)
    {
      ret = imgh->preview_window_is_shown ();
    }
  return ret;
}

/*
%!demo
%! disp("open /dev/video0 and show live images with 2 different formats")
%! vi = __v4l2_handler_open__("/dev/video0");
%! s = __v4l2_handler_enum_framesizes__(vi, "RGB24"); # get available frame sizes
%! __v4l2_handler_s_fmt__(vi, "RGB24", s(1,:));       # use the default framesize
%! __v4l2_handler_streamon__(vi, 2);                  # enable streaming with 2 buffers
%! l = 200;
%! for i=1:l
%!   __v4l2_handler_capture__(vi, 1);                 # capture 200 frames and show preview
%! endfor
%! __v4l2_handler_streamoff__(vi);                    # diable streaming
%! __v4l2_handler_s_fmt__(vi, "RGB24", s(2,:));       # use smales available format
%! disp("The image size is now")
%! disp(__v4l2_handler_g_fmt__(vi))
%! __v4l2_handler_streamon__(vi, 2);                  # enable streaming with 2 buffers
%! for i=1:l
%!   __v4l2_handler_capture__(vi, 1);
%! endfor
%! __v4l2_handler_streamoff__(vi);
*/

/*
%!demo
%! x = __v4l2_handler_open__(__test__device__());
%! disp("get controls")
%! ctrls = __v4l2_handler_queryctrl__(x)
%! fieldnames(__v4l2_handler_queryctrl__(x))
*/

/*
%!test
%! x = __v4l2_handler_open__(__test__device__());
%! s = __v4l2_handler_enum_framesizes__(x, "RGB24");
%! default_size = s(1,:);
%! __v4l2_handler_s_fmt__(x, "RGB24", default_size);
%! t = __v4l2_handler_enum_frameintervals__(x, default_size, "RGB24");
%! #__v4l2_handler_enum_fmt__(x).description
%! __v4l2_handler_streamon__(x, 2);
%! [img, seq, timestamp] = __v4l2_handler_capture__(x);
%! assert(size(img), [default_size(2), default_size(1), 3]);
*/

/*  change controls
%!test
%! x = __v4l2_handler_open__(__test__device__());
%! s = __v4l2_handler_enum_framesizes__(x, "RGB24");
%! __v4l2_handler_s_fmt__(x, "RGB24", s(end,:));
%! ctrls = __v4l2_handler_queryctrl__(x);
%!   if (isfield(ctrls, "brightness"))
%!   min_brightness = ctrls.brightness.min;
%!   max_brightness = ctrls.brightness.max;
%!   __v4l2_handler_s_ctrl__(x, ctrls.brightness.id, min_brightness);
%!   assert(__v4l2_handler_g_ctrl__(x, ctrls.brightness.id), min_brightness)
%!   __v4l2_handler_s_ctrl__(x, ctrls.brightness.id, max_brightness);
%!   assert(__v4l2_handler_g_ctrl__(x, ctrls.brightness.id), max_brightness)
%!   v = round(max_brightness/2);
%!   __v4l2_handler_s_ctrl__(x, ctrls.brightness.id, v);
%!   assert(__v4l2_handler_g_ctrl__(x, ctrls.brightness.id), v);
%! endif
*/

/*  check get timeperframe (1/fps).
 *  This may fail for example with some sn9c20x cameras
%!test
%! x = __v4l2_handler_open__(__test__device__());
%! r = __v4l2_handler_g_parm__(x);
*/
