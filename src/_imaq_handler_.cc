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

DEFUN_DLD(__imaq_handler_querycap__, args, nargout,
          "-*- texinfo -*-\n\
@deftypefn {Loadable Function} {@var{c} = } __imaq_handler_querycap__ (@var{h})\n\
Query device capabilities, driver name, card type etc. from imaq_handler @var{h}.\n\
@end deftypefn")
{
  octave_value_list retval;
  int nargin = args.length();

  if (nargin != 1)
    {
      print_usage();
      return retval;
    }

  imaq_handler* imgh = get_imaq_handler_from_ov(args(0));
  if (imgh)
    {
      retval = imgh->querycap();
    }
  return retval;
}
// INPUTS

DEFUN_DLD(__imaq_handler_enuminput__, args, nargout,
          "-*- texinfo -*-\n\
@deftypefn {Loadable Function} {@var{inputs} = } __imaq_handler_enuminput__ (@var{h})\n\
Enumerate video inputs from imaq_handler @var{h}.\n\
Returns a struct with informations for all avaliable v4l2 inputs.\n\
@end deftypefn")
{
  octave_value_list retval;
  int nargin = args.length();

  if (nargin != 1)
    {
      print_usage();
      return retval;
    }

  imaq_handler* imgh = get_imaq_handler_from_ov(args(0));
  if (imgh)
    {
      retval = imgh->enuminput();
    }
  return retval;
}

DEFUN_DLD(__imaq_handler_g_input__, args, nargout,
          "-*- texinfo -*-\n\
@deftypefn {Loadable Function} {@var{N} =} __imaq_handler_g_input__ (@var{h})\n\
Query the current video input from imaq_handler @var{h}.\n\
@end deftypefn")
{
  octave_value_list retval;
  int nargin = args.length();
  if (nargin != 1)
    {
      print_usage();
      return retval;
    }

  imaq_handler* imgh = get_imaq_handler_from_ov(args(0));
  if (imgh)
    {
      retval = octave_value(imgh->g_input());
    }
  return retval;
}

DEFUN_DLD(__imaq_handler_s_input__, args, nargout,
          "-*- texinfo -*-\n\
@deftypefn {Loadable Function} {} __imaq_handler_s_input__ (@var{h}, @var{n})\n\
Select video input @var{n} from imaq_handler @var{h}.\n\
@end deftypefn")
{
  octave_value_list retval;
  int nargin = args.length();

  if (nargin != 2)
    {
      print_usage();
      return retval;
    }

  imaq_handler* imgh = get_imaq_handler_from_ov(args(0));
  if (imgh)
    {
      int num = args(1).int_value();
      if (!error_state)
        imgh->s_input(num);
      else
        error("N has to be a integer selecting the desired video input, starting from  0.");
    }
  return retval;
}
// FORMAT

DEFUN_DLD(__imaq_handler_enum_fmt__, args, nargout,
          "-*- texinfo -*-\n\
@deftypefn {Loadable Function} {@var{formats} = } __imaq_handler_enum_fmt__ (@var{h})\n\
Enumerate image formats from imaq_handler @var{h}.\n\
Returns a struct with informations for all avaliable v4l2 formats.\n\
@end deftypefn")
{
  octave_value_list retval;
  int nargin = args.length();

  if (nargin != 1)
    {
      print_usage();
      return retval;
    }

  imaq_handler* imgh = get_imaq_handler_from_ov(args(0));
  if (imgh)
    {
      retval = imgh->enum_fmt();
    }
  return retval;
}

DEFUN_DLD(__imaq_handler_enum_framesizes__, args, nargout,
          "-*- texinfo -*-\n\
@deftypefn {Loadable Function} {@var{sizes} = } __imaq_handler_enum_framesizes__ (@var{h})\n\
@deftypefnx {Loadable Function} {@var{sizes} = } __imaq_handler_enum_framesizes__ (@var{h}, @var{format})\n\
Enumerate available frame sizes from imaq_handler @var{h}.\n\
If no format is given, V4L2_PIX_FMT_RGB24 is assumed.\n\
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
      unsigned int pixel_format = V4L2_PIX_FMT_RGB24;
      if (nargin == 2)
        {
          unsigned int tmp_pixel_format = args(1).int_value();
          if (!error_state)
            pixel_format = tmp_pixel_format;
          else
            error("FORMAT not valid");
        }
      retval = octave_value(imgh->enum_framesizes(pixel_format));
    }
  return retval;
}

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

DEFUN_DLD(__imaq_handler_g_fmt__, args, nargout,
          "-*- texinfo -*-\n\
@deftypefn {Loadable Function} @var{fmt} = __imaq_handler_g_fmt__ (@var{h})\n\
Get format.\n\
@end deftypefn")
{
  octave_value_list retval;
  int nargin = args.length ();

  if (nargin != 1)
    {
      print_usage();
      return retval;
    }

  imaq_handler* imgh = get_imaq_handler_from_ov(args(0));
  if (imgh)
    {
      retval = octave_value(imgh->g_fmt());
    }
  return retval;
}

DEFUN_DLD(__imaq_handler_s_fmt__, args, nargout,
          "-*- texinfo -*-\n\
@deftypefn {Loadable Function} __imaq_handler_s_fmt__ (@var{h}, @var{size})\n\
Set format @var{size} (V4L2_PIX_FMT_RGB24, V4L2_FIELD_INTERLACED).\n\
@end deftypefn")
{
  octave_value_list retval;
  int nargin = args.length ();

  if (nargin != 2)
    {
      print_usage();
      return retval;
    }

  imaq_handler* imgh = get_imaq_handler_from_ov(args(0));
  if (imgh)
    {
      Matrix s = args(1).matrix_value();
      unsigned int xres = s(0);
      unsigned int yres = s(1);
      if (! error_state)
        {
          imgh->s_fmt(xres, yres);
        }
    }
  return retval;
}
// CONTROLS

DEFUN_DLD(__imaq_handler_queryctrl__, args, nargout,
          "-*- texinfo -*-\n\
@deftypefn {Loadable Function} {@var{controls} = } __imaq_handler_queryctrl__ (@var{h})\n\
Query controls like brightness, contrast, saturation etc. from imaq_handler @var{h}.\n\
Use the field id for calls to __imaq_handler_s_ctrl__.\n\
@seealso{__imaq_handler_s_ctrl__}\n\
@end deftypefn")
{
  octave_value_list retval;
  int nargin = args.length();

  if (nargin != 1)
    {
      print_usage();
      return retval;
    }

  imaq_handler* imgh = get_imaq_handler_from_ov(args(0));
  if (imgh)
    {
      retval = imgh->queryctrl();
    }
  return retval;
}

DEFUN_DLD(__imaq_handler_g_ctrl__, args, nargout,
          "-*- texinfo -*-\n\
@deftypefn {Loadable Function} {@var{value} =} __imaq_handler_g_ctrl__ (@var{h}, @var{id})\n\
Get value for control @var{id} from imaq_handler @var{h}.\n\
Use the field id from __imaq_handler_queryctrl__.\n\
@seealso{__imaq_handler_queryctrl__}\n\
@end deftypefn")
{
  octave_value_list retval;
  int nargin = args.length();

  if (nargin != 2)
    {
      print_usage();
      return retval;
    }

  imaq_handler* imgh = get_imaq_handler_from_ov(args(0));
  if (imgh)
    {
      unsigned int id = args(1).int_value();
      if (!error_state)
        retval = octave_value(imgh->g_ctrl(id));
      else
        error("ID has to be an integer value");
    }
  return retval;
}

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
// STREAMING

DEFUN_DLD(__imaq_handler_streamoff__, args, nargout,
          "-*- texinfo -*-\n\
@deftypefn {Loadable Function} __imaq_handler_streamoff__ (@var{h})\n\
Stop streaming.\n\
@seealso{streamoff}\n\
@end deftypefn")
{
  octave_value_list retval;
  int nargin = args.length ();

  if (nargin != 1)
    {
      print_usage();
      return retval;
    }

  imaq_handler* imgh = get_imaq_handler_from_ov(args(0));
  if (imgh)
    imgh->streamoff();
  return retval;
}

DEFUN_DLD(__imaq_handler_streamon__, args, nargout,
          "-*- texinfo -*-\n\
@deftypefn {Loadable Function} __imaq_handler_streamon__ (@var{h}, @var{n})\n\
Start streaming with @var{n} buffers. It is recommended to use at least 2 buffers.\n\
@seealso{streamoff, getsnapshot}\n\
@end deftypefn")
{
  octave_value_list retval;
  int nargin = args.length ();

  if (nargin != 2)
    {
      print_usage();
      return retval;
    }

  imaq_handler* imgh = get_imaq_handler_from_ov(args(0));
  if (imgh)
    {
      unsigned int n_buffers = args(1).int_value();
      if (! error_state)
        {
          imgh->streamon(n_buffers);
        }
    }
  return retval;
}
// CAPTURES

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

/*
%!demo
%! disp("open /dev/video0 and show live images with 2 different formats")
%! vi = __imaq_handler_open__("/dev/video0");
%! s = __imaq_handler_enum_framesizes__(vi);  # get available frame sizes
%! __imaq_handler_s_fmt__(vi, s(1,:));         # use the default framesize
%! __imaq_handler_streamon__(vi, 2);           # enable streaming with 2 buffers
%! l = 200;
%! for i=1:l
%!   __imaq_handler_capture__(vi, 1);          # capture 200 frames and show preview
%! endfor
%! __imaq_handler_streamoff__(vi);             # diable streaming
%! __imaq_handler_s_fmt__(vi, s(2,:));         # use smales available format
%! disp("The image size is now")
%! disp(__imaq_handler_g_fmt__(vi))
%! __imaq_handler_streamon__(vi, 2);           # enable streaming with 2 buffers 
%! for i=1:l
%!   __imaq_handler_capture__(vi, 1);
%! endfor
%! __imaq_handler_streamoff__(vi);
